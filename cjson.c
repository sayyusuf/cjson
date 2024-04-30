#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

#include "cjson.h"

#ifdef DEBUG_MODE
# define PRINT		fprintf
# define PRINT_CH	stderr
#endif

#define CHECK_ST_N(status, ret, ...){			\
	if (status)					\
		return (ret);				\
}

#define CHECK_ST_D(status, ret, ...){			\
	if (status)					\
	{						\
		PRINT(PRINT_CH, __VA_ARGS__);		\
		return (ret);				\
	}						\
}


#ifdef DEBUG_MODE
# define CHECK_ST	CHECK_ST_D
#else
# define CHECK_ST	CHECK_ST_N
#endif


#define JUMPD		1
#define JUMPO		2
#define JUMPA		3
#define JUMPB		4
#define JUMPN		5
#define JUMPNUM		6

/* utils begin */

static int
jump(const char **str);


static int
space_iter(const char **str)
{
	if (!str | !*str)
		return (-1);
	while (**str && isspace(**str))
		++(*str);
	if (!**str)
		return (-1);
	return (0);
}

static int
space_iter_rev(const char **str, const char *begin)
{
	if (!str || !*str || *str <= begin)
		return (-1);
	while (*str > begin && isspace(*(*str - 1)))
		--(*str);
	if (*str == begin)
		return (-1);
	return (0);
}

static size_t
key_len(const char *str)
{
	size_t	len;

	len = 0;
	while (str[len] && !strchr(":.[{\"", str[len]) && !isspace(str[len]))
		++len;
	return (len);
}
/* utils begin */




/* return operations begin */

static int
buff_ops(const char *begin, const char *end, va_list *args)
{
	char		*buff;
	unsigned int	n;

	buff = va_arg(*args, char *);
	n = va_arg(*args, int);
	CHECK_ST(!buff, -EBUFF, "Error: cjson: O_BUFF: PARAMS line %d\n", __LINE__)
	CHECK_ST(n < end - begin, -EBUFF, "Error: cjson: [-O_BUFF]: EBUFF line %d\n", __LINE__)
	strncpy(buff, begin, end - begin);
	return (end - begin);
}

static int
ptr_ops(const char *begin, const char *end, va_list *args)
{
	const char	**ret_begin;
	const char	**ret_end;

	ret_begin = va_arg(*args, const char **);
	ret_end = va_arg(*args, const char **);
	CHECK_ST(!ret_begin || !ret_end, \
		-EPARAMS, "Error cjson: O_PTR: [-EPARAMS] line %d\n", __LINE__)
	*ret_begin = begin;
	*ret_end = end;
	return (end - begin);
}

static int
alloc_ops(const char *begin, const char *end, va_list *args)
{
	char	*buff;
	char	**ret;

	ret = va_arg(*args, char **);
	CHECK_ST(!ret, -EPARAMS, "Error: cjson: O_ALLOC: [-EPARAMS] line %d!\n", __LINE__)
	buff = malloc(end - begin + 1);
	CHECK_ST(!buff, -EALLOC, "Error: cjson: O_ALLOC: [-EALLOC] line %d\n", __LINE__)
	strncpy(buff, begin, end - begin);
	buff[end - begin] = 0;
	*ret = buff;
	return (end - begin);

}

static int
ret_ops(const char *begin, const char *end, int ops, va_list *args)
{
	switch (ops)
	{
		case JO_BUFF:
			return (buff_ops(begin, end, args));
		case JO_PTR:
			return (ptr_ops(begin, end, args));
		case JO_ALLOC:
			return (alloc_ops(begin, end, args));
		default:
			return (-EOPS);
	}
	return (0);
}

/* return operations end */




/* Jump operations begin */

static int
jump_dquote(const char **str)
{
	size_t	i;

	while (1)
	{
		++(*str);
		CHECK_ST(!**str, -ESTR, "Error: jump_dquote: [-ESTR] line %d\n", __LINE__);
		if (**str == '\"')
			break ;
		if (**str == '\\')
			switch (*((*str) + 1))
			{
				case '\"':
				case '\\':
				case '/':
				case 'b':
				case 'f':	
				case 'n':	
				case 'r':	
				case 't':
					(*str) += 1;
					break ;
				case 'u':
					i = 2;
					while (i < 6)
						if (!strchr("0123456789abcdefABCDEF", (*str)[i++]))
							CHECK_ST(1, -ESTR, "Error: jump_dquote: [-ESTR] line %d\n", __LINE__);
					(*str) += 5;
					break ;
				default :
					CHECK_ST(1, -ESTR, "Error: jump_dquote: [-ESTR] line %d\n", __LINE__);	
			}
	}
	++(*str);
	return (0);
}

