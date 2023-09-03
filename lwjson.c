
#include <string.h>
#include <stddef.h>
#include <ctype.h>

#define FT_CHECK(status, ret, ch,  ...){\
	if (!(status))\
	{\
		ch(STDERR_FILENO, __VA_ARGS__);\
		return (ret);\
	}\
}


const char *string = "						\
{								\
	\"dummyarray\": [					\
		[],						\
		[],						\
		[						\
			{},					\
			{					\
				\"emlploye\": {			\
					\"name\": \"heyyo\",	\
				\"age\": 19			\
				}				\
			},					\
			{}					\
		]						\
	]							\
}								




/**
 * @param fmt ".dumyarray[2][1].employ.name"
 * @param fmt "[0][2].employ.name"
 * */

#define EFMT	1
#define ESTR	2
#define ENFND	3


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
	while (*str && !strchar(":.[{\"", *str) && !isspace(*str))
		++len;
	return (len);
}

static int
jumpto(const char **str, const char c)
{
	switch(c)
	{
		case '"':
			while (**str && **str != '"')
				(*str)++;
			if (!**str)
				return (-1);
			break;
		case '{':
			while (**str && **str != '}')
			{
				if ('"' == **str || '[' == **str)
					if (0 > jumpto(str, **str))
						return (-1);
				(*str)++;
			}
			if (!**str)
				return (-1);
			break;
		case '[':
			while (**str && **str != ']')
			{
				if ('"' == **str || '{' == **str)
					if (0 > jumpto(str, **str))
						return (-1);
				(*str)++;
			}
			if (!**str)
				return (-1);
			break;
		default:
			return (-1);
	}
	return (0);
}


static int
exec_any(const char *str, const char fmt, va_list *params)
{
	while (*str && isspace(*str))
		++str;
	if (!*str || '[' == *str)
		return (-EFMT);
	if (*str == '{')
		++str;
	moveto(&str, &fmt);
}

static int
exec_array(const char *str, const char fmt, va_list *params)
{
	int	k;
	int	i;
	int	fmt_len

	k = atoi(fmt++);
	fmt_len = key_len(fmt) + 1;
	i = 0;
	++str;
	while (i < k)
	{
		
		while (*str && !strchr("{[", *str))
			++str;
		if (!*str || 0 > jumpto(&str, *str))
			return (-ESTR);
		++str;	
		FT_CHECK( -1 < space_iter(&str), -ESTR, (void), "");
		if (']' == *str)
			return (-ENFND);
		if (',' != *str)
			return (-ESTR);
			++str;
	}
	while (*str && !strchr("{[", *str))
			++str;
	if (!str)
		return (-ESTR);
	return (exec_any(str, fmt + fmt_len, args));
}




static int
exec_obj(const char *str, const char *fmt, va_list *args)
{	
	size_t	fmt_len;
	size_t	str_len;
	int	res;

	str++;
	FT_CHECK( -1 < space_iter(&str), -ESTR, (void), "");
	fmt_len = key_len(++fmt);
	if (!fmt_len)
		return (-EFMT);
	while (*str)
	{
		FT_CHECK( -1 < space_iter(&str), -ESTR, (void), "");
		FT_CHECK( '\"' == *str , -ESTR, (void), "");
		str_len = key_len(++str);
		if (!str_len)
			return (-ESTR);
		res = fmt_len == str_len && !strncmp(str, key, fmt_len > str_len? ftm_len: str_len);
		str +=  str_len;
		while (*str && ':' != *str)
			++str;
		FT_CHECK( -1 < space_iter(&str), -ESTR, (void), "");
		if (res)
			return (exec_any(str, fmt + fmt_len, args)); 
		if (!*str || 0 > jumpto(&str, *str))
			return (-ESTR);
		++str;	
		FT_CHECK( -1 < space_iter(&str), -ESTR, (void), "");
		if ('}' == *str)
			return (-ENFND);
		if (',' != *str)
			return (-ESTR);
			++str;
	}
}

static int
exec_any(const char *str, const char *fmt, va_list *args)
{
	int	c;
	int	status;

	if (!*fmt)
		string
	FT_CHECK( -1 < space_iter(&str), -ESTR, (void), "");
	switch(*fmt)
	{
		case '.':
			switch(*str)
			{
				case '{':
					return  (exec_obj(str, fmt, params));
				default:
			
			}
		case '[':
			return (exec_array(str, fmt, params));
		default:
			return (-EFMT);
	}
}

int
lw_json_parse(const char *str, const char *fmt, ...)
{
	va_list	args;
	int	status;
	if (!str || !fmt)
		return (-1);
	va_start(args, fmt);
	status = detect
	va_end(args);
	return (status);
}
