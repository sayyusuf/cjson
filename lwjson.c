
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

#define FT_CHECK(status, ret, ch,  ...){\
	if (!(status))\
	{\
		ch(STDERR_FILENO, __VA_ARGS__);\
		return (ret);\
	}\
}


/**
 * @param fmt ".dumyarray[2][1].employe.name"
 * @param fmt "[0][2].employ.name"
 * */

#define EFMT	1
#define ESTR	2
#define ENFND	3
#define EBUFF	4


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
jumpto(const char **str, const char c)
{
	switch(c)
	{
		case '"':
			++(*str);
			while (**str && **str != '"')
				++(*str);
			if (!**str)
				return (-1);
			++(*str);
			break;
		case '{':
			++(*str);
			while (**str && **str != '}')
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
			break ;
		case '[':
			++(*str);
			while (**str && **str != ']')
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
			break ;
		case 't':
			if (!strncmp("true", *str, strlen("true")))
				*str += strlen("true");
			break ;
		case 'f':
			if (!strncmp("false", *str, strlen("fasle")))
				*str += strlen("fasle");
			break ;
		case 'n':
			if (!strncmp("null", *str, strlen("null")))
				*str += strlen("null");
			break ;
		default:
			if (**str && isdigit(**str) || '.' == **str)
			{
				while (**str && (isdigit(**str) || '.' == **str))
					++(*str);
				if (!**str)
					return (-1);
				break ;
			}
			else
				return (-1);
	}
	return (0);
}

static int
parse_array(const char *fmt, const char **begin, const char **end)
{
	int		k;
	int		i;
	int		fmt_len;
	const char	*str;
	const char	*tmp;

	k = atoi(++fmt);
	i = 0;
	str = *begin;
	++str;
	while (i < k)
	{
		FT_CHECK( -1 < space_iter(&str) && str < *end, -ESTR, (void), "");
		if (!*str || 0 > jumpto(&str, *str) || str >= *end)
			return (-ESTR);
		FT_CHECK( -1 < space_iter(&str) && str < *end, -ESTR, (void), "");
		if (']' == *str)
			return (-ENFND);
		if (',' != *str)
			return (-ESTR);
			++str;
		++i;
	}
	FT_CHECK( -1 < space_iter(&str) && str < *end, -ESTR, (void), "");
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
		FT_CHECK( -1 < space_iter(&str) && str < *end, -ESTR, (void), "");
		FT_CHECK( '\"' == *str , -ESTR, (void), "");
		str_len = key_len(++str);
		if (!str_len)
			return (-ESTR);
		res = fmt_len == str_len && !strncmp(str, fmt, fmt_len > str_len? fmt_len: str_len);
		str +=  str_len;
		while (*str && ':' != *str)
			++str;
		++str;
		FT_CHECK( -1 < space_iter(&str) && str < *end, -ESTR, (void), "");
		tmp = str;
		if (!*str || 0 > jumpto(&str, *str) || str >= *end)
			return (-ESTR);
		if (res)
		{
			*begin = tmp;
			*end = str;
			return (0);
		}
		FT_CHECK( -1 < space_iter(&str) && str < *end, -ESTR, (void), "");
		if ('}' == *str)
			return (-ENFND);
		if (',' != *str)
			return (-ESTR);
			++str;
	}
	return (-ENFND);
}

int
exec_type(const char *begin, const char *end, va_list *args)
{
	char	*buff;
	int	n;

	buff = va_arg(*args, char *);
	n = va_arg(*args, int);
	if (*begin == '"')
	{
		++begin;
		--end;
	}
	if (n < end - begin)
		return (-EBUFF);
	strncpy(buff, begin, end - begin);
	buff[end - begin] = 0;
	return (0);

}

static int
parse_any(const char *str, const char *fmt, va_list *args)
{
	int		len;
	int		status;
	const char	*begin;
	const char	*end;

	FT_CHECK( -1 < space_iter(&str), -ESTR, (void), "");
	len = strlen(str);
	begin = str;
	end = str + len;
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
	return (exec_type(begin, end, args));
}


int
lw_json_parse(const char *str, const char *fmt, ...)
{
	va_list	args;
	int	status;
	if (!str || !fmt)
		return (-1);
	va_start(args, fmt);
	status = parse_any(str, fmt, &args);
	va_end(args);
	return (status);
}

#include <stdio.h>
#include <assert.h>
#include <fcntl.h>

int main(int ac, char *av[])
{
	char 	strbuf[10000];
	char	buff[10000];
	int	stat;
	int	fd;
	int	ret;

	fd = open(av[1], O_RDONLY);
	assert(0 < fd);
	ret = read(fd, strbuf, 10000);
	assert(0 <= ret);
	strbuf[ret] = 0;
	printf("%s\n", av[2]);
	stat =  lw_json_parse(strbuf, av[2], buff, 10000);
	printf("%s\n", buff);
	return (stat);


}
