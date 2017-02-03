#pragma once

#include <new>
#include <cstring>
#include <cstdlib>

namespace TinySTL {
	typedef void(*malloc_handler)();

	//一级内存分配器
	class __malloc_alloc {
	private:
		//以下函数用于处理内存不足的情况
		static void *oom_malloc(size_t);
		static void *oom_realloc(void *, size_t);
		//当内存分配失败时，执行此函数
		static malloc_handler __malloc_alloc_oom_handler;
	public:
		static void *allocate(size_t n);
		//deallocate与reallocate都有一个未使用到的参数
		//设置这个参数是为了与__default_alloc中的同名函数保持一致
		static void deallocate(void *p, size_t);
		static void *reallocate(void *p, size_t, size_t new_sz);

		//仿C++的set_new_handler,指定当分配内存失败时调用的函数
		//static void(*set_malloc_handler(void(*f)()));
		//函数的声明是接收一个void (*f)()函数指针作为参数，
		//返回值也是一个同样类型的函数指针。
		//为增加可读性可以改写为：
		// typedef void (*malloc_handler)();
		static malloc_handler set_malloc_handler(malloc_handler f) {
			malloc_handler old = __malloc_alloc_oom_handler;
			__malloc_alloc_oom_handler = f;
			return old;
		}
	};

	const size_t __MAX_BYTES = 128;
	const size_t __ALIGIN = 8;
	const size_t __FREELIST_SIZE = __MAX_BYTES / __ALIGIN;

	//二级空间配置器
	class __default_alloc {
	private:
		static size_t inline round_up(size_t bytes) {
			return (bytes + __ALIGIN - 1) & (~(__ALIGIN - 1));
		}

		static inline size_t index(size_t bytes) {
			return round_up(bytes) / __ALIGIN - 1;
		}

	private:
		union obj {
			union obj* free_list_link;
			char client_data[1];
		};

		static obj* free_list[__FREELIST_SIZE];
		static const int NOBJS = 20;
	public:
		//
		static void * refill(size_t n);

		//用于从内存池中取出空间，供free_list使用
		//返回一个大小为n * size的区块，如果空间不足，则n会减小
		static char * chunk_alloc(size_t size, int &n);

		static char *start_free; //内存池起始位置
		static char *end_free; //内存池结束位置
		static size_t heap_size; //从堆上总共申请的字节数

		static void * allocate(size_t n);
		static void deallocate(void *p, size_t n);
		static void *reallocate(void *p, size_t old_sz, size_t new_size);
	};

#ifdef __USE_MALLOC
	typedef __malloc_alloc alloc;
#else
	typedef __default_alloc alloc;
#endif

	template<class T, class Alloc>
	class simple_alloc {
	public:
		static T *allocate(size_t n) {
			return (0 == n) ? nullptr : static_cast<T*>Alloc::allocate(n * sizeof(T));
		}

		static T *allocate(void) {
			return static_cast<T*>Alloc::allocate(sizeof(T));
		}

		static void deallocate(T *p, size_t n) {
			Alloc::deallocate(p, n * sizeof(T));
		}

		static void deallocate(T *p) {
			Alloc::deallocate(p, sizeof(T));
		}
	};
}