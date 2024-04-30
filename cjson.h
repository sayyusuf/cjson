#ifndef CJSON_H
#define CJSON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>


#define EINP		1
#define EFMT		2
#define ESTR		3
#define EOPS		4
#define EPARAMS		5
#define ENFND		6

#define EBUFF		7
#define EPTR		8
#define EALLOC		9

#define JO_BUFF		0
#define JO_PTR		1
#define JO_ALLOC	2

/**
 *@param str `json string`
 *@param fmt `parser format:` `".dummyarray[2][1].employe.name"` or `"[0][2].employ.name"`
 *@param ops `options:` `O_BUFF` or `O_PTR` or `O_ALLOC`
 *@return `api status`
 *`int cjson_parse(const char *str, const char *fmt, int ops, char *buff, int n);`
 *`int cjson_parse(const char *str, const char *fmt, int ops, char **begin, char **end);`
 *`int cjson_parse(const char *str, const char *fmt, int ops, char **alloc);`
 */

struct cjson_map
{
	void (*obj)(void *prv, const char *b_key, const char *e_key, const char *b_obj, const char *e_obj);
	void (*arr)(void *prv, const char *b_obj, const char *e_obj);
};

struct cjson_match
{
	void (*obj)(void *prv, const char *b_obj, const char *e_obj);
	void (*arr)(void *prv, const char *b_obj, const char *e_obj);
	void (*str)(void *prv, const char *b_obj, const char *e_obj);
	void (*num)(void *prv, const char *b_obj, const char *e_obj);
	void (*bl)(void *prv, const char *b_obj, const char *e_obj);
	void (*null)(void *prv, const char *b_obj, const char *e_obj);
};

int
cjson_parse_str(const char *str, const char *fmt, int ops, ...);

int
cjson_parse_ptr(const char *begin, const char *end, const char *fmt, int ops, ...);


int
cjson_match(const char *str,  struct cjson_match *fs, void *prv);

int
cjson_match_ptr(const char *begin, const char *end, struct cjson_match *fs, void *prv);



int
cjson_map(const char *str,  struct cjson_map *fs, void *prv);

int
cjson_map_ptr(const char *begin, const char *end, struct cjson_map *fs, void *prv);




#ifdef __cplusplus
}
#endif

#endif


