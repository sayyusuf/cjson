
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include "lwjson.h"
#include <unistd.h>

int main(int ac, char *av[])
{
	char 	strbuf[10000];
	char	buff[10000];
	int	stat;
	int	fd;
	int	ret;

	(void)ac;
	fd = open(av[1], O_RDONLY);
	FT_ASSERT(0 < fd, -1,"fd < 0\n");
	ret = read(fd, strbuf, 10000);
	assert(0 <= ret);
	strbuf[ret] = 0;
	printf("%s\n", av[2]);
	stat =  lwjson_parse(strbuf, av[2], O_BUFF, buff, 10000);
	printf("%s\n", buff);
	return (stat);
}
