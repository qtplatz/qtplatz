//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////


#ifndef __EfsBTreeController_h__
#define __EfsBTreeController_h__

#include "BTreeNode.h"
#include <map>

class Efs;

template< int NodeSize, typename KeyT, typename DataT >
class EfsBTreeController
{
public:

	typedef BTreeNode< NodeSize, KeyT, DataT > NodeType;

	///
	/// Initialize all data member
	EfsBTreeController();

	///
	/// Install afs pointer and assign root node
	bool setFS( Efs *afs, int rootOffset, int blockSize );

	///
	/// Returns controller state
	bool isOpen() const;

	void releaseNode( int addr );
	guint cacheSize() const;
	void setMaxCacheSize( guint maxSize );
	bool nodeInCache( int addr );

protected:

	///
	/// Initialize root node
	bool initRoot();

	// Storage related operations
	NodeType* newNode();
	void deleteNode( NodeType *node );
	bool loadNode( NodeType **node, int addr );
	void saveNode( NodeType *node );
	void releaseCache( bool saveAll = true );
	int rootAddr();
	void rootAddr( int addr );

	void closeController();

	// Data
	NodeType *root_;
	Efs *efs_;
	int rootAddr_;
	int rootOffset_;
	int preferAddr_;
	int nodeBlockCount_;

	typedef std::map< int, NodeType* > ChunkCacheMap;
	ChunkCacheMap cache_;
};

//	===================================================================
//	EfsBTreeController< NodeSize, KeyT, DataT >::EfsBTreeController

template< int NodeSize, typename KeyT, typename DataT >
EfsBTreeController< NodeSize, KeyT, DataT >::EfsBTreeController() : 
	root_( 0 ),
	efs_( 0 ),
	rootAddr_( 0 ),
	rootOffset_( 0 ),
	nodeBlockCount_( 0 )
{}

//	===================================================================
//	EfsBTreeController< NodeSize, KeyT, DataT >::open

template< int NodeSize, typename KeyT, typename DataT >
bool EfsBTreeController< NodeSize, KeyT, DataT >::setFS( Efs *afs, int rootOffset, int blockSize )
{
	efs_ = afs;
	rootOffset_ = rootOffset;

	const int nodeSize = sizeof( NodeType );
	nodeBlockCount_ = ( nodeSize / blockSize ) + ( ( nodeSize % blockSize ) > 0 );

	return initRoot();
}

//	===================================================================
//	EfsBTreeController< NodeSize, KeyT, DataT >::newNode

template< int NodeSize, typename KeyT, typename DataT >
typename EfsBTreeController< NodeSize, KeyT, DataT >::NodeType* EfsBTreeController< NodeSize, KeyT, DataT >::newNode()
{
	if ( !efs_->ensureSpace( nodeBlockCount_ ) )
	{
		return 0;
	}

	EfsFragmentList fragments;
	if ( !efs_->smartAllocate( 0, nodeBlockCount_, EfsAllocService, fragments ) )
	{
		return 0;
	}

	if ( fragments.empty() )
	{
		return 0;
	}

	NodeType *newnode = new NodeType();
	newnode->addr_ = fragments.begin()->addr;

	saveNode( newnode );
	cache_[ newnode->addr_ ] = newnode;

	return newnode;
}

//	===================================================================
//	EfsBTreeController< NodeSize, KeyT, DataT >::deleteNode

template< int NodeSize, typename KeyT, typename DataT >
void EfsBTreeController< NodeSize, KeyT, DataT >::deleteNode( NodeType* node )
{
	typename ChunkCacheMap::iterator iter = cache_.find( node->addr_ );

	if ( iter != cache_.end() )
	{
		cache_.erase( iter );
	}

	efs_->deallocateSpace( node->addr_, nodeBlockCount_ );
	delete node;
}

//	===================================================================
//	EfsBTreeController< NodeSize, KeyT, DataT >::loadNode

template< int NodeSize, typename KeyT, typename DataT >
bool EfsBTreeController< NodeSize, KeyT, DataT >::loadNode( NodeType **node, int addr )
{
	if ( !addr )
	{
		*node = 0;
		return true;
	}

	typename ChunkCacheMap::iterator iter = cache_.find( addr );

	if ( iter == cache_.end() )
	{
		*node = new NodeType();
		if ( !efs_->readBlock( addr, ( char* ) *node, sizeof( NodeType ), 0 ) )
		{
			return false;
		}

		if ( addr == ( *node )->addr_ )
		{
			cache_[ addr ] = *node;
		}
	}
	else
	{
		*node = iter->second;
	}

	if ( ( *node )->addr_ != addr )
	{
		// Storage is corrupted!
		return false;
	}

	return true;
}

