#ifdef __STL_ALLOC_TEST
#include <src\stl_alloc.h>
void unit_test()
{
	printf("test alloc now!\n");
	for (int i = 1; i < 5000; i++)
	{
		void *p = alloc::allocate(i);
		for (int j = 0; j < i; j++)
		{
			((char*)p)[j] += 1;
		}
		alloc::deallocate(p, i);
	}
	alloc::allocate(90);
	printf("alloc test end now!\n");
}
#endif