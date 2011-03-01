//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////


#ifndef __EfsCallback_h__
#define __EfsCallback_h__

///
/// General interface for callback support in Efs
///

class EfsCallback
{
public:

	///
	/// This callback calls when Efs grow its space
	virtual gint64 growSpace( gint64 bytes ) = 0;

};

///
/// Template callback class for function as delegate for callback
///

template< typename T >
class EfsFuncCallback : public EfsCallback
{
public:

	void setCallback( T func );
	virtual gint64 growSpace( gint64 bytes );

protected:

	T func_;
};

///
/// Template callback class for class member method as 
/// delegate for callback
///

template< class T >
class EfsMethodCallback : public EfsCallback
{
public:

	typedef gint64 ( T::*FGrowSpace )( gint64 );

	void setCallback( T *obj, typename EfsMethodCallback< T >::FGrowSpace func );
	virtual gint64 growSpace( gint64 bytes );

protected:

	T *obj_;

	// Delegate for growSpace
	FGrowSpace func_;
};


//	=================================================
//	Implementation of EfsFuncCallback

template< typename T >
void EfsFuncCallback< T >::setCallback( T func )
{
	func_ = func;
}

template< typename T >
gint64 EfsFuncCallback< T >::growSpace( gint64 bytes )
{
	if ( func_ )
	{
		return func_( bytes );
	}

	return bytes;
}

//	=================================================
//	Implementation of EfsMethodCallback

template< class T >
void EfsMethodCallback< T >::setCallback( T *obj, 
			typename EfsMethodCallback< T >::FGrowSpace func )
{
	obj_ = obj;
	func_ = func;
}

template< class T >
gint64 EfsMethodCallback< T >::growSpace( gint64 bytes )
{
	if ( func_ )
	{
		return ( obj_->*func_ )( bytes );
	}

	return bytes;
}

#endif // __EfsCallback_h__

