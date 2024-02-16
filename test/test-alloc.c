
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <cjson.h>
#include <unistd.h>

int main(int ac, char *av[])
{
	char 	strbuf[10000];
	int	stat;
	int	fd;
	int	ret;

	(void)ac;
	fd = open(av[1], O_RDONLY);
	ret = read(fd, strbuf, 10000);
	assert(0 <= ret);
	strbuf[ret] = 0;
	
	char *str;
	stat =  cjson_parse_str(strbuf, av[2]? av[2]: "", JO_ALLOC, &str);
	if (0 <= stat)
	{
		printf("%s\n", str);
		free(str);
	}
	return (stat);
}