//	===================================================================
//	EfsBTreeController< NodeSize, KeyT, DataT >::saveNode

template< int NodeSize, typename KeyT, typename DataT >
void EfsBTreeController< NodeSize, KeyT, DataT >::saveNode( NodeType *node )
{
	efs_->writeBlock( node->addr_, ( const char* ) node, sizeof( NodeType ), 0, false );
}

//	===================================================================
//	EfsBTreeController< NodeSize, KeyT, DataT >::releaseCache

template< int NodeSize, typename KeyT, typename DataT >
void EfsBTreeController< NodeSize, KeyT, DataT >::releaseCache( bool saveAll )
{
	typename ChunkCacheMap::iterator iter = cache_.begin();

	for ( ; iter != cache_.end(); iter++ )
	{
		if ( saveAll )
		{
			saveNode( iter->second );
		}

		if ( iter->second != root_ )
		{
			delete iter->second;
		}
	}

	cache_.clear();
	if ( root_ )
	{
		cache_[ root_->addr_ ] = root_;
	}
}


template< int NodeSize, typename KeyT, typename DataT >
void EfsBTreeController< NodeSize, KeyT, DataT >::releaseNode( int addr )
{
	typename ChunkCacheMap::iterator iter = cache_.find( addr );

	if ( cache_.end() != iter )
	{
		saveNode( iter->second );

		if ( iter->second != root_ )
		{
			delete iter->second;
			cache_.erase( iter );
		}
	}
}

template< int NodeSize, typename KeyT, typename DataT >
void EfsBTreeController< NodeSize, KeyT, DataT >::setMaxCacheSize( guint )
{}

template< int NodeSize, typename KeyT, typename DataT >
bool EfsBTreeController< NodeSize, KeyT, DataT >::nodeInCache( int addr )
{
	return cache_.end() != cache_.find( addr );
}

//	===================================================================
//	EfsBTreeController< NodeSize, KeyT, DataT >::rootAddr

template< int NodeSize, typename KeyT, typename DataT >
int EfsBTreeController< NodeSize, KeyT, DataT >::rootAddr()
{
	if ( rootAddr_ )
	{
		return rootAddr_;
	}

	efs_->fs_->seek( rootOffset_ );
	if ( sizeof( rootAddr_ ) != efs_->fs_->read( ( char* ) &rootAddr_, sizeof( rootAddr_ ) ) )
	{
		return 0;
	}

	return rootAddr_;
}

//	===================================================================
//	EfsBTreeController< NodeSize, KeyT, DataT >::rootAddr

template< int NodeSize, typename KeyT, typename DataT >
void EfsBTreeController< NodeSize, KeyT, DataT >::rootAddr( int addr )
{
	efs_->fs_->seek( rootOffset_ );
	efs_->fs_->write( ( const char* ) &addr, sizeof( addr ) );
	rootAddr_ = addr;
}

//	===================================================================
//	EfsBTreeController< NodeSize, KeyT, DataT >::closeController

template< int NodeSize, typename KeyT, typename DataT >
void EfsBTreeController< NodeSize, KeyT, DataT >::closeController()
{
	cache_.clear();
	efs_ = 0;
	rootAddr_ = 0;
	rootOffset_ = 0;
	nodeBlockCount_ = 0;
}

//	===================================================================
//	EfsBTreeController< NodeSize, KeyT, DataT >::isOpen

template< int NodeSize, typename KeyT, typename DataT >
bool EfsBTreeController< NodeSize, KeyT, DataT >::isOpen() const
{
	return ( root_ != 0 );
}

//	===================================================================
//	EfsBTreeController< NodeSize, KeyT, DataT >::initRoot

template< int NodeSize, typename KeyT, typename DataT >
bool EfsBTreeController< NodeSize, KeyT, DataT >::initRoot()
{
	int addr = rootAddr();

	if ( !addr )
	{
		// Root is empty
		root_ = newNode();
		if ( !root_ )
		{
			return false;
		}

		rootAddr( root_->addr_ );
	}
	else
	{
		if ( !loadNode( &root_, addr ) )
		{
			return false;
		}
	}

	return true;
}


#endif // __EfsBTreeController_h__

