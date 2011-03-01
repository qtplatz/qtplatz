//////////////////////////////////////////////////////////////////
///
/// (C) 2007: ScalingWeb.com
///
/// Author: Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __StreamBTreeController_h__
#define __StreamBTreeController_h__

#include <set>
#include <QSet>
#include <QMap>
#include <map>

#include "BTreeNode.h"
#include "JStream.h"
#include "JBufferedOutputStream.h"
#include "JBufferedInputStream.h"

const int constRemovedAddr = sizeof( int )*2;

///
/// BTreeController with stream as storage implementation
///

template< int NodeSize, typename KeyT, typename DataT >
class StreamBTreeController
{
public:

	typedef BTreeNode< NodeSize, KeyT, DataT > NodeType;

	struct CacheData
	{
		CacheData():
		node( 0 )
		{}

		CacheData( time_t lastAccess ):
		node( 0 )
		{}

		CacheData( NodeType *innode ):
		node( innode )
		{}

		NodeType *node;
	};

	typedef std::set< int > FreeChunksSet;
	typedef QMap< int, CacheData > ChunkCacheMap;

	StreamBTreeController();

	///
	/// Open stream-based BTree storage
	bool open( JStream *stor, guint maxCacheSize, JStream *journal = 0 );

	///
	/// Returns true if tree is opened
	bool isOpen();

	///
	/// Flush changes without cache releasing
	void flushChanges();

	///
	/// Release all cache if size of cache less then max accepted
	void releaseCache();

	///
	/// Clear all cached node, except root node
	void clearCache();

	///
	/// Release only one node
	void releaseNode( int addr );

	///
	/// Return current cache size
	guint cacheSize() const;

	///
	/// Set max accepted cache size
	void setMaxCacheSize( guint maxSize );

	///
	/// Return max accepted cache size
	guint getMaxCacheSize();

	///
	/// Returns true if node is in cache
	bool nodeInCache( int addr );

	///
	/// Returns internal storage pointer
	JStream *storage();

	///
	/// Clear tree fully
	bool clear();

	///
	/// Get root pointer
	typename NodeType* root();

	///
	/// Load node is specified
	bool loadNode( NodeType **node, int addr );

protected:

	// Storage related operations
	NodeType* newNode();
	bool allocFreeSpace();
	bool readAllOffsets( NodeType *node );
	void deleteNode( NodeType *node );
	void saveNode( NodeType *node );
	int rootAddr();
	void rootAddr( int addr );

	bool checkJournal();

	void closeController();

	// Data
	NodeType *root_;
	JStream *storage_;
	JStream *journal_;

	FreeChunksSet freeChunks_;
	ChunkCacheMap cache_;
	guint maxCacheSize_;
	guint rootAddr_;
};

//	===================================================================
//	StreamBTreeController< NodeSize, KeyT, DataT >::StreamBTreeController

template< int NodeSize, typename KeyT, typename DataT >
StreamBTreeController< NodeSize, KeyT, DataT >::StreamBTreeController() : 
root_( 0 ),
storage_( 0 ),
journal_( 0 ),
maxCacheSize_( 0 ),
rootAddr_( 0 )
{}

//	===================================================================
//	StreamBTreeController< NodeSize, KeyT, DataT >::open

