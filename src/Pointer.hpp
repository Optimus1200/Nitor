#ifndef NTR_POINTER_HPP
#define NTR_POINTER_HPP

#include "Pointer.h"

namespace ntr
{
	//#################################################################################################
	//
	// POINTER IMPLEMENTATION
	//
	//#################################################################################################

	template <typename T>
	inline Pointer<T>::Pointer(T* p)
		: mPtr{ p }
	{

	}

	template <typename T>
	inline Pointer<T>::Pointer(const Pointer<T>& p)
		: mPtr{ p.mPtr }
	{
	}

	template <typename T>
	inline Pointer<T>::Pointer(const ScopedPointer<T>& p)
		: mPtr{ p.mPtr }
	{
	}
	
	template <typename T>
	inline T& Pointer<T>::operator*()
	{
		return *mPtr;
	}

	template <typename T>
	inline const T& Pointer<T>::operator*() const
	{
		return *mPtr;
	}
	
	template <typename T>
	inline T* Pointer<T>::operator->()
	{
		return mPtr;
	}

	template <typename T>
	inline const T* Pointer<T>::operator->() const
	{
		return mPtr;
	}
	
	template <typename T>
	inline Pointer<T>::operator bool() const
	{
		return mPtr != nullptr;
	}

	template <typename T>
	inline Pointer<T>::operator T* () const
	{
		return mPtr;
	}

	template <typename T>
	inline void Pointer<T>::operator delete(void* p)
	{
		delete mPtr;
	}
	
	template <typename T>
	inline bool Pointer<T>::operator!() const
	{
		return mPtr == nullptr;
	}
	
	template <typename T>
	inline bool Pointer<T>::operator==(const T* p) const
	{
		return mPtr == p;
	}

	template <typename T>
	inline bool Pointer<T>::operator==(const Pointer<T> p) const
	{
		return mPtr == p.mPtr;
	}

	template <typename T>
	inline bool Pointer<T>::operator==(const ConstPointer<T> p) const
	{
		return mPtr == p.mPtr;
	}

	template<typename T>
	inline bool Pointer<T>::operator==(const ScopedPointer<T>& p) const
	{
		return false;
	}

	template <typename T>
	inline bool Pointer<T>::operator!=(const T* p) const
	{
		return mPtr != p;
	}

	template <typename T>
	inline bool Pointer<T>::operator!=(const Pointer<T> p) const
	{
		return mPtr != p.mPtr;
	}

	template <typename T>
	inline bool Pointer<T>::operator!=(const ConstPointer<T> p) const
	{
		return mPtr != p.mPtr;
	}

	template<typename T>
	inline bool Pointer<T>::operator!=(const ScopedPointer<T>& p) const
	{
		return false;
	}
	
	template <typename T>
	inline T* Pointer<T>::get() const
	{
		return mPtr;
	}

	//#################################################################################################
	//
	// CONST POINTER IMPLEMENTATION
	//
	//#################################################################################################

	template <typename T>
	inline ConstPointer<T>::ConstPointer(const T* p)
		: mPtr{ p }
	{
	}

	template <typename T>
	inline ConstPointer<T>::ConstPointer(const Pointer<T> p)
		: mPtr{ p.get() }
	{
	}

	template<typename T>
	inline ConstPointer<T>::ConstPointer(const ConstPointer<T>& p)
		: mPtr{ p.mPtr }
	{
	}

	template<typename T>
	inline ConstPointer<T>::ConstPointer(const ScopedPointer<T>& p)
		: mPtr{ p.mPtr }
	{
	}

	template <typename T>
	inline const T& ConstPointer<T>::operator*() const
	{
		return *mPtr;
	}

	template <typename T>
	inline const T* ConstPointer<T>::operator->() const
	{
		return mPtr;
	}

	template <typename T>
	inline ConstPointer<T>::operator bool() const
	{
		return mPtr != nullptr;
	}

	template <typename T>
	inline ConstPointer<T>::operator const T* () const
	{
		return mPtr;
	}

	template <typename T>
	inline bool ConstPointer<T>::operator!() const
	{
		return mPtr == nullptr;
	}

