//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __FreesBTree_h__
#define __FreesBTree_h__

#include "BTree.h"
#include "EfsBTreeController.h"

class Efs;

typedef BTreeAlgorithms< EfsTreePtrNodeSize, int, int, EfsBTreeController > FreesBTreeBase;

///
/// Class for storing free chunks infos
///

class FreesBTree : public BTreeAlgorithms< EfsTreePtrNodeSize, int, int, EfsBTreeController >
{
public:

	///
	/// Default ctor
	FreesBTree() :
	freeAmount_( 0 )
	{}

	///
	/// Open free ptrs storage
	virtual bool setFS( Efs *afs, int rootOffset, int blockSize )
	{
		freeAmount_ = 0;
		return FreesBTreeBase::setFS( afs, rootOffset, blockSize );
	}

	///
	/// Add new element to the storage
	virtual bool add( const int &key, const int &data )
	{
		if ( FreesBTreeBase::add( key, data ) )
		{
			freeAmount_ += data;
			return true;
		}

		return false;
	}

	///
	/// Remove element from the storage
	virtual bool remove( const int &key )
	{
		int freeSize = 0;
		if ( FreesBTreeBase::remove( key, freeSize ) )
		{
			freeAmount_ -= freeSize;
			return true;
		}

		return false;
	}

	///
	/// Change data element in the storage
	virtual void changeData( const int &key, const int &newData )
	{
		if ( !root_ )
		{
			return;
		}

		int index = 0, parentIndex = 0;
		bool found = false;
		NodeType *node = FreesBTreeBase::findNode( root_, key, index, parentIndex, found );

		if ( !node || index < 0 )
		{
			releaseCache( false );
			return;
		}

		if ( found )
		{
			// Key found
			freeAmount_ -= node->elems_[ index ].data_;
			node->elems_[ index ].data_ = newData;
			freeAmount_ += node->elems_[ index ].data_;
			releaseCache( true );
			return;
		}

		releaseCache( false );
	}

	///
	/// Return amount of freeSpace available in file system
	inline gint64 freeAmount()
	{
		return freeAmount_;
	}

	gint64 freeAmount_;
};

#endif // __FreesBTree_h__

