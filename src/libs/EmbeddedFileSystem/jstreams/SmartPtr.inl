//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
/// Author: Anton Fedoruk
///
//////////////////////////////////////////////////////////////////

//	===================================================================
//	SmartPtr< T >::SmartPtr()

template< class T > inline
SmartPtr< T >::SmartPtr()
:	pointer_( 0 ),
	ce_( 0 )
{}

//	===================================================================
//	SmartPtr< T >::SmartPtr( T* pointer )

template< class T > inline
SmartPtr< T >::SmartPtr( T *pointer )
:	pointer_( 0 ),
	ce_( 0 )
{
	*this = pointer;
}

//	===================================================================
//	SmartPtr< T >::SmartPtr( const SmartPtr< T >& copy )

template< class T > inline 
SmartPtr< T >::SmartPtr( const SmartPtr< T >& copy )
:	pointer_( 0 ),
	ce_( 0 )
{
	if ( !copy.ce_ ) return;

	pointer_ = copy.pointer_;
	ce_ = copy.ce_;

	ce_->counter_++;
}

template< class T > inline 
SmartPtr< T >::SmartPtr( SmartPtrTransportStruct< void > defVal )
:	pointer_( 0 ),
	ce_( 0 )
{
	if ( !defVal.ce_ ) return;

	pointer_ = ( T* ) defVal.ptr_;
	ce_ = defVal.ce_;

	ce_->counter_++;
}

//	===================================================================
//	SmartPtr< T >::~SmartPtr()

template< class T > inline
SmartPtr< T >::~SmartPtr()
{
	release();
}

//	===================================================================
//	SmartPtr< T >::relative

template< class T > inline
SmartPtrTransportStruct< void > SmartPtr< T >::relative()
{
	SmartPtrTransportStruct< void > spts;

	spts.ptr_ = pointer_;
	spts.ce_ = ce_;

	return spts;
}

//	===================================================================
//	SmartPtr< T >::operator=( const MRefPtr< void >* rhs )

template< class T > inline
SmartPtr< T >& SmartPtr< T >::operator=( SmartPtrTransportStruct< void > rhs )
{
	if ( pointer_ == ( T* ) rhs.ptr_ )
	{
		// If the object assigned already
		return *this;
	}

	if ( !isNull() )
	{
		release();
	}
	
	if ( !rhs.ptr_ || !rhs.ce_ )
	{
		return *this;
	}

	pointer_ = ( T* ) rhs.ptr_;
	ce_ = rhs.ce_;

	ce_->counter_++;

	return *this;
}

//	===================================================================
//	SmartPtr< T >::operator=( const SmartPtr< T >& rhs )

template< class T > inline
SmartPtr< T >& SmartPtr< T >::operator=( const SmartPtr< T > &rhs )
{
	if ( pointer_ == rhs.pointer_ )
	{
		// If the object assigned already
		return *this;
	}

	if ( !isNull() )
	{
		release();
	}
	
	if ( rhs.isNull() )
	{
		return *this;
	}

	pointer_ = rhs.pointer_;
	ce_ = rhs.ce_;

	ce_->counter_++;

	return *this;
}

//	===================================================================
//	SmartPtr< T >::operator=( T* pointer )

template< class T > inline 
SmartPtr< T >& SmartPtr<T>::operator=( T* pointer )
{
	if ( pointer == pointer_ )
	{
		// If we try to assign the same pointer
		return *this;
	}

	if( !isNull() )
	{
		release();
	}

	if( !pointer )
	{
		return *this;
	}

	pointer_ = pointer;

	ce_ = new SmartPtrCounterEntry;
	ce_->counter_ = 1;
	ce_->flags_ = 0;

	return *this;
}

//	===================================================================
//	SmartPtr< T >::operator T*

template< class T > inline
T* SmartPtr< T >::get() const
{
	return pointer_;
}

//	===================================================================
//	SmartPtr< T >::operator T*

template< class T > inline
bool SmartPtr< T >::isNull() const
{
	return !pointer_;
}

//	===================================================================
//	SmartPtr< T >::operator->

template< class T > inline
T* SmartPtr< T >::operator->() const
{
	return pointer_;
}

//	===================================================================
//	SmartPtr< T >::operator*

template< class T > inline
T& SmartPtr< T >::operator*() const
{
	return *pointer_;
}

//	===================================================================
//	SmartPtr< T >::operator!

template< class T > inline
bool SmartPtr< T >::operator !() const
{
	return isNull();
}

//	===================================================================
//	SmartPtr< T >::release

template < class T > inline
void SmartPtr< T >::release()
{
	if ( pointer_ && ce_ )
	{
		ce_->counter_--;

		// If count is null, then delete it
		if ( 0 == ce_->counter_ )
		{
			if ( 0 == ce_->flags_ )
			{
				delete pointer_;
			}

			delete ce_;
		}

		pointer_ = 0;
		ce_ = 0;
	}
}