	template <typename T>
	inline bool ConstPointer<T>::operator==(const Pointer<T> p) const
	{
		return mPtr == p.mPtr;
	}

	template <typename T>
	inline bool ConstPointer<T>::operator==(const ConstPointer<T> p) const
	{
		return mPtr == p.mPtr;
	}

	template <typename T>
	inline bool ConstPointer<T>::operator==(const ScopedPointer<T>& p) const
	{
		return mPtr == p.mPtr;
	}

	template <typename T>
	inline bool ConstPointer<T>::operator==(const T* p) const
	{
		return mPtr == p;
	}

	template <typename T>
	inline bool ConstPointer<T>::operator!=(const Pointer<T> p) const
	{
		return mPtr != p.mPtr;
	}

	template <typename T>
	inline bool ConstPointer<T>::operator!=(const ConstPointer<T> p) const
	{
		return mPtr != nullptr;
	}

	template <typename T>
	inline bool ConstPointer<T>::operator!=(const ScopedPointer<T>& p) const
	{
		return mPtr == p.mPtr;
	}

	template <typename T>
	inline bool ConstPointer<T>::operator!=(const T* p) const
	{
		return mPtr != p;
	}
	
	template <typename T>
	inline const T* ConstPointer<T>::get() const
	{
		return mPtr;
	}

	//#################################################################################################
	//
	// SCOPED POINTER IMPLEMENTATION
	//
	//#################################################################################################

	template <typename T>
	ScopedPointer<T>::ScopedPointer(T* p)
		: mPtr{ p }
	{

	}

	template <typename T>
	ScopedPointer<T>::ScopedPointer(const Pointer<T> p)
		: mPtr{ p.mPtr }
	{

	}
	
	template <typename T>
	inline ScopedPointer<T>::~ScopedPointer()
	{
		if (mPtr)
		{
			delete mPtr;
		}
	}
	
	template <typename T>
	inline T& ScopedPointer<T>::operator*()
	{
		return *mPtr;
	}
	
	template <typename T>
	inline const T& ScopedPointer<T>::operator*() const
	{
		return *mPtr;
	}
	
	template <typename T>
	inline T* ScopedPointer<T>::operator->()
	{
		return mPtr;
	}
	
	template <typename T>
	inline const T* ScopedPointer<T>::operator->() const
	{
		return mPtr;
	}

	template <typename T>
	inline ScopedPointer<T>::operator bool() const
	{
		return mPtr != nullptr;
	}

	template <typename T>
	inline ScopedPointer<T>::operator T* () const
	{
		return mPtr;
	}

	template <typename T>
	inline void ScopedPointer<T>::operator delete(void* p)
	{
		delete mPtr;
	}

	template <typename T>
	inline bool ScopedPointer<T>::operator!() const
	{
		return mPtr == nullptr;
	}

	template <typename T>
	inline bool ScopedPointer<T>::operator==(const T* p) const
	{
		return mPtr == p;
	}

	template <typename T>
	inline bool ScopedPointer<T>::operator==(const Pointer<T> p) const
	{
		return mPtr == p.mPtr;
	}

	template <typename T>
	inline bool ScopedPointer<T>::operator==(const ConstPointer<T> p) const
	{
		return mPtr == p.mPtr;
	}

	template <typename T>
	inline bool ScopedPointer<T>::operator==(const ScopedPointer<T>& p) const
	{
		return mPtr == p.mPtr;
	}

	template <typename T>
	inline bool ScopedPointer<T>::operator!=(const T* p) const
	{
		return mPtr != p;
	}

	template <typename T>
	inline bool ScopedPointer<T>::operator!=(const Pointer<T> p) const
	{
		return mPtr != p.mPtr;
	}

	template <typename T>
	inline bool ScopedPointer<T>::operator!=(const ConstPointer<T> p) const
	{
		return mPtr != p.mPtr;
	}

	template <typename T>
	inline bool ScopedPointer<T>::operator!=(const ScopedPointer<T>& p) const
	{
		return mPtr != p.mPtr;
	}
	
	template <typename T>
	inline T* ScopedPointer<T>::get() const
	{
		return mPtr;
	}
}

#endif