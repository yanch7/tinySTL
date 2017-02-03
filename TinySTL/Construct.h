#pragma once

#include <new>

namespace TinySTL
{
	template <class T1, class T2>
	inline void construct(T1 *p, const T2 &value)
	{
		new (p) T1(value);
	}

	template <class T>
	inline void destroy(T *p)
	{
		p->~T();
	}

	template <class ForwardIterator>
	inline void destroy(ForwardIterator first, ForwardIterator last)
	{
		for (; first != last; ++first)
		{
			destroy(&*first);
		}
	}

	inline void destroy(char *, char *) {}
	inline void destroy(wchar_t*, wchar_t*) {}
}