static int
jump_object(const char **s)
{
	const char	*str;

	str = (*s) + 1;
	CHECK_ST(0 > space_iter(&str), -ESTR, "Error: jump_object: [-ESTR]: line %d\n", __LINE__);
	if (*str == '}')
		;
	else while (*str)
	{
		CHECK_ST( '\"' != *str , -ESTR, "Error: jump_object: [-ESTR]: line %d\n", __LINE__);
		CHECK_ST(0 > jump_dquote(&str) , -ESTR, "Error: jump_object: [-ESTR]: line %d\n", __LINE__);
		CHECK_ST(!*str , -ESTR, "Error: jump_object: [-ESTR]: line %d\n", __LINE__);
		CHECK_ST(0 > space_iter(&str), -ESTR, "Error: jump_object: [-ESTR]: line %d\n", __LINE__);
		CHECK_ST(*str != ':' , -ESTR, "Error: jump_object: [-ESTR]: line %d\n", __LINE__);
		++str;
		CHECK_ST(0 > space_iter(&str), -ESTR, "Error: jump_object: [-ESTR]: line %d\n", __LINE__);
		if (0 > jump(&str))
			return (-1);
		CHECK_ST(0 > space_iter(&str), -ESTR, "Error: jump_object: [-ESTR]: line %d\n", __LINE__);
		if (*str == '}')
			break ;
		CHECK_ST(',' != *str, -ESTR, "Error: jump_object: [-ESTR]: line %d\n", __LINE__);
		++str;
		CHECK_ST(0 > space_iter(&str), -ESTR, "Error: jump_object: [-ESTR]: line %d\n", __LINE__);
	}
	*s = str + 1;
	return (0);
}

static int
jump_array(const char **s)
{
	const char	*str;

	str = (*s) + 1;
	CHECK_ST(0 > space_iter(&str), -ESTR, "Error: jump_array: [-ESTR]: line %d\n", __LINE__);
	if (*str == ']')
		;
	else while (*str)
	{
		if (0 > jump(&str))
			return (-1);	
		CHECK_ST(0 > space_iter(&str), -ESTR, "Error: jump_array: [-ESTR]: line %d\n", __LINE__);
		if (*str == ']')
			break ;
		CHECK_ST(',' != *str, -ESTR, "Error: jump_array: [-ESTR]: line %d\n", __LINE__);
		++str;
		CHECK_ST(0 > space_iter(&str), -ESTR, "Error: jump_array: [-ESTR]: line %d\n", __LINE__);
	}
	*s = str + 1;
	return (0);
}

static int
jump_number(const char **str)
{
	int	d;
	int	e;

	e = 0;
	d = 0;
	if (**str == '-')
		++(*str);
	CHECK_ST(!isdigit(**str), -ESTR, "Error: jump_number: [-ESTR] line %d\n", __LINE__);
	while (*(++(*str)))
	{
		while (**str && isdigit(**str))
			++(*str);
		if (!e && **str == '.')
			++d;
		else if (**str == 'e' || **str == 'E')
		{	
			++e;
			if (*(*str + 1) == '+' || *(*str + 1) == '-')
				++(*str);
		}
		else
			return (0);
		CHECK_ST(e > 1 || d > 1, -ESTR, "Error: jump_number: [-ESTR] line %d\n", __LINE__);
	}
	return (0);
}

static int
jump_bool(const char **str)
{
	if (!strncmp("true", *str, sizeof("true") - 1))
		return ((*str += sizeof("true") - 1), 0);
	else if (!strncmp("false", *str, sizeof("fasle") - 1))
		return ((*str += sizeof("fasle") - 1), 0);
	else
		return (-ESTR);
}

static int
jump_null(const char **str)
{
	if (!strncmp("null", *str, sizeof("null") - 1))
		return ((*str += sizeof("null") - 1), 0);
	else
		return (-ESTR);
}

static int
jump(const char **str)
{
	switch(**str)
	{
		case '"':
			return (0 > jump_dquote(str)? -ESTR: JUMPD);
		case '{':
			return (0 > jump_object(str)? -ESTR: JUMPO);
		case '[':
			return (0 > jump_array(str)? -ESTR: JUMPA);	
		case 't':
		case 'f':
			return (0 > jump_bool(str)? -ESTR: JUMPB);
		case 'n':
			return (0 > jump_null(str)? -ESTR: JUMPN);
		default:
			if (**str && (isdigit(**str) || '-' == **str))
				return (0 > jump_number(str)? -ESTR: JUMPNUM);
			else
				CHECK_ST(1, -ESTR, "Error: jump: [-ESTR] line %d\n", __LINE__);
	}
	return (-ESTR);
}

