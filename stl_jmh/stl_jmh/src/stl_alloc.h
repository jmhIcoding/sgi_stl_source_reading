/*
2019/11/9 Minghao Jiang.
Email : jiangminghao@iie.ac.cn
功能: 内存分配的基本功能

*/
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>


/*
最基本的Alloc，都需要实现以下四个函数,方面上层调用。
*/
template <class Alloc>
class SimpleAlloc
{
public:
	static void * allocate(int n)
	{
		return Alloc::allocate(n);
	}

	static void * reallocate(void * p, int n)
	{
		return Alloc::reallocate(p, n);
	}

	static void  deallocate(void *p)
	{
		return Alloc::deallocate(p);
	}

	static void  deallocate(void *p, int n)
	{
		return Alloc:deallocate(p, n);
	}
};

class __default_allocator
{
public:
	static void * allocate(int n)
	{
		if (n > __MAX_SIZE)
			//超出范围,直接向系统申请
		{
			void *p=(void *)malloc(n *sizeof(char));
			if (p == NULL) {
				printf("out of memory");
				exit(-1);
			}
			return p;
		}
		else
		{
			int index = FREELIST_INDEX(n);
			
			if(NULL==free_list[index])
			{
				return refill(ROUND_UP(n));//当前链表已经没有空闲的chunk,于是想办法从内存池里面申请一块内存下来。
			}
			else
			{
				void * p = free_list[index];
				free_list[index] = free_list[index]->next_chunk;
				return p;
			}
		}
	}
	static void * reallocate(void *p, int n);
	static void  deallocate(void *p);
	static void  deallocate(void *p, int n)
	{
		if (n > __MAX_SIZE)
		{
			free(p);
		}
		else
		{
			int index = FREELIST_INDEX(n);
			free_chunk * q = (free_chunk*)p;
			q->next_chunk = free_list[index];
			free_list[index] = q;
		}
	}
	static void * refill(int size_of_chunk)
		//先内存池申请33个,大小为n的内存chunk,其中一个返回给申请者，剩下32个挂在free_list[index]上。
	{
		int nobjs = 33;
		char * p = (char *)chunk_alloc(size_of_chunk, nobjs);
		if (nobjs == 1)
		{
			return p;
		}
		else if (nobjs > 1)
		{
			void * result = p;
			p += size_of_chunk;
			//把后面nobjs-1个chunk挂在free_list上
			int index = FREELIST_INDEX(size_of_chunk);
			for (int i = 1; i < nobjs; i++)
			{
				free_chunk * chunk = (free_chunk*)p;
				chunk->next_chunk = free_list[index];
				free_list[index] = chunk;
				p += size_of_chunk;
			}
			return result;
		}
	}
	static void * chunk_alloc(int size_of_chunk, int & nobjs)
	{
		size_t bytes_left = (size_t)(end_free - start_free);
		size_t bytes_require = (size_t)nobjs *size_of_chunk;
		if (bytes_left >= bytes_require)
			//可以满足申请33个chunk的条件
		{
			void * p = start_free;
			start_free += bytes_require;
			return p;
		}
		else if (bytes_left >= size_of_chunk)
			//不能满足33个，但是可以满足一部分
		{
			void * p = start_free;
			nobjs = bytes_left / size_of_chunk;
			start_free += nobjs * size_of_chunk;
			return p;
		}
		else
			//一个都不能满足
		{
			if (bytes_left > 0)
				//内存池还有零头内存,把它丢free_list上去
			{
				int index = FREELIST_INDEX(bytes_left);
				//index一定是[0,_NFREELIST)
				free_chunk* chunk = (free_chunk*)start_free;
				chunk->next_chunk = free_list[index];
				free_list[index] = chunk;
			}
			int bytes_get = bytes_require * 2 + heap_size & ~(__ALIGN);
			start_free = (char *)malloc(bytes_get);
			if (start_free == NULL)
				//从malloc中分配内存失败
			{
				for (int i = size_of_chunk; i < __MAX_SIZE; i += __ALIGN)
					//再挣扎一下,看其他free_list有没有刚好挂着一个合适的
				{
					if (free_list[FREELIST_INDEX(i)] != NULL)
						//刚好找到可以分配一个size-of-chunk的空闲块,把它从free_list摘下来,返回给内存池
					{
						start_free = (char*)free_list[FREELIST_INDEX(i)];
						end_free = start_free + i;
						free_list[FREELIST_INDEX(i)] = free_list[FREELIST_INDEX(i)]->next_chunk;
						return chunk_alloc(size_of_chunk, nobjs);
					}
				}
				//throw "out of memory";
			}
			else
			{
				void * p = start_free;
				end_free = start_free + bytes_get;
				start_free += size_of_chunk;
				heap_size += bytes_get;
			}
			return chunk_alloc(size_of_chunk, nobjs);
		}
	}
public:
	static int ROUND_UP(int n)
	{
		return (n + __ALIGN - 1) & ~(__ALIGN - 1);//把n向上取值为__ALIGN的倍数
	}
	static int FREELIST_INDEX(int n)
	{
		return ROUND_UP(n) / __ALIGN - 1;		  //确定待申请内存大小n应该在free_list的那条链表上找
	}
private:
	union free_chunk
	{
		free_chunk * next_chunk;
		char * client_data;
	};
private:
	static char * start_free;
	static char * end_free;//维护内存池的两个变量
	static size_t heap_size;
	enum {__ALIGN = 8, __MAX_SIZE = 2048};		//
	enum {__NFREE_LIST = __MAX_SIZE / __ALIGN};

	static free_chunk* free_list[__NFREE_LIST]; //空闲的chunk链表，全部挂接到free_list上
};
//初始化这几个变量
char * __default_allocator::start_free = 0;
char * __default_allocator::end_free = 0;
size_t __default_allocator::heap_size = 0;
__default_allocator::free_chunk * __default_allocator::free_list[__default_allocator::__NFREE_LIST] = { 0 };
typedef __default_allocator alloc;