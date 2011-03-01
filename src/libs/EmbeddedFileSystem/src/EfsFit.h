//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __EfsFit_h__
#define __EfsFit_h__

///
/// Block iterator for reading and writing 
/// through fragments
///

class EfsBlockIterator
{
public:

	EfsBlockIterator() :
	blockAddr( 0 )
	{}

	EfsFragment fragInfo;
	int fragIndex;
	int blockAddr;
	int blockCount;
};

///
/// Class EfsFit
///

class EfsFit
{
public:

	EfsFit();
	EfsFit( const EfsFit &rhv );
	EfsFit( gint64 afsBlockSize );

	~EfsFit();

	///
	/// Make new FIT with specified attributes
	void makeNew( int attributes );

	///
	/// Get FIT information
	inline EfsFITServiceInfo* serviceInfo() const;
	inline EfsFileInfo* fileInfo() const;
	inline const char* fit() const;
	inline const char* frags() const;

	///
	/// Return addr of current block
	void getCurrentBlockAddr( gint64 offsetPtr, EfsBlockIterator &iter ) const;

	///
	/// Add fragment to the end
	void addFragment( int blockPtr, int blockSize );

	///
	/// Set new file name
	void setFilename( const char *name );

	void setLastFragment( const EfsFragment &fragInfo );
	void lastFragment( EfsFragment &fragInfo ) const;

	void touch();
	void changed();

	void setServiceInfo( const EfsFITServiceInfo &si );
	int size() const;

	// Data in the FIT
	bool isEnoughSpace( int size, gint64 offsetPtr ) const;
	int fragsSpaceSize() const;
	void write( const char *buf, int size, gint64 &offsetPtr );
	int read( char *buf, int size, gint64 &offsetPtr );

	// Copy methods for using in maps
	void copy( const EfsFit &entry );
	EfsFit& operator=( const EfsFit &copy );
	void resize( int fitSize );

	// Data members
	int refs_;
	const gint64 afsBlockSize_;
	char *blocksData_;
};

#endif // __EfsFit_h__
