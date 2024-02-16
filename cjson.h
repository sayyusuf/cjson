#ifndef CJSON_H
#define CJSON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#define EFMT		1
#define ESTR		2
#define EOPS		3
#define EPARAMS		4
#define ENFND		5

#define EBUFF		6
#define EPTR		7
#define EALLOC		8

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

int
cjson_parse_str(const char *str, const char *fmt, int ops, ...);

int
cjson_parse_ptr(const char *begin, const char *end, const char *fmt, int ops, ...);


#ifdef __cplusplus
}
#endif

#endif


