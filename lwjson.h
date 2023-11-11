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
 * @param fmt ".dumyarray[2][1].employe.name"
 * @param fmt "[0][2].employ.name"
 * */

#define EFMT	1
#define ESTR	2
#define EFLAG	3
#define ENFND	4
#define EBUFF	5
#define EPTR	6
#define EALLOC	6

#define O_BUFF	0
#define O_PTR	1
#define O_ALLOC	2

int
lwjson_parse(const char *str, const char *fmt, int ops, ...);

#ifdef __cplusplus
}
#endif

#endif


