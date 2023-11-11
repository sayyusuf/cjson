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

#include "lwjson.h"


static int
jumpto(const char **str, const char c);


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

static size_t
key_len(const char *str)
{
	size_t	len;

	len = 0;
	while (str[len] && !strchr(":.[{\"", str[len]) && !isspace(str[len]))
		++len;
	return (len);
}

static int
jumpto_dquote(const char **str)
{
	++(*str);
	while (**str && **str != '"')
		++(*str);
	if (!**str)
		return (-1);
	++(*str);
	return (0);
}

static int
jumpto_bracket(const char **str, const char c)
{
	++(*str);
	while (**str && **str != c)
	{
		if ('"' == **str || '{' == **str || '[' == **str)
		{
			if (0 > jumpto(str, **str))
				return (-1);
		}
		else
			++(*str);
	}
	if (!**str)
		return (-1);
	++(*str);
	return (0);
}

static int
jumpto_number(const char **str)
{
	while (**str && (isdigit(**str) || '.' == **str))
		++(*str);
	if (!**str)
		return (-1);
	return (0);
}

static int
jumpto(const char **str, const char c)
{
	switch(c)
	{
		case '"':
			return (jumpto_dquote(str));
		case '{':
			return (jumpto_bracket(str, '}'));
		case '[':
			return (jumpto_bracket(str, ']'));	
		case 't':
			if (!strncmp("true", *str, sizeof("true") - 1))
				return ((*str += sizeof("true") - 1), 0);
			break ;
		case 'f':
			if (!strncmp("false", *str, sizeof("fasle") - 1))
				return ((*str += sizeof("fasle") - 1), 0);
			break ;
		case 'n':
			if (!strncmp("null", *str, sizeof("null") - 1))
				return ((*str += sizeof("null") - 1), 0);
			break ;
		default:
			if (**str && (isdigit(**str) || '.' == **str))
				return (jumpto_number(str));
			else
				return (-1);
	}
	return (-1);
}

static int
parse_array(const char *fmt, const char **begin, const char **end)
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
		FT_CHECK( -1 < space_iter(&str) && str < *end, -ESTR, "\n");
		if (!*str || 0 > jumpto(&str, *str) || str >= *end)
			return (-ESTR);
		FT_CHECK( -1 < space_iter(&str) && str < *end, -ESTR, "\n");
		if (']' == *str)
			return (-ENFND);
		if (',' != *str)
			return (-ESTR);
		++str;
		++i;
	}
	FT_CHECK( -1 < space_iter(&str) && str < *end, -ESTR, "\n");
	tmp = str;
	if (!*str || 0 > jumpto(&str, *str) || str >= *end)
		return (-ESTR);
	*begin = tmp;
	*end = str;
	return (0);
}


static int
parse_obj(const char *fmt, const char **begin, const char **end)
{	
	size_t		fmt_len;
	size_t		str_len;
	int		res;
	const char	*str;
	const char	*tmp;

	str = *begin;
	++str;
	fmt_len = key_len(++fmt);
	if (!fmt_len)
		return (-EFMT);
	while (*str)
	{
		FT_CHECK( -1 < space_iter(&str) && str < *end, -ESTR, "\n");
		FT_CHECK( '\"' == *str , -ESTR, "\n");
		str_len = key_len(++str);
		if (!str_len)
			return (-ESTR);
		res = fmt_len == str_len && !strncmp(str, fmt, fmt_len > str_len? fmt_len: str_len);
		str +=  str_len;
		while (*str && ':' != *str)
			++str;
		++str;
		FT_CHECK( -1 < space_iter(&str) && str < *end, -ESTR, "\n");
		tmp = str;
		if (!*str || 0 > jumpto(&str, *str) || str >= *end)
			return (-ESTR);
		if (res)
		{
			*begin = tmp;
			*end = str;
			return (0);
		}
		FT_CHECK( -1 < space_iter(&str) && str < *end, -ESTR, "\n");
		if ('}' == *str)
			return (-ENFND);
		if (',' != *str)
			return (-ESTR);
		++str;
	}
	return (-ENFND);
}

static int
buff_ops(const char *begin, const char *end, va_list *args)
{
	char	*buff;
	int	n;

	buff = va_arg(*args, char *);
	n = va_arg(*args, int);
	FT_CHECK(buff && -1 != (long int)buff && -1 != n, -EBUFF, "lwjson: O_BUFF: PARAMS error!\n")
	FT_CHECK(n >= end - begin, -EBUFF, "lwjson: O_BUFF: EBUFF error!\n")
	strncpy(buff, begin, end - begin);
	buff[end - begin] = 0;
	return (0);

}

static int
ptr_ops(const char *begin, const char *end, va_list *args)
{
	const char	**ret_begin;
	const char	**ret_end;

	ret_begin = va_arg(*args, const char **);
	ret_end = va_arg(*args, const char **);
	FT_CHECK(ret_begin && -1 != (long int)ret_begin && ret_end && -1 != (long int)ret_end, \
		-EPARAMS, "lwjson: O_PTR: EPARAMS error!\n")
	*ret_begin = begin;
	*ret_end = end;
	return (0);
}

static int
alloc_ops(const char *begin, const char *end, va_list *args)
{
	char	*buff;
	char	**ret;

	ret = va_arg(*args, char **);
	FT_CHECK(ret && -1 != (long int) ret, -EPARAMS, "lwjson: O_ALLOC: EPARAMS error!\n")
	buff = malloc(end - end + 1);
	FT_CHECK(buff, -EALLOC, "lwjson:O_ALLOC: EALLOC error!\n")
	strncpy(buff, begin, end - begin);
	buff[end - begin] = 0;
	*ret = buff;
	return (0);

}

static int
ret_ops(const char *begin, const char *end, int ops, va_list *args)
{
	if (*begin == '"')
	{
		++begin;
		--end;
	}
	switch (ops)
	{
		case O_BUFF:
			return (buff_ops(begin, end, args));
		case O_PTR:
			return (ptr_ops(begin, end, args));
		case O_ALLOC:
			return (alloc_ops(begin, end, args));
		default:
			return (-EOPS);
	}
	return (0);
}

static int
parse_any(const char *str, const char *fmt, int ops, va_list *args)
{
	int		len;
	int		status;
	const char	*begin;
	const char	*end;

	FT_CHECK( -1 < space_iter(&str), -ESTR, "\n");
	len = strlen(str);
	begin = str;
	end = str + len;
	space_iter(&fmt);
	while (*fmt)
	{
		switch(*fmt)
		{
			case '.':
				status = parse_obj(fmt, &begin, &end);
				break ;
			case '[':
				status = parse_array(fmt, &begin, &end);
				break ;
			default:
				return (-EFMT);
		}
		if (0 > status)
			return (status);
		fmt += key_len(fmt + 1) + 1;
	}
	return (ret_ops(begin, end, ops, args));
}


int
lwjson_parse(const char *str, const char *fmt, int ops, ...)
{
	va_list	args;
	int	status;
	if (!str || !fmt)
		return (-1);
	va_start(args, ops);
	status = parse_any(str, fmt, ops, &args);
	va_end(args);
	return (status);
}

#ifdef __cplusplus
}
#endif
