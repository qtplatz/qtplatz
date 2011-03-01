//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
/// Author: Anton Fedoruk
///
//////////////////////////////////////////////////////////////////

#ifndef __MB_SmartPtr_h__
#define __MB_SmartPtr_h__

class SmartPtrCounterEntry
{
public:
	SmartPtrCounterEntry()
	{
		counter_ = 0;
		flags_ = 0;
	}

	int counter_;
	int flags_;
};

template< class Z >
class SmartPtrTransportStruct
{
public:

	SmartPtrTransportStruct()
	{
		ptr_ = 0;
		ce_ = 0;
	}

	SmartPtrTransportStruct( const SmartPtrTransportStruct< Z >& copy )
	{
		ptr_ = copy.ptr_;
		ce_ = copy.ce_;
	}

	Z *ptr_;
	SmartPtrCounterEntry *ce_;
};

/// Class SmartPtr

///
/// Implementation of a smart pointer
///

template< class T > class SmartPtr
{
public:

	// Default ctor
	SmartPtr();

	/// Assign ctor
	SmartPtr( T* pointer );
	SmartPtr( SmartPtrTransportStruct< void > defVal );

	/// Copy ctor
	SmartPtr( const SmartPtr< T >& copy );

	/// Dtor
	~SmartPtr();

	// Assign operator
	SmartPtr< T >& operator=( const SmartPtr< T >& rhs );
	SmartPtr< T >& operator=( SmartPtrTransportStruct< void > rhs );
	SmartPtr< T >& operator=( T* pointer );

	/// For class relatives
	SmartPtrTransportStruct< void > relative();

	/// Return pointer
	T* get() const;

	/// Is SmartPtr null object
	bool isNull() const;

	/// Call user method
	T* operator->() const;

	/// Return reference on pointer object
	T& operator*() const;

	/// Allow if ( !sp ) condition
	bool operator !() const;

	// Operators for each possible case

	inline friend bool operator==( const SmartPtr& lhs, const SmartPtr& rhs )
	{ return lhs.pointer_ == rhs.pointer_; }

	template < class U >
	inline friend bool operator==( const SmartPtr& lhs, const U* rhs )
	{ return lhs.pointer_ == rhs; }

	template < class U >
	inline friend bool operator==( const U *lhs, const SmartPtr &rhs )
	{ return lhs == rhs.pointer_; }

	inline friend bool operator==( const SmartPtr &lhs, const void* rhs )
	{ return lhs.pointer_ == rhs; }

	inline friend bool operator==( const void *lhs, const SmartPtr &rhs )
	{ return lhs == rhs.pointer_; }

	inline friend bool operator!=( const SmartPtr& lhs, const SmartPtr& rhs )
	{ return lhs.pointer_ != rhs.pointer_; }

	template < class U >
	inline friend bool operator!=( const SmartPtr& lhs, const U* rhs )
	{ return lhs.pointer_ != rhs; }

	template < class U >
	inline friend bool operator!=( const U *lhs, const SmartPtr& rhs )
	{ return lhs.pointer_ != rhs; }

	inline friend bool operator!=( const SmartPtr& lhs, const void* rhs )
	{ return lhs.pointer_ != rhs; }

	inline friend bool operator!=( const void *lhs, const SmartPtr& rhs )
	{ return lhs != rhs.pointer_; }

	void notAutoDelete( bool flag )
	{
		if ( !ce_ ) return;
		ce_->flags_ = flag ? 1 : 0;
	}

private:

	void release();

protected:

	T *pointer_;
	SmartPtrCounterEntry *ce_;

};

#include "SmartPtr.inl"

#endif // __MB_SmartPtr_h__

