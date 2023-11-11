#ifndef LWJSON_H
#define LWJSON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#ifdef DEBUG_MODE
# define PRINT		fprintf
# define NOP_PARAM
#else
# define PRINT		(void)
# define NOP_PARAM	(void)
#endif


#define FT_CHECK(status, ret, ...){			\
	if (!(status))					\
	{						\
		PRINT(NOP_PARAM stderr, __VA_ARGS__);	\
		return (ret);				\
	}						\
}

#define FT_ASSERT(status, ret, ...){			\
	if (!(status))					\
	{						\
		fprintf(stderr, __VA_ARGS__);		\
		exit(ret);				\
	}						\
}

/**
 * @param fmt ".dummyarray[2][1].employe.name"
 * @param fmt "[0][2].employ.name"
 * */

#define EFMT	1
#define ESTR	2
#define EOPS	3
#define EPARAMS	4
#define ENFND	5

#define EBUFF	6
#define EPTR	7
#define EALLOC	8

#define O_BUFF	0
#define O_PTR	1
#define O_ALLOC	2


/**
 *@param str `json string`
 *@param fmt `parser format:` `".dummyarray[2][1].employe.name"` or `"[0][2].employ.name"`
 *@param ops `options:` `O_BUFF` or `O_PTR` or `O_ALLOC`
 *@return `api status`
 *`int lwjson_parse(const char *str, const char *fmt, int ops, char *buff, int n);`
 *`int lwjson_parse(const char *str, const char *fmt, int ops, char **begin, char **end);`
 *`int lwjson_parse(const char *str, const char *fmt, int ops, char **alloc);`
 */
int
lwjson_parse(const char *str, const char *fmt, int ops, ...);

#ifdef __cplusplus
}
#endif

#endif


