#include <stdlib.h>
#include <string.h>
#include <FreeRTOS.h>

void *realloc(void *p, size_t n)
{
	return pvPortRealloc(p, n, MODULE_UNKNOW);
}

void *malloc (size_t size)
{
	return pvPortMalloc(size, GFP_KERNEL, MODULE_UNKNOW);
}

void free (void *addr)
{
	vPortFree(addr);
}

void *calloc(size_t m, size_t n)
{
	void *p;
	//size_t *z;
	if (n && m > (size_t)-1/n) {
		//errno = ENOMEM;
		return 0;
	}
	n *= m;
	p = malloc(n);
	if (!p) return 0;

	memset(p, 0, n);
#if 0
	/* Only do this for non-mmapped chunks */
	if (((size_t *)p)[-1] & 7) {
		/* Only write words that are not already zero */
		m = (n + sizeof *z - 1)/sizeof *z;
		for (z=p; m; m--, z++) if (*z) *z=0;
	}
#endif
	return p;
}