/* jump operations end */




/* map operations begin */

static int
map_object(const char *str, struct cjson_map *fs, void *prv)
{
	const char	*b_key;
	const char	*e_key;
	const char	*b_obj;
	const char	*e_obj;

	++str;
	CHECK_ST(0 > space_iter(&str), -ESTR, "Error: map_object: [-ESTR]: line %d\n", __LINE__);
	if (*str == '}')
		;
	else while (*str)
	{
		CHECK_ST( '\"' != *str , -ESTR, "Error: map_object: [-ESTR]: line %d\n", __LINE__);
		b_key = str;
		CHECK_ST(0 > jump_dquote(&str) , -ESTR, "Error: map_object: [-ESTR]: line %d\n", __LINE__);
		CHECK_ST(!*str , -ESTR, "Error: map_object: [-ESTR]: line %d\n", __LINE__);
		e_key = str;
		CHECK_ST(0 > space_iter(&str), -ESTR, "Error: map_object: [-ESTR]: line %d\n", __LINE__);
		CHECK_ST(*str != ':' , -ESTR, "Error: map_object: [-ESTR]: line %d\n", __LINE__);
		++str;
		CHECK_ST(0 > space_iter(&str), -ESTR, "Error: map_object: [-ESTR]: line %d\n", __LINE__);
		b_obj = str;
		if (0 > jump(&str))
			return (-1);
		e_obj = str;
		CHECK_ST(0 > space_iter(&str), -ESTR, "Error: map_object: [-ESTR]: line %d\n", __LINE__);
		fs->obj(prv, b_key, e_key, b_obj, e_obj);
		if (*str == '}')
			break ;
		CHECK_ST(',' != *str, -ESTR, "Error: map_object: [-ESTR]: line %d\n", __LINE__);
		++str;
		CHECK_ST(0 > space_iter(&str), -ESTR, "Error: map_object: [-ESTR]: line %d\n", __LINE__);
	}
	return (0);
}

static int
map_array(const char *str, struct cjson_map *fs, void *prv)
{
	const char	*b_obj;
	const char	*e_obj;

	++str;
	CHECK_ST(0 > space_iter(&str), -ESTR, "Error: jump_array: [-ESTR]: line %d\n", __LINE__);
	if (*str == ']')
		;
	else while (*str)
	{
		b_obj = str;
		if (0 > jump(&str))
			return (-1);
		e_obj = str;
		CHECK_ST(0 > space_iter(&str), -ESTR, "Error: jump_array: [-ESTR]: line %d\n", __LINE__);
		fs->arr(prv, b_obj, e_obj);
		if (*str == ']')
			break ;
		CHECK_ST(',' != *str, -ESTR, "Error: jump_array: [-ESTR]: line %d\n", __LINE__);
		++str;
		CHECK_ST(0 > space_iter(&str), -ESTR, "Error: jump_array: [-ESTR]: line %d\n", __LINE__);
	}
	return (0);
}

/* map operations end */




/* format operations begin */

static int
fmt_array(const char *fmt, const char **begin, const char **end)
{
	int		k;
	int		i;
	const char	*str;
	const char	*tmp;

	k = atoi(++fmt);
	i = 0;
	str = *begin;
	++str;
	while (i < k)
	{
		CHECK_ST(0 > space_iter(&str) || str >= *end, -ESTR, "Error: fmt_array: [-ESTR]: line %d\n", __LINE__);
		CHECK_ST(0 > jump(&str) || str >= *end, -ESTR, "Error: fmt_array: [-ESTR]: line %d\n", __LINE__);
		CHECK_ST(0 > space_iter(&str) || str >= *end, -ESTR, "Error: fmt_array: [-ESTR]: line %d\n", __LINE__);
		CHECK_ST(']' == *str, -ENFND, "Error: fmt_array: [-EFND]: line %d\n", __LINE__);
		CHECK_ST(',' != *str, -ESTR, "Error: fmt_array: [-ESTR]: line %d\n", __LINE__);
		++str;
		++i;
	}
	CHECK_ST(0 > space_iter(&str) || str >= *end, -ESTR, "Error: fmt_array: [-ESTR]: line %d\n", __LINE__);
	tmp = str;
	CHECK_ST(0 > jump(&str) || str >= *end, -ESTR, "Error: fmt_array: [-ESTR]: line %d\n", __LINE__);
	*begin = tmp;
	*end = str;
	return (0);
}