template< int NodeSize, typename KeyT, typename DataT >
bool StreamBTreeController< NodeSize, KeyT, DataT >::open( JStream *stor, 
								guint maxCacheSize, JStream *journal )
{
	if ( storage_ )
	{
		clearCache();
		delete root_;
		root_ = 0;
		closeController();
	}

	storage_ = stor;
	journal_ = journal;
	maxCacheSize_ = maxCacheSize;

	int sig = 0x77377377;

	if ( !checkJournal() )
	{
		int addr = rootAddr();
		if ( addr )
		{
			if ( !loadNode( &root_, addr ) )
			{
				return false;
			}
		}

		if ( !allocFreeSpace() ) return false;
	}

	if ( 0 == storage_->size() )
	{
		storage_->write( ( char* ) &sig, sizeof( sig ) );
		sig = 0;

		// Root addr
		storage_->write( ( char* ) &sig, sizeof( sig ) );
		// Removed addr
		storage_->write( ( char* ) &sig, sizeof( sig ) );
	}
	else
	{
		storage_->seek( 0 );
		int fileSig = 0;
		storage_->read( ( char* ) &fileSig, sizeof( fileSig ) );

		if ( sig != fileSig )
		{
			storage_->close();
			return false;
		}

		// Load removed nodes
		guint removedListAddr = 0;
		storage_->seek( constRemovedAddr );
		if ( sizeof( removedListAddr ) != storage_->read( 
			( char* ) &removedListAddr, sizeof( removedListAddr ) ) )
		{
			return false;
		}

		if ( removedListAddr )
		{
			if ( removedListAddr > storage_->size() ) return false;
			guint rlSize = storage_->size() - removedListAddr;
			if ( rlSize )
			{
				storage_->seek( removedListAddr );
				QByteArray rblock;
				rblock.resize( rlSize );
				storage_->read( rblock.data(), rlSize );

				guint curAddr = 0;
				for ( guint i = 0; i < rlSize/sizeof( guint ); i++ )
				{
					::memcpy( ( char* ) &curAddr, rblock.data() + sizeof( guint )*i,
						sizeof( curAddr ) );

					freeChunks_.insert( curAddr );
				}

				storage_->resize( removedListAddr );
			}
		}

		int addr = rootAddr();
		if ( addr )
		{
			if ( !loadNode( &root_, addr ) )
			{
				return false;
			}
		}
	}

	return true;
}

//	===================================================================
//	StreamBTreeController< NodeSize, KeyT, DataT >::checkJournal

template< int NodeSize, typename KeyT, typename DataT >
bool StreamBTreeController< NodeSize, KeyT, DataT >::checkJournal()
{
	if ( !journal_ || 0 == journal_->size() ) return true;

	JBufferedInputStream< JStream > journal( journal_ );
	journal.seek( 0 );

	guint label = 0;
	if ( sizeof( label ) != 
		journal.read( ( char* ) &label, sizeof( label ) ) )
	{
		journal.setSource( 0 );
		journal_->resize( 0 );
		return false;
	}

	if ( ~0 != label )
	{
		journal.setSource( 0 );
		journal_->resize( 0 );
		return false;
	}

	// Root
	label = 0;
	if ( sizeof( label ) != 
		journal.read( ( char* ) &label, sizeof( label ) ) )
	{
		journal.setSource( 0 );
		journal_->resize( 0 );
		return false;
	}

	if ( label )
	{
		storage_->seek( sizeof( int ) );
		storage_->write( ( char* ) &label, sizeof( label ) );
	}

	// Nodes
	NodeType node;
	while ( !journal.atEnd() )
	{
		journal.read( ( char* ) &node, sizeof( node ) );

		// Move out the node
		storage_->seek( node.addr_ );
		storage_->write( ( char* ) &node, sizeof( node ) );
	}

	journal.setSource( 0 );
	journal_->resize( 0 );
	return false;
}

//	===================================================================
//	StreamBTreeController< NodeSize, KeyT, DataT >::isOpen

template< int NodeSize, typename KeyT, typename DataT >
bool StreamBTreeController< NodeSize, KeyT, DataT >::isOpen()
{
	return ( storage_ != 0 );
}

//	===================================================================
//	StreamBTreeController< NodeSize, KeyT, DataT >::newNode

template< int NodeSize, typename KeyT, typename DataT >
typename StreamBTreeController< NodeSize, KeyT, DataT >::NodeType* 
				StreamBTreeController< NodeSize, KeyT, DataT >::newNode()
{
	int addr = 0;

	if ( !freeChunks_.empty() )
	{
		freeChunks_.erase( freeChunks_.begin() );
	}
	else
	{
		addr = storage_->size();
	}

	NodeType *newnode = new NodeType();
	newnode->addr_ = addr;
	newnode->flags_ = NodeType::NodeChanged;

	CacheData cacheData( newnode );
	saveNode( newnode );
	cache_[ addr ] = cacheData;

	return newnode;
}

