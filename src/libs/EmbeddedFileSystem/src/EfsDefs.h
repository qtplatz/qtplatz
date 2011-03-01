//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////


#ifndef __EfsDefs_h__
#define __EfsDefs_h__

#include "PlatformTypes.h"
#include "BTreeAlgorithms.h"
#include "EfsEncryptor.h"
#include "EfsErrorCodes.h"
#include "EfsCallback.h"
#include <QString>
#include <QByteArray>
#include <vector>
#include <list>
#include <map>

const gint64 EfsDefaultGrowthFactor = 1024*1024*8;
const gint64 EfsDefaultBlockSize = 8192;
const int EfsDefaultFileBuffer = 0;
const int EfsDefaultMinFreeBlockCount = 1;	// Minimal size of free chunk for fs resizing
const int EfsDefaultMaxFilenameSize = 30;	// Size of the max object name in file system
const int EfsCurrentVersion = 6;			// Current Adaptive File System version
const int EfsMinChunkForDivide = 512;		// Very important value governs on fs work speed

const gint64 EfsFirstWritableAddr = 1;

// Calculate node sizes which are most suitable for one fs block
#include "EfsNameKey.h"

const int EfsFitsElemSize = sizeof( BTreeElement< EfsNameKey, int > );
const int EfsFitsNodeC = sizeof( BTreeNode< 1, EfsNameKey, int > ) - EfsFitsElemSize;
const int EfsTreeNameNodeSize = ( EfsDefaultBlockSize - EfsFitsNodeC )/EfsFitsElemSize;

const int EfsFreesElemSize = sizeof( BTreeElement< int, int > );
const int EfsFreesNodeC = sizeof( BTreeNode< 1, int, int > ) - EfsFreesElemSize;
const int EfsTreePtrNodeSize = ( EfsDefaultBlockSize - EfsFreesNodeC )/EfsFreesElemSize;

// Allocation methods
enum EfsAllocMethod
{
	EfsAllocLinear		= 0,
	EfsAllocQuadratic	= 1,
	EfsAllocService		= 2
};

const EfsAllocMethod EfsDefaultAllocMethod = EfsAllocQuadratic;

// File information structure
#pragma pack( 1 )
struct EfsFileInfo
{
	char fileName[ EfsDefaultMaxFilenameSize ];
	int attributes; // ReadOnly, Hidden, Encripted and so on.
	gint64 fileSize;
	int crc;
	char encryption[ 56 ];
	int creationTime;
	int lastChangeTime;
	int lastAccessTime;
};
#pragma pack()

// FIT service information structure
#pragma pack( 1 )
struct EfsFITServiceInfo
{
	int addr;
	int fitSize;
	int fragmentsCount;
	int allBlocks;
	int liberalSize;
	int reserved1;
	int reserved2;
	int reserved3;
};
#pragma pack()

const int EfsFISize = sizeof( EfsFileInfo );
const int EfsFITServiceSize = sizeof( EfsFITServiceInfo );

// File attributes
enum EfsFileAttributes
{
	EfsNormal		= 0,
	EfsReadOnly		= 1,
	EfsHidden		= 2,
	EfsEncripted	= 4
};

// File access
enum EfsAccessEnum
{
	EfsNoAccess		= 0,
	EfsReadAccess	= 1,
	EfsWriteAccess	= 2,
	EfsReadWrite	= 3
};

// File create disposition params
enum EfsCreateDisposition
{
	EfsCreateNew	= 1,
	EfsCreateAlways	= 2,
	EfsOpenExisting	= 3,
	EfsOpenAlways	= 4,
	EfsOpenTruncate	= 5
};

// Space fragment, with start block addr (in blocks space)
// and count of blocks in fragment
#pragma pack( 1 )
struct EfsFragment
{
	EfsFragment() : addr( 0 ), count( 0 ) {}

	int addr;
	int count;
};
#pragma pack()

const int EfsFragmentSize = sizeof( EfsFragment );

const int EfsSignatureSize = 8;
const char EfsSignature[ EfsSignatureSize ] = 
{
	( char ) 0x77, ( char ) 0x77, ( char ) 0x77, ( char ) 0x88, 
	( char ) 0x88, ( char ) 0x88, ( char ) 0x66, ( char ) 0xFD 
};

// Efs service block structure (which puts in 0 addr)
#pragma pack( 1 )
struct EfsServiceBlock
{
	char signature[ EfsSignatureSize ];
	int version;
	int mountedState; // By now: 1 - mounted, 0 - unmounted successfully
	int freesRootAddr;
	int fitsRootAddr;
};
#pragma pack()

// Represents handle of an open file
struct EfsHandle
{
	int access;
	gint64 offsetPtr;
	int fileBlockAddr;
};

// Overhead information structure with all necessary data
// for mounting and using Adaptive File System
#pragma pack( 1 )
struct EfsOverhead
{
	gint64 blockSize;
	gint64 growthFactor;
	int minFreeBlockCount;	// Minimal amount of blocks for space growth
	int defaultFileBuffer;	// Size of the liberal file space
	EfsAllocMethod allocationMethod;
};
#pragma pack()

const int EfsServiceBlockSize = sizeof( EfsServiceBlock );
const int EfsOverheadSize = sizeof( EfsOverhead );

const int EfsFreesRootOffset = EfsSignatureSize + sizeof( int )*2;
const int EfsFitsRootOffset = EfsFreesRootOffset + sizeof( int );

// Struct which contains all information about file system
struct EfsInfoStruct
{
	gint64 size;
	gint64 freeSpace;
	float freeQuotient;
	EfsOverhead overheadInfo;
};

#ifdef WIN32
// Use non deprecated function in VS 2005
#pragma warning(disable : 4996)
#endif // WIN32

#include "EfsFit.h"

typedef std::vector< QByteArray > EfsFilenameList;
typedef std::list< int > EfsHandleList;
typedef std::list< EfsFragment > EfsFragmentList;
typedef std::multimap< int, int, std::greater< int > > FreeSizesMap;
typedef std::map< int, int, std::greater< int > > FreePtrsMap;
typedef std::map< int, EfsFit > EfsFitCache;
typedef std::map< int, EfsHandle > EfsHandleMap;

#endif // __EfsDefs_h__
