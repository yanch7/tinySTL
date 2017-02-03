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
}