//	===================================================================
//	StreamBTreeController< NodeSize, KeyT, DataT >::allocFreeSpace

template< int NodeSize, typename KeyT, typename DataT >
bool StreamBTreeController< NodeSize, KeyT, DataT >::allocFreeSpace()
{
	gint64 fullSize = storage_->size() - sizeof( int )*3;
	if ( !fullSize ) return true;

	qint32 nodeCount = fullSize / sizeof( NodeType );
	gint64 offset = sizeof( int )*3;

	for ( int i = 0; i < nodeCount; i++ )
	{
		freeChunks_.insert( offset );
		offset += sizeof( NodeType );
	}

	if ( !root_ ) return false;

	if ( !readAllOffsets( root_ ) ) return false;
	freeChunks_.erase( root_->addr_ );
	releaseCache();
	return true;
}

//	===================================================================
//	StreamBTreeController< NodeSize, KeyT, DataT >::readAllOffsets

template< int NodeSize, typename KeyT, typename DataT >
bool StreamBTreeController< NodeSize, KeyT, DataT >::readAllOffsets( 
				NodeType *node )
{
	if ( !node ) return true;

	if ( node->less_ )
	{
		NodeType *less;
		if ( !loadNode( &less, node->less_ ) ) return false;
		freeChunks_.erase( node->less_ );
		readAllOffsets( less );
		if ( less ) releaseNode( less->addr_ );

		// Iterate other children if we have a less one
		for ( int i = 0; i < node->count(); i++ )
		{
			NodeType *kid = 0;

			if ( node->elems_[ i ].link_ )
			{
				if ( !loadNode( &kid, node->elems_[ i ].link_ ) ) return false;
			}
			freeChunks_.erase( node->elems_[ i ].link_ );

			if ( kid )
			{
				readAllOffsets( kid );
				releaseNode( kid->addr_ );
			}
		}
	}

	return true;
}

//	===================================================================
//	StreamBTreeController< NodeSize, KeyT, DataT >::deleteNode

template< int NodeSize, typename KeyT, typename DataT >
void StreamBTreeController< NodeSize, KeyT, DataT >::deleteNode( NodeType* node )
{
	typename ChunkCacheMap::iterator iter = cache_.find( node->addr_ );

	if ( iter != cache_.end() )
	{
		cache_.erase( iter );
	}

	freeChunks_.insert( node->addr_ );
	delete node;
}

//	===================================================================
//	StreamBTreeController< NodeSize, KeyT, DataT >::loadNode

template< int NodeSize, typename KeyT, typename DataT >
bool StreamBTreeController< NodeSize, KeyT, DataT >::loadNode( 
			NodeType **node, int addr )
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
		storage_->seek( addr );
		storage_->read( ( char* ) *node, sizeof( NodeType ) );

		if ( addr == ( *node )->addr_ )
		{
			CacheData cacheData( *node );
			cache_[ addr ] = cacheData;
		}
	}
	else
	{
		*node = iter->node;
	}

	if ( ( *node )->addr_ != addr )
	{
		// Storage is corrupted!
		return false;
	}

	return true;
}

//	===================================================================
//	StreamBTreeController< NodeSize, KeyT, DataT >::saveNode

template< int NodeSize, typename KeyT, typename DataT >
void StreamBTreeController< NodeSize, KeyT, DataT >::saveNode( NodeType *node )
{
	node->flags_ = 0;
	storage_->seek( node->addr_ );
	storage_->write( ( const char* ) node, sizeof( NodeType ) );
}

//	===================================================================
//	StreamBTreeController< NodeSize, KeyT, DataT >::flushChanges

