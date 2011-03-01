//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#include "EfsFile.h"

#include "EfsWin32File.h"
#include "EfsPosixFile.h"

//	=============================================================
//	EfsFile::GetInstance

EfsFile* EfsFile::GetInstance()
{
#ifdef WIN32
	return new EfsWin32File();
#else
	return new EfsPosixFile();
#endif // WIN32
}

