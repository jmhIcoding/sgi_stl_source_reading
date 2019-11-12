/*
2019/11/9 Minghao Jiang.
Email : jiangminghao@iie.ac.cn
����: �ڴ����Ļ�������

*/
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>


/*
�������Alloc������Ҫʵ�������ĸ�����,�����ϲ���á�
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
			//������Χ,ֱ����ϵͳ����
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
				return refill(ROUND_UP(n));//��ǰ�����Ѿ�û�п��е�chunk,������취���ڴ����������һ���ڴ�������
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
		//���ڴ������33��,��СΪn���ڴ�chunk,����һ�����ظ������ߣ�ʣ��32������free_list[index]�ϡ�
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
			//�Ѻ���nobjs-1��chunk����free_list��
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
			//������������33��chunk������
		{
			void * p = start_free;
			start_free += bytes_require;
			return p;
		}
		else if (bytes_left >= size_of_chunk)
			//��������33�������ǿ�������һ����
		{
			void * p = start_free;
			nobjs = bytes_left / size_of_chunk;
			start_free += nobjs * size_of_chunk;
			return p;
		}
		else
			//һ������������
		{
			if (bytes_left > 0)
				//�ڴ�ػ�����ͷ�ڴ�,������free_list��ȥ
			{
				int index = FREELIST_INDEX(bytes_left);
				//indexһ����[0,_NFREELIST)
				free_chunk* chunk = (free_chunk*)start_free;
				chunk->next_chunk = free_list[index];
				free_list[index] = chunk;
			}
			int bytes_get = bytes_require * 2 + heap_size & ~(__ALIGN);
			start_free = (char *)malloc(bytes_get);
			if (start_free == NULL)
				//��malloc�з����ڴ�ʧ��
			{
				for (int i = size_of_chunk; i < __MAX_SIZE; i += __ALIGN)
					//������һ��,������free_list��û�иպù���һ�����ʵ�
				{
					if (free_list[FREELIST_INDEX(i)] != NULL)
						//�պ��ҵ����Է���һ��size-of-chunk�Ŀ��п�,������free_listժ����,���ظ��ڴ��
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
		return (n + __ALIGN - 1) & ~(__ALIGN - 1);//��n����ȡֵΪ__ALIGN�ı���
	}
	static int FREELIST_INDEX(int n)
	{
		return ROUND_UP(n) / __ALIGN - 1;		  //ȷ���������ڴ��СnӦ����free_list��������������
	}
private:
	union free_chunk
	{
		free_chunk * next_chunk;
		char * client_data;
	};
private:
	static char * start_free;
	static char * end_free;//ά���ڴ�ص���������
	static size_t heap_size;
	enum {__ALIGN = 8, __MAX_SIZE = 2048};		//
	enum {__NFREE_LIST = __MAX_SIZE / __ALIGN};

	static free_chunk* free_list[__NFREE_LIST]; //���е�chunk����ȫ���ҽӵ�free_list��
};
//��ʼ���⼸������
char * __default_allocator::start_free = 0;
char * __default_allocator::end_free = 0;
size_t __default_allocator::heap_size = 0;
__default_allocator::free_chunk * __default_allocator::free_list[__default_allocator::__NFREE_LIST] = { 0 };
typedef __default_allocator alloc;