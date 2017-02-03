#include "Alloc.h"
#include "IteratorClass.h"

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
		void *result;

		while (true) {
			//如果未设置处理函数，直接抛出异常
			if (nullptr == __malloc_alloc_oom_handler) {
				throw std::bad_alloc();
			}
			(*__malloc_alloc_oom_handler)();
			result = malloc(n);
			if (result) return result;
		}
	}

	void* __malloc_alloc::oom_realloc(void *p, size_t new_sz) {
		void *result;

		while (true) {
			if (nullptr == __malloc_alloc_oom_handler) {
				throw std::bad_alloc();
			}

			(*__malloc_alloc_oom_handler)();
			result = realloc(p, new_sz);
			if (result) return result;
		}
	}

	char* __default_alloc::start_free = nullptr;
	char* __default_alloc::end_free = nullptr;
	size_t __default_alloc::heap_size = 0;

	__default_alloc::obj* __default_alloc::free_list[__FREELIST_SIZE] = {
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr
	};

	void * __default_alloc::allocate(size_t bytes) {
		if (bytes > __MAX_BYTES) {
			return (__malloc_alloc::allocate(bytes));
		}

		obj ** my_free_list = free_list + index(bytes);
		obj *result = *my_free_list;
		if (result == nullptr) {
			void *r = refill(round_up(bytes));
			return r;
		}

		*my_free_list = result->free_list_link;
		return free;
	}

	void __default_alloc::deallocate(void *p, size_t bytes) {
		if (bytes > __MAX_BYTES) {
			__malloc_alloc::deallocate(p, bytes);
			return;
		}

		obj *q = static_cast<obj*>(p);
		obj **my_free_list = free_list + index(bytes);
		q->free_list_link = *my_free_list;
		*my_free_list = q;
	}

	char* __default_alloc::chunk_alloc(size_t size, int& n) {
		size_t total_size = n * size;
		size_t bytes_left = end_free - start_free;
		char *result = nullptr;

		if (bytes_left >= total_size) { //完全满足
			result = start_free;
			start_free += total_size;
			return result;
		}
		else if (bytes_left >= size) { //不能完全满足，但能至少满足一个对象
			n = bytes_left / size;
			total_size = n * size;
			result = start_free;
			start_free += total_size;
			return result;
		}
		else { //连一个对象都不能满足了
			   //首先物尽其用，将内存池中剩余的资源分到适当的free_list中
			if (bytes_left > 0) {
				obj **my_free_list = free_list + index(bytes_left);
				reinterpret_cast<obj *>(start_free)->free_list_link = *my_free_list;
				*my_free_list = reinterpret_cast<obj *>(start_free);
			}

			//然后试着调用malloc，一次分配（2 * total_size + 一定附加量）大小的空间
			//然后将total_size大小的分配出去，剩下的放到内存池中以后使用
			size_t bytes_to_get = 2 * total_size + round_up(heap_size >> 4);
			start_free = (char *)malloc(bytes_to_get);
			if (start_free) {
				heap_size += bytes_to_get;
				end_free = start_free + bytes_to_get;
				//递归调用自己，只把需求大小的分出去，剩余的留在内存池
				return(chunk_alloc(size, n));
			}
			else {	//堆上已经没有那么多空间了
					//先找找free_list上有无更大的空间，如果有，就找出一块来，
					//先放入内存池中。再调用自己，从内存池中取出相应的资源。
				for (size_t i = size; i <= __MAX_BYTES; i += __ALIGIN) {
					obj **my_free_list = free_list + index(i);
					obj *p = *my_free_list;
					if (p) {
						*my_free_list = p->free_list_link;
						start_free = reinterpret_cast<char *>(p);
						end_free += i;
						return chunk_alloc(size, n);
					}
				}

				//如果堆上不能分配，已有的链表中也没法榨取。就只能抛出异常了
				end_free = nullptr;
				start_free = nullptr;
				throw std::bad_alloc();
			}
		}
	}

	//函数在free_list所指空间不足时调用，refill调用chunk_alloc申请空间，
	//再将申请到的空间挂载再free_list上。所以成为refill
	void * __default_alloc::refill(size_t bytes) {
		int nobjs = NOBJS;
		char* chunk = chunk_alloc(bytes, nobjs);

		if (1 == nobjs) { //只申请到一个对象的大小，直接返回
			return chunk;
		}

		//申请到多个对象大小的节点，先挂载到free_list上
		obj** my_freee_list = free_list + index(bytes);
		//第一个返回给调用者
		obj *result = reinterpret_cast<obj *>(chunk);
		//从第二个节点开始挂入free_list
		*my_freee_list = reinterpret_cast<obj*>(chunk + bytes);

		obj* next = *my_freee_list;
		obj* current;
		for (int i = 1; i < nobjs; i++) {
			current = next;
			next = reinterpret_cast<obj*>(reinterpret_cast<char*>(next) + bytes);
			current->free_list_link = next;
		}
		current->free_list_link = nullptr;

		return result;
	}

	void * __default_alloc::reallocate(void *p, size_t old_sz, size_t new_sz) {
		if ((round_up(old_sz) > __MAX_BYTES) && (round_up(new_sz) > __MAX_BYTES)) {
			return __malloc_alloc::reallocate(p, old_sz, new_sz);
		}

		if (round_up(old_sz) == round_up(new_sz)) {
			return p;
		}

		void *result = allocate(new_sz);
		size_t bytes_to_copy = (old_sz > new_sz) ? new_sz : old_sz;
		memcpy(result, p, bytes_to_copy);
		deallocate(p, old_sz);
		return result;
	}
}

