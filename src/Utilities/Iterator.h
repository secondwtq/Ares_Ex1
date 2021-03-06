#ifndef ARES_ITERATOR_H
#define ARES_ITERATOR_H

#include <ArrayClasses.h>
#include <vector>

template<typename T>
class Iterator {
private:
	const T* items;
	size_t count;
public:
	Iterator() : items(nullptr), count(0) {}
	Iterator(const T* first, size_t count) : items(first), count(count) {}
	Iterator(const std::vector<T> &vec) : items(vec.data()), count(vec.size()) {}
	Iterator(const VectorClass<T> &vec) : items(vec.Items), count(vec.Capacity) {}
	Iterator(const DynamicVectorClass<T> &vec) : items(vec.Items), count(vec.Count) {}

	T at(size_t index) const {
		return this->items[index];
	}

	size_t size() const {
		return this->count;
	}

	const T* begin() const {
		return this->items;
	}

	const T* end() const {
		if(!this->valid()) {
			return nullptr;
		}

		return &this->items[count];
	}

	bool valid() const {
		return items != nullptr;
	}

	bool empty() const {
		return !this->valid() || !this->count;
	}

	bool contains(T other) const {
		return std::find(this->begin(), this->end(), other) != this->end();
	}

	operator bool() const {
		return !this->empty();
	}

	bool operator !() const {
		return this->empty();
	}

	const T& operator [](size_t index) const {
		return this->items[index];
	}

	template<typename Out>
	operator Iterator<Out>() const {
		// note: this does only work if pointer-to-derived equals pointer-to-base.
		// if derived has virtual methods and base hasn't, this will just break.
		static_assert(std::is_assignable<Out&, T>::value, "Cannot assign to Out from T.");
		return Iterator<Out>(reinterpret_cast<const Out*>(this->items), this->count);
	}
};

#endif
