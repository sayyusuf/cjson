
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <cjson.h>
#include <unistd.h>


void m_obj(void *prv, const char *bo, const char *eo)
{
	(void)bo;
	(void)eo;
	(void)prv;
	printf("Object\n");
}

void m_arr(void *prv, const char *bo, const char *eo)
{
	(void)bo;
	(void)eo;
	(void)prv;
	printf("Array\n");
}

void m_str(void *prv, const char *bo, const char *eo)
{
	(void)bo;
	(void)eo;
	(void)prv;
	printf("string\n");
}

void m_num(void *prv, const char *bo, const char *eo)
{
	(void)bo;
	(void)eo;
	(void)prv;
	printf("number\n");
}
void m_bool(void *prv, const char *bo, const char *eo)
{
	(void)bo;
	(void)eo;
	(void)prv;
	printf("Bool\n");
}

void m_null(void *prv, const char *bo, const char *eo)
{
	(void)bo;
	(void)eo;
	(void)prv;
	printf("NULL\n");
}

struct cjson_match cm = {.obj = m_obj, .arr = m_arr, .str = m_str, .num = m_num, .bl = m_bool, .null = m_null};

void map_obj(void *prv, const char *bk, const char *ek, const char *bo, const char *eo)
{
	(void)bk;
	(void)ek;
	cjson_match_ptr(bo, eo, &cm, prv);
}

void map_arr(void *prv, const char *bo, const char *eo)
{
	cjson_match_ptr(bo, eo, &cm, prv);
}

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
	struct cjson_map fs = {.obj = map_obj, .arr = map_arr};
	stat =  cjson_map(strbuf, &fs, " <<hello world");
	return (stat);
}
