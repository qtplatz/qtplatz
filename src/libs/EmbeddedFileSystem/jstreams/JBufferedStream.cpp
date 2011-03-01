//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#include "JBufferedStream.h"
#include <QtDebug>

JBufferedStream::JBufferedStream(JStream* source, const int bufSize, bool deleteSource)
: sourceStream(source),
  bufferSize(bufSize),
  deleteSourceStream(deleteSource),
  modified(false),
  count(0),
  bufferStart(0),		  // position in file of buffer
  bufferPosition(0)		  // position in buffer
{
	buffer = new char[bufferSize];
}

JBufferedStream::~JBufferedStream()
{
	delete [] buffer;

	if(deleteSourceStream)
	{
		delete sourceStream;
	}
}

void JBufferedStream::setDeleteSource(bool value)
{
	deleteSourceStream = value;
}

bool JBufferedStream::seek( gint64 position )
{
	if(position == bufferStart)
	{
		bufferPosition = 0;
	}
	else
	if(position > size())
	{				
		flushBuffer();
		bufferStart = size();
		bufferPosition = position - size();
		count = 0;	
		memset(buffer, 0, bufferSize);
	}
	else
	if (position > bufferStart && position < (bufferStart + bufferSize))
	{		
		bufferPosition = position > bufferStart ? position - bufferStart : position;  // seek within buffer
		Q_ASSERT(bufferPosition < bufferSize);
	}
	else 
	{
		if(modified)
		{
			flushBuffer(true);
		}

		if(sourceStream->seek(position))
		{
			//bufferPosition = position % bufferSize;
			//bufferStart = position - bufferPosition;			
			bufferStart = position;
			bufferPosition = 0;
			count = 0;				  // trigger refill() on read()
			refill();
			return true;
		}
		return false;
	}
	return true;
}

gint64 JBufferedStream::size()
{
	return sourceStream->size() < pos() ? pos() : sourceStream->size();
}

gint64 JBufferedStream::pos() const
{
	return bufferStart + bufferPosition;
}

//	======================================================
//	JFileStream::reset

void JBufferedStream::reset()
{
	seek(0);
	//sourceStream->reset();
}

//	======================================================
//	JBufferedStream::resize

void JBufferedStream::resize( gint64 newSize )
{
	sourceStream->resize( newSize );
	reset();
}


//	======================================================
//	JBufferedStream::flush

void JBufferedStream::flush()
{
	flushBuffer();
	sourceStream->flush();
}

void JBufferedStream::close()
{
	if(modified)
	{
		flushBuffer();
	}

	count = 0;
	bufferPosition = 0;
	bufferStart = 0;

	sourceStream->close();
}

gint64 JBufferedStream::read( char* pBuffer, gint64 bufLen )
{		
	if(count == 0)
	{
		refill();
	}

	gint64 toRead = bufLen;
		
	while(toRead > 0)
	{
		gint64 avail = count - bufferPosition;
		if(avail == 0)
		{
			if(!refill())
			{
				return bufLen - toRead;
			}
			avail = count - bufferPosition;
		}
		gint64 toCopy = toRead > avail ? avail : toRead;
		// Fill from buffer
		::memcpy(&pBuffer[bufLen - toRead], &buffer[bufferPosition], toCopy);
		
		bufferPosition += toCopy;
		toRead -= toCopy;
	}

	return bufLen;
}

void JBufferedStream::writeByte(const char b)
{
	if (bufferPosition >= bufferSize)
		flushBuffer();
	
	buffer[bufferPosition++] = b;

	if(count < bufferPosition)
	{
		count = bufferPosition;
	}

	modified = true;
}


gint64 JBufferedStream::write(const char* pBuffer, gint64 bufLen )
{

	//gint64 i = 0;
	//for (; i < bufLen; i++)
	//	writeByte(pBuffer[i]);

	//return i;
	gint64 toWrite = bufLen;
		
	while(toWrite > 0)
	{
		gint64 avail = bufferSize - bufferPosition;
		if(avail <= 0)
		{
			flushBuffer(true); // refill
			avail = bufferSize - bufferPosition;
		}
		gint64 toCopy = toWrite > avail ? avail : toWrite;
		// Fill from buffer
		::memcpy(&buffer[bufferPosition], &pBuffer[bufLen - toWrite], toCopy);
	
		bufferPosition += toCopy;

		if(count < bufferPosition)
		{
			count = bufferPosition;
		}
		
		Q_ASSERT(bufferPosition > 0);
		Q_ASSERT(count > 0);

		toWrite -= toCopy;
	}

	modified = true;

	return bufLen;
}

void JBufferedStream::flushBuffer(bool doRefill)
{

	if(pos() != sourceStream->pos())
	{
		sourceStream->seek(bufferStart);
	}

	gint64 written = sourceStream->write(buffer, count);

	// move start pointer
	bufferStart += written;
	bufferPosition = 0;
	count = 0;
	modified = false;

	if(!doRefill)
	{
		return;
	}

	if(bufferStart < sourceStream->size())
	{
		refill();
	}

}

bool JBufferedStream::refill()
{
	if(modified)
	{
		// flush without refill
		flushBuffer(false);
	}

	gint64 start = bufferStart + bufferPosition;
	gint64 end = start + bufferSize;
	
	if (end > sourceStream->size())				  // don't read past EOF
		end = sourceStream->size();

	count = end - start;

	if(count <= 0)
	{
		return false;
	}

	memset(buffer, 0, bufferSize);
	
	gint64 bytesRead = sourceStream->read(buffer, bufferSize);

	if(bytesRead = -1)
	{
		return false;
	}
	bufferStart = start;
	bufferPosition = 0;
	count = bytesRead;

	return true;
}

