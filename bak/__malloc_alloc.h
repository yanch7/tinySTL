#pragma once

#include <cstdlib>
#include <new>

namespace TinySTL {
	typedef void(*malloc_handler)();

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
}
