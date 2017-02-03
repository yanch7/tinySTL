#pragma once

#include "Allocator.h"
#include "Construct.h"
#include <stdexcept>

namespace TinySTL
{
	template <class T, class Alloc = alloc>
	class vector
	{
	public:
		typedef T value_type;
		typedef value_type* pointer;
		typedef value_type& reference;
		typedef const value_type* const_pointer;
		typedef const value_type& const_reference;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;

		//vector使用数组作为内部存储，原生指针即为迭代器
		typedef value_type* iterator;
		typedef const value_type* const_iterator;

	protected:
		typedef simple_alloc<value_type, Alloc> data_allocator;
		iterator start;
		iterator finish;
		iterator end_of_storage;

		void insert_aux(iterator position, const T& x)
		{
			if (finish != end_of_storage)
			{
				construct(finish, *(finish - 1));
				++finish;
				uninitialized_copy_backward(position, finish - 2, finish - 1);
				*position = x;
			}
			else
			{
				const size_type old_size = size();
				const size_type len = (old_size == 0) ? 1 : 2 * old_size;

				iterator new_start = data_allocator::allocate(len);
				iterator new_finish = new_start;
				new_finish = uninitialized_copy(start, position, new_start);
				construct(new_finish, x);
				new_finish = uninitialized_copy(position, finish, new_finish);

				destroy(begin(), end());
				deallocate();

				start = new_start;
				finish = new_finish;
				end_of_storage = new_strat + len;
			}
		}

		void deallocate()
		{
			if (start)
			{
				data_allocator::deallocate(start, end_of_storage - start);
			}
		}

		void fill_initialize(size_type n, const T& value)
		{
			start = allocate_and_fill(n, value);
			finish = start + n;
			end_of_storage = finish;
		}

		iterator allocate_and_fill(size_type n, const T& x)
		{
			iterator result = data_allocator::allocate(n);
			uninitialized_fill_n(result, n, x);
			return result;
		}
	public:
		iterator begin() { return start; }
		iterator end() { return finish; }
		size_type size() { return size_type(end() - begin()); }
		size_type capacity() const
		{
			return size_type(end_of_storage - start);
		}

		bool empty() const
		{
			return begin() == end();
		}

		reference operator[] (size_type n)
		{
			if(n >= size())
			{
				throw std::out_of_range("out of range");
			}

			return *(begin() + n);
		}

		vector() : start(nullptr), finish(nullptr), end_of_storage(nullptr) {}
		vector(size_type n, const T& value) 
		{
			fill_initialize(n, value);
		}

		vector(int n, const T& value) { fill_initialize(n, value); }
		vector(long n, const T& value) { fill_initialize(n, value); }
		explicit vector(size_type n) { fill_initialize(n, T()); }
		~vector()
		{
			destroy(start, finish);
			deallocate();
		}

		reference front() { return *begin(); }
		reference back() { return *(end() - 1); }

		void push_back(const T& x)
		{
			if (finish != end_of_storage)
			{
				construct(finish, x);
				++finish;
			}
			else
			{
				insert_aux(end(), x);
			}
		}

		void pop_back()
		{
			--finish();
			destroy(finish);
		}

		//清除某位置上的元素，剩下的元素往前移动
		iterator erase(iterator position)
		{
			if (position + 1 != end())
			{
				uninitialized_copy(position + 1, finish, position);
			}
			--finish;
			destroy(finish);
		}

		iterator erase(iterator first, iterator last)
		{
			iterator i = uninitialized_copy(last, finish, first);
			destroy(finish);
			finish = finish - (last - first);
			return first;
		}

		void clear() { erase(begin(), end()); }
	};
}