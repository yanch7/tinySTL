#pragma once

#include <new>
#include <cstring>
#include <cstdlib>

namespace TinySTL {
	typedef void(*malloc_handler)();

	//һ���ڴ������
	class __malloc_alloc {
	private:
		//���º������ڴ����ڴ治������
		static void *oom_malloc(size_t);
		static void *oom_realloc(void *, size_t);
		//���ڴ����ʧ��ʱ��ִ�д˺���
		static malloc_handler __malloc_alloc_oom_handler;
	public:
		static void *allocate(size_t n);
		//deallocate��reallocate����һ��δʹ�õ��Ĳ���
		//�������������Ϊ����__default_alloc�е�ͬ����������һ��
		static void deallocate(void *p, size_t);
		static void *reallocate(void *p, size_t, size_t new_sz);

		//��C++��set_new_handler,ָ���������ڴ�ʧ��ʱ���õĺ���
		//static void(*set_malloc_handler(void(*f)()));
		//�����������ǽ���һ��void (*f)()����ָ����Ϊ������
		//����ֵҲ��һ��ͬ�����͵ĺ���ָ�롣
		//Ϊ���ӿɶ��Կ��Ը�дΪ��
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

	//�����ռ�������
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

		//���ڴ��ڴ����ȡ���ռ䣬��free_listʹ��
		//����һ����СΪn * size�����飬����ռ䲻�㣬��n���С
		static char * chunk_alloc(size_t size, int &n);

		static char *start_free; //�ڴ����ʼλ��
		static char *end_free; //�ڴ�ؽ���λ��
		static size_t heap_size; //�Ӷ����ܹ�������ֽ���

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