template< int NodeSize, typename KeyT, typename DataT >
void StreamBTreeController< NodeSize, KeyT, DataT >::flushChanges()
{
	if ( !storage_ ) return;

	// Journaling
	if ( journal_ )
	{
		JBufferedOutputStream< JStream > journal( journal_ );
		if ( journal.size() > 0 )
		{
			journal.resize( 0 );
		}

		bool firstTime = true;
		guint complete = 0;

		typename ChunkCacheMap::iterator iter = cache_.begin();
		for ( ; iter != cache_.end(); iter++ )
		{
			NodeType *node = iter->node;

			if ( NodeType::NodeChanged == node->flags_ )
			{
				if ( firstTime )
				{
					// Write completion label
					journal.seek( 0 );
					journal.write( ( char* ) &complete, sizeof( complete ) );
					journal.write( ( char* ) &rootAddr_, sizeof( rootAddr_ ) );
					firstTime = false;
				}

				node->flags_ = 0;
				journal.write( ( const char* ) node, sizeof( NodeType ) );
				node->flags_ = NodeType::NodeChanged;
			}
		}

		if ( !firstTime )
		{
			// Complete it
			journal.seekRaw( 0 );
			complete = ~0;
			journal.write( ( char* ) &complete, sizeof( complete ) );
			journal.flush();
		}
		journal.setSource( 0 );
	}

	// Actual saving
	typename ChunkCacheMap::iterator iter = cache_.begin();
	for ( ; iter != cache_.end(); iter++ )
	{
		if ( NodeType::NodeChanged == iter->node->flags_ )
		{
			// Save node
			saveNode( iter->node );
		}
	}

	// Write root
	if ( rootAddr_ )
	{
		storage_->seek( sizeof( int ) );
		storage_->write( ( char* ) &rootAddr_, sizeof( rootAddr_ ) );
		rootAddr_ = 0;
	}

	// Clear journal
	if ( journal_ ) journal_->resize( 0 );
}

//	===================================================================
//	StreamBTreeController< NodeSize, KeyT, DataT >::releaseCache

template< int NodeSize, typename KeyT, typename DataT >
void StreamBTreeController< NodeSize, KeyT, DataT >::releaseCache()
{
	if ( cacheSize() > maxCacheSize_ )
	{
		clearCache();
		return;
	}
}

//	===================================================================
//	StreamBTreeController< NodeSize, KeyT, DataT >::clearCache

template< int NodeSize, typename KeyT, typename DataT >
void StreamBTreeController< NodeSize, KeyT, DataT >::clearCache()
{
	flushChanges();

	typename ChunkCacheMap::iterator iter = cache_.begin();
	for ( ; iter != cache_.end(); iter++ )
	{
		if ( iter->node != root_ )
		{
			delete iter->node;
		}
	}

	cache_.clear();
	if ( root_ )
	{
		cache_[ root_->addr_ ] = CacheData( root_ );
	}
}

//	===================================================================
//	StreamBTreeController< NodeSize, KeyT, DataT >::releaseNode

template< int NodeSize, typename KeyT, typename DataT >
void StreamBTreeController< NodeSize, KeyT, DataT >::releaseNode( int addr )
{
	typename ChunkCacheMap::iterator iter = cache_.find( addr );

	if ( cache_.end() != iter )
	{
		if ( NodeType::NodeChanged == iter->node->flags_ )
		{
			flushChanges();
			releaseNode( addr );
			return;
		}

		if ( iter->node != root_ )
		{
			delete iter->node;
			cache_.erase( iter );
		}
	}
}

//	===================================================================
//	StreamBTreeController< NodeSize, KeyT, DataT >::rootAddr

template< int NodeSize, typename KeyT, typename DataT >
int StreamBTreeController< NodeSize, KeyT, DataT >::rootAddr()
{
	int addr = 0;
	storage_->seek( sizeof( int ) );
	storage_->read( ( char* ) &addr, sizeof( addr ) );
	return addr;
}

//	===================================================================
//	StreamBTreeController< NodeSize, KeyT, DataT >::closeController

