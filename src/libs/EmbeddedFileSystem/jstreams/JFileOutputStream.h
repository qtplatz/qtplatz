//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

# ifndef __JFileOutputStream__
# define __JFileOputputSteam__

#include "JOutputStream.h"
#include <QString>
#include <QFile>
#include <QByteArray>

/// Class JFileInputStream

class JSTREAM_EXPORT JFileOutputStream : public JOutputStream
{
public:

	JFileOutputStream(const QFile& file, QIODevice::OpenModeFlag mode = QIODevice::WriteOnly);
	JFileOutputStream(const QString& name, QIODevice::OpenModeFlag mode = QIODevice::WriteOnly);

	inline virtual gint64 size();
	inline virtual void close();

	using JOutputStream::write; 	// unhide inherited read methods

	virtual gint64 write( const char* pBuffer, gint64 bufLen );

	inline virtual void flush();		//flush output file's buffers

	inline virtual bool seek( gint64 position );
	inline virtual gint64 pos() const;
	inline virtual void reset();

	bool open(const QString& fileName, QIODevice::OpenModeFlag mode = QIODevice::WriteOnly);

	void resize( gint64 newSize );

private:
	JFileOutputStream(const JFileOutputStream& rhs);            // cannot be copied
	JFileOutputStream& operator=(const JFileOutputStream& rhs); // nor assigned

	QFile File_;
};

# endif //__JFileOutputStream__