//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#include "JOutputStream.h"

//	=========================================================
//	JOutputStream::~JOutputStream

JOutputStream::~JOutputStream()
{
	close();
}

//	=========================================================
//	JOutputStream::write

int JOutputStream::write(char ch)
{
	return write( &ch, 1 );
}

//	=========================================================
//	JOutputStream::write

gint64 JOutputStream::write(const char* pBuffer )
{
	return write( pBuffer, strlen( pBuffer ) );
}

//	=========================================================
//	JOutputStream::pos

gint64 JOutputStream::pos() const
{
	return -1;
}

//	=========================================================
//	JOutputStream::reset

void JOutputStream::reset()
{
	seek( 0 );
}

//	=========================================================
//	JOutputStream::close

void JOutputStream::close()
{}

//	=========================================================
//	JOutputStream::flush

void JOutputStream::flush()
{}

//	=========================================================
//	JOutputStream::resize

void JOutputStream::resize( gint64 )
{}