template< int NodeSize, typename KeyT, typename DataT >
void StreamBTreeController< NodeSize, KeyT, DataT >::closeController()
{
	cache_.clear();

	if ( storage_ )
	{
		if ( !freeChunks_.empty() )
		{
			// Save removed nodes list
			guint rlistAddr = storage_->size();
			QByteArray rblock;
			FreeChunksSet::const_iterator iter = freeChunks_.begin();
			guint aaddr = 0;
			for ( ; iter != freeChunks_.end(); iter++ )
			{
				aaddr = *iter;
				rblock.resize( rblock.size() + sizeof( aaddr ) );
				::memcpy( rblock.data() + rblock.size() - sizeof( aaddr ), 
					( char* ) &aaddr, sizeof( aaddr ) );
			}
			freeChunks_.clear();

			storage_->seek( rlistAddr );
			storage_->write( rblock.data(), rblock.size() );

			storage_->seek( constRemovedAddr );
			storage_->write( ( char* ) &rlistAddr, sizeof( rlistAddr ) );
		}

		storage_->close();
		storage_ = 0;

		if ( journal_ )
		{
			journal_->close();
			journal_ = 0;
		}
	}
}

//	===================================================================
//	StreamBTreeController< NodeSize, KeyT, DataT >::rootAddr

template< int NodeSize, typename KeyT, typename DataT >
void StreamBTreeController< NodeSize, KeyT, DataT >::rootAddr( int addr )
{
	rootAddr_ = addr;
}

//	===================================================================
//	StreamBTreeController< NodeSize, KeyT, DataT >::rootAddr

template< int NodeSize, typename KeyT, typename DataT >
guint StreamBTreeController< NodeSize, KeyT, DataT >::cacheSize() const
{
	return sizeof( NodeType )*cache_.size();
}

//	===================================================================
//	StreamBTreeController< NodeSize, KeyT, DataT >::setMaxCacheSize

template< int NodeSize, typename KeyT, typename DataT >
void StreamBTreeController< NodeSize, KeyT, DataT >::setMaxCacheSize( guint maxSize )
{
	maxCacheSize_ = maxSize;
}

//	===================================================================
//	StreamBTreeController< NodeSize, KeyT, DataT >::getMaxCacheSize

template< int NodeSize, typename KeyT, typename DataT >
guint StreamBTreeController< NodeSize, KeyT, DataT >::getMaxCacheSize()
{
	return maxCacheSize_;
}

//	===================================================================
//	StreamBTreeController< NodeSize, KeyT, DataT >::getMaxCacheSize

template< int NodeSize, typename KeyT, typename DataT >
bool StreamBTreeController< NodeSize, KeyT, DataT >::nodeInCache( int addr )
{
	return cache_.end() != cache_.find( addr );
}

//	===================================================================
//	StreamBTreeController< NodeSize, KeyT, DataT >::getMaxCacheSize

template< int NodeSize, typename KeyT, typename DataT >
JStream* StreamBTreeController< NodeSize, KeyT, DataT >::storage()
{
	return storage_;
}

//	===================================================================
//	StreamBTreeController< NodeSize, KeyT, DataT >::clear

template< int NodeSize, typename KeyT, typename DataT >
bool StreamBTreeController< NodeSize, KeyT, DataT >::clear()
{
	// Clear cache
	typename ChunkCacheMap::iterator iter = cache_.begin();
	for ( ; iter != cache_.end(); iter++ )
	{
		delete iter->node;
	}

	cache_.clear();
	freeChunks_.clear();
	root_  = 0;

	JStream *treeStorage = storage_;
	storage_->resize( 0 );
	storage_ = 0;

	return open( treeStorage, maxCacheSize_ );
}

//	===================================================================
//	StreamBTreeController< NodeSize, KeyT, DataT >::root

template< int NodeSize, typename KeyT, typename DataT >
typename StreamBTreeController< NodeSize, KeyT, DataT >::NodeType* 
		StreamBTreeController< NodeSize, KeyT, DataT >::root()
{
	return root_;
}

#endif // __StreamBTreeController_h__
