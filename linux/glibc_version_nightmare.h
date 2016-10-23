#if defined(__GNUC__) && defined(__LP64__)  /* only under 64 bit gcc */
#include <features.h>       /* for glibc version */
#if defined(__GLIBC__) && (__GLIBC__ == 2) && (__GLIBC_MINOR__ >= 14)
/* force mempcy to be from earlier compatible system */
__asm__(".symver memcpy,memcpy@GLIBC_2.2.5");
#endif
#undef _FEATURES_H      /* so gets reloaded if necessary */
#endif
