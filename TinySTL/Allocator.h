#pragma once

#include "Alloc.h"

//以下为STL规范中，allocator的必要接口
namespace TinySTL {
	template <class T>
	class allocator {
	public:
		typedef T value_type;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
	public:
		pointer allocate(size_type n);

	};

	template <class T, class Alloc = alloc>
	class simple_alloc
	{
	public:
		static inline T* allocate(size_t n)
		{
			return (0 == n) ? nullptr : static_cast<T*>Alloc::allocate(n * sizeof(T));
		}

		static inline T* allocate(void)
		{
			return (T*)Alloc::allocate(sizeof(T));
		}

		static inline void deallocate(T *p, size_t n)
		{
			if (0 != n)
			{
				Alloc::deallocate(p, n * sizeof(T));
			}
		}

		static inline void deallocate(T *p)
		{
			Alloc::deallocate(p, sizeof(T));
		}
	};

	template <class ForwardIterator, class T>
	inline void uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& val)
	{
		ForwardIterator cur = first;
		for (; cur != last; ++cur)
		{
			construct(&*cur, val);
		}
	}

	template <class ForwardIterator, class Size, class T>
	inline ForwardIterator uninitialized_fill_n(ForwardIterator first, Size n, const T& val)
	{
		ForwardIterator cur = first;
		for (; n > 0; --n, ++cur)
		{
			construct(&*cur, val);
		}

		return first;
	}

	template <class ForwardIterator, class InputIterator>
	inline ForwardIterator uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result)
	{
		ForwardIterator cur = result;
		for (; first != last; ++first, ++cur)
		{
			construct(&*cur, *first);
		}

		return cur;
	}

	template <class InputIterator, class BidirectionalIterator>
	inline BidirectionalIterator uninitialized_copy_backward(InputIterator first, InputIterator last, BidirectionalIterator result)
	{
		InputIterator cur = result - 1;
		for (--last; last >= first; cur--, last--)
		{
			construct(cur, *last);
		}
		return cur;
	}
}
