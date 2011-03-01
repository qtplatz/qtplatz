//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __Efs_h__
#define __Efs_h__

#include "EfsConfig.h"
#include "EfsDefs.h"
#include "EfsFile.h"
#include "FreesBTree.h"
#include "JFileStream.h"

///
/// General class for Adaptive File System
///

class Efs
{
public:

	Efs();
	~Efs();

	///
	/// Mount existing file system with specified access rights
	bool mount( const unsigned short *path );

	///
	/// Unmount filesystem
	void umount();

	///
	/// Create new or open existing file in the system.
	/// returns 0 if creation failed
	int createFile( const char *name, int fileAccess, int createDisp );

	///
	/// Delete file with specified filename
	bool deleteFile( const char *name );

	///
	/// Close specified file and free allocated resources
	void closeFile( int fd );

	// Change file info
	gint64 seekFile( int fd, gint64 newPos );
	bool rename( const char* oldName, const char *newName );
	bool changeFileAttrs( int fd, int newAttrs );

	// File info
	bool isExists( const char *name ) const;
	bool atEnd( int fd ) const;
	gint64 filePos( int fd ) const;
	gint64 fileSize( int fd ) const;
	bool queryFileInfo( int fd, EfsFileInfo &fileInfo ) const;
	int fileAttrs( int fd ) const;
	void listFiles( EfsFilenameList &retList );

	// File system info
	gint64 size() const;
	void info( EfsInfoStruct &info );
	float fragDegree();
	int version() const;

	// File operations
	bool resize( int fd, gint64 newSize );

	bool writeFile( int fd, const char *buf, int bufSize );
	int readFile( int fd, char *buf, int bufSize );

	// Adaptive File System error code return
	EfsErrorCode lastErrorCode() const;

	///
	/// Set growth factor for file system resizing
	void setGrowthFactor( gint64 factor );

	///
	/// Set callback function for call when space is growing
	void setCallback( EfsCallback *cback );

	///
	/// Defragment file system
	bool defrag();

	// Set and get general overhead information
	void setOverheadInfo( const EfsOverhead &general );
	void getOverheadInfo( EfsOverhead &general ) const;

	///
	/// Recreate file system, for example when one is damaged.
	/// Returns true if recreate and mount was successful
	bool recreate();

	///
	/// Return path to the file system 
	QString path() const;

	///
	/// Set encryptor for deal with afs data
	void setEncryptor( EfsEncryptor *encryptor );

	///
	/// Get current afs encryptor
	EfsEncryptor* getEncryptor();

#if AFS_DEBUG
	void debugWrite();
#endif // AFS_DEBUG

protected: // Methods

	bool getMaxFreeChunk( EfsFragment &chunk );
	bool getMinFreeChunk( EfsFragment &chunk, int size );

	bool growFS();

	int allocateSpace( EfsFragment &chunk, const EfsFragment &need );
	bool smartAllocate( int preferAddr, int needBlocks, 
		EfsAllocMethod allocMethod, EfsFragmentList &retList );

	void removeFreeSpace( int blockAddr, int blockCount );
	void deallocateSpace( int blockAddr, int blockCount );
	void deallocateLiberalSpace( EfsFitCache::iterator &iter );

	bool makeRoom( EfsFitCache::iterator &iter, gint64 newSize );
	bool ensureSpace( int blocks );

	int findFile( const char *name ) const;

	// Block read/write logic
	bool readBlock( int blockAddr, char *buf, int bufSize, int blockOffset );
	bool writeBlock( int blockAddr, const char *buf, int bufSize, 
		int blockOffset, bool appendData );

	bool readFit( int addr, EfsFit &entry );
	bool writeFit( int addr, const EfsFit &entry, bool checkSi = false );

	void verifyFSStructure();
	void removeFromFreeSpaceMap( EfsFragment &chunk );

	void deleteFileOS( const QString &path );
	void renameFileOS( const QString &from, const QString &to );

protected: // Data members

	friend class EfsBTreeController< EfsTreeNameNodeSize, EfsNameKey, int >;
	friend class EfsBTreeController< EfsTreePtrNodeSize, int, int >;

	QString path_;
	EfsFile *fs_;

	typedef BTreeAlgorithms< EfsTreeNameNodeSize, EfsNameKey, int, EfsBTreeController > FitsBTreeType;

	mutable FitsBTreeType fits_;
	FreeSizesMap freeSizes_;
	FreePtrsMap freeMemPtrs_;
	FreesBTree freePtrs_;

	// Cache of opened fits
	EfsFitCache fitCache_;

	// Opened files
	EfsHandleMap handles_;
	int handleCntr_;
	EfsOverhead overhead_;

	mutable EfsErrorCode lastErrorCode_;
	EfsCallback *callback_;
	EfsEncryptor *encryptor_;

	int ver_;

	// Buffers
	char readBuffer_[ EfsDefaultBlockSize + 1 ];
	int readBufAddr_;

	// Multithread support
	AFS_MULTITHREAD_SUPPORT;

private:

	// Don't permit copy file systems
	Efs( const Efs &copy );
	Efs& operator=( const Efs &rv );

};

#endif // __Efs_h__

