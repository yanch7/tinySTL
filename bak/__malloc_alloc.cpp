#include "__malloc_alloc.h"

namespace TinySTL {
	void * __malloc_alloc::allocate(size_t n) {
		void *result = malloc(n);
		if (nullptr == result) result = oom_malloc(n);
		return result;
	}

	void __malloc_alloc::deallocate(void *p, size_t) {
		free(p);
	}

	void * __malloc_alloc::reallocate(void *p, size_t, size_t new_sz) {
		void *result = realloc(p, new_sz);
		if (nullptr == result) {
			result = oom_realloc(p, new_sz);
		}
		return result;
	}

	//初始化为nullptr
	malloc_handler __malloc_alloc::__malloc_alloc_oom_handler = nullptr;

	void* __malloc_alloc::oom_malloc(size_t n) {
		void(*my_malloc_handler)();
		void *result;

		while (true) {
			my_malloc_handler = __malloc_alloc_oom_handler;

			//如果未设置处理函数，直接抛出异常
			if (nullptr == __malloc_alloc_oom_handler) {
				throw std::bad_alloc();
			}
			(*my_malloc_handler)();
			result = malloc(n);
			if (result) return result;
		}
	}

	void* __malloc_alloc::oom_realloc(void *p, size_t new_sz) {
		malloc_handler my_handler;
		void *result;

		while (true) {
			if (nullptr == my_handler) {
				throw std::bad_alloc();
			}

			(*my_handler)();
			result = realloc(p, new_sz);
			if (result) return result;
		}
	}
}