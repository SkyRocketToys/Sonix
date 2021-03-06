#include <features.h>

#undef assert

#ifdef NDEBUG
#define	assert(x) (void)0
#else
//#define assert(x) ((void)((x) || (__assert_fail(#x, __FILE__, __LINE__, __func__),0)))
#define assert(x) ((void)((x) || (vAssertCalled(__FILE__, __LINE__, __func__),0)))
#endif

#ifndef __cplusplus
#define static_assert _Static_assert
#endif

#ifdef __cplusplus
extern "C" {
#endif

void __assert_fail (const char *, const char *, int, const char *);

#ifdef __cplusplus
}
#endif
