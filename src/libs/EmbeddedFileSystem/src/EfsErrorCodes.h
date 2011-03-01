//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __EfsErrorCodes_h__
#define __EfsErrorCodes_h__

enum EfsErrorCode
{
	EfsNoError = 0,
	EfsFileOpenFailed,
	EfsTestSignatureFailed,
	EfsVersionNotSupported,
	EfsBlockSizeNotSupported,
	EfsFITBeaconOpenFailed,
	EfsFreePtrsBeaconOpenFailed,
	EfsInsufficientRights,
	EfsSpaceAllocationFailed,
	EfsUnsupportedParam,
	EfsFileNotFound,
	EfsGrowFailed,
	EfsCorrupted,
	EfsMountFailed
};

#endif // __EfsErrorCodes_h__

