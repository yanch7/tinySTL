#pragma once

#include "__malloc_alloc.h"
#include <cstring>

namespace TinySTL {
	const size_t __MAX_BYTES = 128;
	const size_t __ALIGIN = 8;
	const size_t __FREELIST_SIZE = __MAX_BYTES / __ALIGIN;

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
}