static int
fmt_obj(const char *fmt, const char **begin, const char **end)
{	
	size_t		fmt_len;
	size_t		str_len;
	int		res;
	const char	*str;
	const char	*tmp;

	str = *begin;
	++str;
	fmt_len = key_len(++fmt);
	CHECK_ST(!fmt_len, -EFMT, "Error: fmt_obj: [-EFMT]: line %d\n", __LINE__);
	while (*str)
	{
		CHECK_ST(0 > space_iter(&str) || str >= *end, -ESTR, "Error: fmt_obj: [-ESTR]: line %d\n", __LINE__);
		CHECK_ST( '\"' != *str , -ESTR, "Error: fmt_obj: [-ESTR]: line %d\n", __LINE__);
		str_len = key_len(++str);
		CHECK_ST(!str_len, -ESTR, "Error: fmt_obj: [-EFMT]: line %d\n", __LINE__);
		res = fmt_len == str_len && !strncmp(str, fmt, fmt_len);
		str +=  str_len;
		CHECK_ST(*str != '\"',  -ESTR, "Error: fmt_obj: [-ESTR]: line %d\n", __LINE__);
		++str;
		CHECK_ST(0 > space_iter(&str) || str >= *end, -ESTR, "Error: fmt_obj: [-ESTR]: line %d\n", __LINE__);
		CHECK_ST(*str != ':',  -ESTR, "Error: fmt_obj: [-ESTR]: line %d\n", __LINE__);
		++str;
		CHECK_ST(0 > space_iter(&str) || str >= *end, -ESTR, "Error: fmt_obj: [-ESTR]: line %d\n", __LINE__);
		tmp = str;
		CHECK_ST(0 > jump(&str) || str >= *end, -ESTR, "Error: fmt_obj: [-ESTR]: line %d\n", __LINE__);
		if (res)
		{
			*begin = tmp;
			*end = str;
			return (0);
		}
		CHECK_ST(0 > space_iter(&str) && str < *end, -ESTR, "\n");
		CHECK_ST('}' == *str, -ENFND, "Error: fmt_obj: [-EFND]: line %d\n", __LINE__);
		CHECK_ST(',' != *str, -ESTR, "Error: fmt_obj: [-ESTR]: line %d\n", __LINE__);
		++str;
	}
	return (-ESTR);
}

static int
parse_ops(const char **begin, const char **end, const char *fmt);

static int
fmt_ops(const char **begin, const char **end, const char *fmt)
{
	int status;

	if (!*fmt)
		return (0);
	switch(*fmt)
	{
		case '.':
			status = fmt_obj(fmt, begin, end);
			break ;
		case '[':
			status = fmt_array(fmt, begin, end);
			break ;
		default:
			CHECK_ST(1, -EFMT, "Error: fmt_ops: [-EFMT]: line %d\n", __LINE__);
	}
	CHECK_ST(0 > status, status, "\n");
	fmt += key_len(fmt + 1) + 1;
	return  (parse_ops(begin, end, fmt));
}

/* format operations end */




static int
parse_ops(const char **begin, const char **end, const char *fmt)
{
	int		status;
	const char	*str;

	str = *begin;
	status = jump(&str);
	CHECK_ST(0 > status, status, "\n");
	CHECK_ST(*end != str, -ESTR,  "Error: parse_ops: [-ESTR]: line %d\n", __LINE__);
	return (fmt_ops(begin, end, fmt));
}

static int
match_ops(const char **begin, const char **end, struct cjson_match *fs, void *prv)
{
	int		status;
	const char	*str;

	str = *begin;
	status = jump(&str);
	CHECK_ST(0 > status, status, "\n");
	CHECK_ST(*end != str, -ESTR,  "Error: parse_ops: [-ESTR]: line %d\n", __LINE__);
	switch (status)
	{
		case JUMPO:	return(fs->obj? (fs->obj(prv, *begin, *end), 0): -EOPS);
		case JUMPA:	return(fs->arr? (fs->arr(prv, *begin, *end), 0): -EOPS);
		case JUMPD:	return(fs->str? (fs->str(prv, *begin, *end), 0): -EOPS);
		case JUMPNUM:	return(fs->num? (fs->num(prv, *begin, *end), 0): -EOPS);
		case JUMPB: 	return(fs->bl? (fs->bl(prv, *begin, *end), 0): -EOPS);
		case JUMPN:	 return(fs->null? (fs->null(prv, *begin, *end), 0): -EOPS);
		default:	return (-ESTR);
	}
	return (-ESTR);
}

