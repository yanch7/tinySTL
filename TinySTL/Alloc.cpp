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

	//��ʼ��Ϊnullptr
	malloc_handler __malloc_alloc::__malloc_alloc_oom_handler = nullptr;

	void* __malloc_alloc::oom_malloc(size_t n) {
		void *result;

		while (true) {
			//���δ���ô�������ֱ���׳��쳣
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

		if (bytes_left >= total_size) { //��ȫ����
			result = start_free;
			start_free += total_size;
			return result;
		}
		else if (bytes_left >= size) { //������ȫ���㣬������������һ������
			n = bytes_left / size;
			total_size = n * size;
			result = start_free;
			start_free += total_size;
			return result;
		}
		else { //��һ�����󶼲���������
			   //�����ﾡ���ã����ڴ����ʣ�����Դ�ֵ��ʵ���free_list��
			if (bytes_left > 0) {
				obj **my_free_list = free_list + index(bytes_left);
				reinterpret_cast<obj *>(start_free)->free_list_link = *my_free_list;
				*my_free_list = reinterpret_cast<obj *>(start_free);
			}

			//Ȼ�����ŵ���malloc��һ�η��䣨2 * total_size + һ������������С�Ŀռ�
			//Ȼ��total_size��С�ķ����ȥ��ʣ�µķŵ��ڴ�����Ժ�ʹ��
			size_t bytes_to_get = 2 * total_size + round_up(heap_size >> 4);
			start_free = (char *)malloc(bytes_to_get);
			if (start_free) {
				heap_size += bytes_to_get;
				end_free = start_free + bytes_to_get;
				//�ݹ�����Լ���ֻ�������С�ķֳ�ȥ��ʣ��������ڴ��
				return(chunk_alloc(size, n));
			}
			else {	//�����Ѿ�û����ô��ռ���
					//������free_list�����޸���Ŀռ䣬����У����ҳ�һ������
					//�ȷ����ڴ���С��ٵ����Լ������ڴ����ȡ����Ӧ����Դ��
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

				//������ϲ��ܷ��䣬���е�������Ҳû��եȡ����ֻ���׳��쳣��
				end_free = nullptr;
				start_free = nullptr;
				throw std::bad_alloc();
			}
		}
	}

	//������free_list��ָ�ռ䲻��ʱ���ã�refill����chunk_alloc����ռ䣬
	//�ٽ����뵽�Ŀռ������free_list�ϡ����Գ�Ϊrefill
	void * __default_alloc::refill(size_t bytes) {
		int nobjs = NOBJS;
		char* chunk = chunk_alloc(bytes, nobjs);

		if (1 == nobjs) { //ֻ���뵽һ������Ĵ�С��ֱ�ӷ���
			return chunk;
		}

		//���뵽��������С�Ľڵ㣬�ȹ��ص�free_list��
		obj** my_freee_list = free_list + index(bytes);
		//��һ�����ظ�������
		obj *result = reinterpret_cast<obj *>(chunk);
		//�ӵڶ����ڵ㿪ʼ����free_list
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

