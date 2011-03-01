//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __JStreamWrapper_h__
#define __JStreamWrapper_h__

#include <JStream.h>
#include <QIODevice>


class JStreamWrapper
	: public QIODevice
{

public:
	JStreamWrapper(JInputStream* stream)
		: mStream(stream), QIODevice()
	{		
	}

	~JStreamWrapper()
	{
	}

	virtual bool open( OpenMode mode )
	{
		QIODevice::open(mode);
		return true;
	}

	virtual bool isSequential () const
	{
		return false;
	}

	virtual void close()
	{
		QIODevice::close();
		mStream->close();
	}

	virtual bool atEnd () const
	{
		return mStream->atEnd();
	}

	virtual qint64 bytesAvailable () const
	{
		return size() - pos();
	}

	virtual bool canReadLine () const
	{
		return false;
	}

	qint64 peek(char *data, qint64 maxSize)
	{
		qint64 oldPos = pos();
		qint64 readBytes = read(data, maxSize);
		seek(oldPos);
		return readBytes;
	}

	virtual qint64 pos() const 
	{
		return mStream->pos();
	}

	virtual bool reset()
	{
		mStream->reset();
		return true;
	}	

	virtual bool seek(qint64 pos)
	{
		return mStream->seek(pos);
	}

	virtual qint64 size() const
	{
		return mStream->size();
	}

protected:
	virtual qint64 readData(char *data, qint64 maxSize)
	{
		return mStream->read(data, maxSize);
	}
	virtual qint64 writeData(const char *data, qint64 size)
	{
		return -1;
	}
	    
    private:
        JInputStream *mStream;     
};

#endif