static int
map_ops(const char **begin, const char **end, struct cjson_map *fs, void *prv)
{
	int		status;
	const char	*str;

	str = *begin;
	status = jump(&str);
	CHECK_ST(0 > status, status, "\n");
	CHECK_ST(*end != str, -ESTR,  "Error: parse_ops: [-ESTR]: line %d\n", __LINE__);
	switch (status)
	{
		case JUMPO:	return(fs->obj? map_object(*begin, fs, prv ): -EOPS);
		case JUMPA:	return(fs->arr? map_array(*begin, fs, prv): -EOPS);
		default:	return (-ESTR);
	}
	return (-ESTR);
}

int
cjson_parse_str(const char *str, const char *fmt, int ops, ...)
{
	va_list		args;
	int		status;
	const char	*begin;
	const char	*end;

	if (!str || !fmt)
		return (-EINP);
	begin = str;
	end = begin + strlen(str);
	CHECK_ST(0 > space_iter(&begin), -ESTR, "Error: cjeson_parse: [-ESTR]: line %d\n", __LINE__);
	CHECK_ST(0 > space_iter_rev(&end, begin), -ESTR, "Error: cjson_parse: [-ESTR]: line %d\n", __LINE__);
	va_start(args, ops);
	status = parse_ops(&begin, &end, fmt);
	if (-1 < status)
		status = ret_ops(begin, end, ops, &args);
	va_end(args);
	return (status);
}

int
cjson_parse_ptr(const char *begin, const char *end, const char *fmt, int ops, ...)
{
	va_list		args;
	int		status;

	if (!begin || !end || !fmt || begin >= end)
		return (-EINP);
	CHECK_ST(0 > space_iter(&begin), -ESTR, "Error: cjeson_parse: [-ESTR]: line %d\n", __LINE__);
	CHECK_ST(0 > space_iter_rev(&end, begin), -ESTR, "Error: cjson_parse: [-ESTR]: line %d\n", __LINE__);
	va_start(args, ops);
	status = parse_ops(&begin, &end, fmt);
	if (-1 < status)
		status = ret_ops(begin, end, ops, &args);
	va_end(args);
	return (status);
}

int
cjson_match(const char *str,  struct cjson_match *fs, void *prv)
{
	const char	*begin;
	const char	*end;

	if (!str || !fs)
		return (-EINP);
	begin = str;
	end = begin + strlen(str);
	CHECK_ST(0 > space_iter(&begin), -ESTR, "Error: cjson_match: [-ESTR]: line %d\n", __LINE__);
	CHECK_ST(0 > space_iter_rev(&end, begin), -ESTR, "Error: cjson_match: [-ESTR]: line %d\n", __LINE__);
	return (match_ops(&begin, &end, fs, prv));
}

int
cjson_match_ptr(const char *begin, const char *end,  struct cjson_match *fs, void *prv)
{
	if (!begin || !end || !fs|| begin >= end)
		return (-EINP);
	CHECK_ST(0 > space_iter(&begin), -ESTR, "Error: cjson_match: [-ESTR]: line %d\n", __LINE__);
	CHECK_ST(0 > space_iter_rev(&end, begin), -ESTR, "Error: cjson_match: [-ESTR]: line %d\n", __LINE__);
	return (match_ops(&begin, &end, fs, prv));
}

int
cjson_map(const char *str,  struct cjson_map *fs, void *prv)
{
	const char	*begin;
	const char	*end;

	if (!str || !fs)
		return (-EINP);
	begin = str;
	end = begin + strlen(str);
	CHECK_ST(0 > space_iter(&begin), -ESTR, "Error: cjson_match: [-ESTR]: line %d\n", __LINE__);
	CHECK_ST(0 > space_iter_rev(&end, begin), -ESTR, "Error: cjson_match: [-ESTR]: line %d\n", __LINE__);
	return (map_ops(&begin, &end, fs, prv));
}

int
cjson_map_ptr(const char *begin, const char *end,  struct cjson_map *fs, void *prv)
{
	if (!begin || !end || !fs|| begin >= end)
		return (-EINP);
	CHECK_ST(0 > space_iter(&begin), -ESTR, "Error: cjson_match: [-ESTR]: line %d\n", __LINE__);
	CHECK_ST(0 > space_iter_rev(&end, begin), -ESTR, "Error: cjson_match: [-ESTR]: line %d\n", __LINE__);
	return (map_ops(&begin, &end, fs, prv));
}

# undef CHECK_ST
# undef CHECK_D
# undef CHECK_N
# undef PRINT
# undef NOP_PARAM

#ifdef __cplusplus
}
#endif
