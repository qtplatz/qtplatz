//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __FileInputStream_h__
#define __FileInputStream_h__

#include "JInputStream.h"
#include <QString>
#include <QFile>
#include <QByteArray>
#include "JUtils.h"

#ifdef WIN32
#include "windows.h"
#endif // WIN32

/// Class JFileInputStream

class JSTREAM_EXPORT JFileInputStream : public JInputStream
{
public:

	JFileInputStream();
	JFileInputStream(const QFile& file);
	JFileInputStream(const QString& name);
	~JFileInputStream();

	virtual gint64 size();
	virtual void close();

	using JInputStream::read; 	// unhide inherited read methods

	virtual gint64 read(char* pBuffer, gint64 bufLen);
	virtual gint64 readLine( char* data, gint64 maxSize );
	virtual QByteArray readLine();

	virtual bool atEnd() const;

	virtual bool seek( gint64 position );
	virtual gint64 pos() const;
	virtual void reset();

	time_t lastModified() const;

	bool open(const QString& fileName);

private:
	JFileInputStream(const JFileInputStream& rhs);            // cannot be copied
	JFileInputStream& operator=(const JFileInputStream& rhs); // nor assigned

#ifdef WIN32
	HANDLE hFile_;
#else
	QFile File_;
#endif // WIN32

};

#endif // __FileInputStream_h__

