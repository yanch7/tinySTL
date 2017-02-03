#pragma once

#include <cstdlib>
#include <new>

namespace TinySTL {
	typedef void(*malloc_handler)();

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
}
