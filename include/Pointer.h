#ifndef NTR_POINTER_H
#define NTR_POINTER_H

namespace ntr
{
	template <typename T> class Pointer;
	template <typename T> class ConstPointer;
	template <typename T> class ScopedPointer;

	// Wrapper for T*.
	template <typename T>
	class Pointer
	{
		friend class ConstPointer<T>;
		friend class ScopedPointer<T>;

	public:

		Pointer() = default;
		Pointer(T* p);
		Pointer(const Pointer<T>& p);
		Pointer(const ScopedPointer<T>& p); // note: pass scoped-pointers by reference, otherwise a temporary scoped-pointer copy may unintentionally delete data

		T& operator*();
		const T& operator*() const;
		T* operator->();
		const T* operator->() const;
		operator bool() const;
		operator T* () const;
		void operator delete(void* p);
		bool operator!() const;
		bool operator==(const T* p) const;
		bool operator==(const Pointer<T> p) const;
		bool operator==(const ConstPointer<T> p) const;
		bool operator==(const ScopedPointer<T>& p) const;
		bool operator!=(const T* p) const;
		bool operator!=(const Pointer<T> p) const;
		bool operator!=(const ConstPointer<T> p) const;
		bool operator!=(const ScopedPointer<T>& p) const;

		T* get() const;

	private:

		T* mPtr;
	};

	// Wrapper for const T*.
	template <typename T>
	class ConstPointer
	{
		friend class Pointer<T>;
		friend class ScopedPointer<T>;

	public:

		ConstPointer() = default;
		ConstPointer(const T* p);
		ConstPointer(const Pointer<T> p);
		ConstPointer(const ConstPointer<T>& p);
		ConstPointer(const ScopedPointer<T>& p);

		const T& operator*() const;
		const T* operator->() const;
		operator bool() const;
		operator const T* () const;
		bool operator!() const;
		bool operator==(const T* p) const;
		bool operator==(const Pointer<T> p) const;
		bool operator==(const ConstPointer<T> p) const;
		bool operator==(const ScopedPointer<T>& p) const;
		bool operator!=(const T* p) const;
		bool operator!=(const Pointer<T> p) const;
		bool operator!=(const ConstPointer<T> p) const;
		bool operator!=(const ScopedPointer<T>& p) const;

		const T* get() const;

	private:

		const T* mPtr;
	};

	// Wrapper for T* that deallocates data when going out of scope.
	template <typename T>
	class ScopedPointer
	{
		friend class Pointer<T>;
		friend class ConstPointer<T>;

	public:

		ScopedPointer() = default;
		ScopedPointer(T* p);
		ScopedPointer(const Pointer<T> p);
		~ScopedPointer();

		T& operator*();
		const T& operator*() const;
		T* operator->();
		const T* operator->() const;

		operator bool() const;
		operator T* () const;
		void operator delete(void* p);
		bool operator!() const;
		bool operator==(const T* p) const;
		bool operator==(const Pointer<T> p) const;
		bool operator==(const ConstPointer<T> p) const;
		bool operator==(const ScopedPointer<T>& p) const;
		bool operator!=(const T* p) const;
		bool operator!=(const Pointer<T> p) const;
		bool operator!=(const ConstPointer<T> p) const;
		bool operator!=(const ScopedPointer<T>& p) const;

		T* get() const;

	private:

		T* mPtr;
	};
}

#include "Pointer.hpp"

#endif
