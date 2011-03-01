//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#include "JInputStream.h"

//	=========================================================
//	JInputStream::~JInputStream

JInputStream::~JInputStream()
{
	close();
}

//	=========================================================
//	JInputStream::skip

gint64 JInputStream::skip( gint64 n )
{
	gint64 count = 0;
	while( count < n )
	{
		if ( EndOfFile == read() )
		{
			break;
		}

		count++;
	}
	return count;
}

//	=========================================================
//	JInputStream::read

int JInputStream::read()
{
	char buffer;
	if(read(&buffer, 1) == EndOfFile)
		return EndOfFile;
	else
		return (int)buffer;
}

//	=========================================================
//	JInputStream::readLine

gint64 JInputStream::readLine( char*, gint64 )
{
	return 0;
}

//	=========================================================
//	JInputStream::readLine

QByteArray JInputStream::readLine()
{
	return QByteArray();
}

//	=========================================================
//	JInputStream::atEnd

bool JInputStream::atEnd() const
{
	return true;
}

//	=========================================================
//	JInputStream::close

void JInputStream::close()
{}

//	=========================================================
//	JInputStream::pos

gint64 JInputStream::pos() const
{
	return -1;
}

//	=========================================================
//	JInputStream::reset

void JInputStream::reset()
{
	seek( 0 );
}

