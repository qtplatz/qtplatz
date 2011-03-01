//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#include "EfsEncryptor.h"

const char xorVal = 77;

//	==============================================
//	EfsEncryptor::encrypt

bool EfsEncryptor::encrypt( char* /*data*/, int /*dataSize*/ )
{
	//for ( int i = 0; i < dataSize; i++ )
	//{
	//	data[ i ] ^= xorVal;
	//}

	return true;
}

//	==============================================
//	EfsEncryptor::decrypt

bool EfsEncryptor::decrypt( char* /*data*/, int /*dataSize*/ )
{
	//for ( int i = 0; i < dataSize; i++ )
	//{
	//	data[ i ] ^= xorVal;
	//}

	return true;
}

EfsEncryptor EfsDefaultEncryptor;

