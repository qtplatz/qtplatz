//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __ByteOutputStream_h__
#define __ByteOutputStream_h__

#include "JOutputStream.h"
#include <QByteArray>
#include <QBuffer>
#include <QIODevice>

// Class JByteInputStream

class JSTREAM_EXPORT JByteOutputStream : public JOutputStream
{
public:

	JByteOutputStream( QByteArray &array, QIODevice::OpenModeFlag mode = QIODevice::WriteOnly );

	inline virtual bool seek( gint64 position );

	inline virtual gint64 size();

	inline virtual gint64 pos() const;
	inline virtual void close();
	inline virtual void reset();

	using JOutputStream::write;

	inline virtual gint64 write(const char* pBuffer, gint64 bufLen);	

	void resize( gint64 newSize );

private:

	JByteOutputStream(const JByteOutputStream& rhs);            // cannot be copied
	JByteOutputStream& operator=(const JByteOutputStream& rhs); // nor assigned

private:

	QBuffer Buffer_;
};

#endif // __ByteOutputStream_h__