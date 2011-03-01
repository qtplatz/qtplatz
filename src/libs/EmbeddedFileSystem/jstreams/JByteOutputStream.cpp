//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#include "JByteOutputStream.h"


JByteOutputStream::JByteOutputStream(QByteArray &array, QIODevice::OpenModeFlag mode )
:	Buffer_( &array )
{
	Buffer_.open( mode );
}

inline void JByteOutputStream::close()
{
	Buffer_.close();
}

inline bool JByteOutputStream::seek(gint64 position)
{
	return Buffer_.seek( position );
}

inline void JByteOutputStream::reset()
{
	Buffer_.reset();
}

inline gint64 JByteOutputStream::pos() const
{
	return Buffer_.pos();
}

inline gint64 JByteOutputStream::size()
{
	return Buffer_.size();
}

inline gint64 JByteOutputStream::write(const char* pBuffer, gint64 bufLen)
{
	return Buffer_.write(pBuffer, bufLen);
}

void JByteOutputStream::resize( gint64 newSize )
{
	Buffer_.buffer().resize( newSize );
}
