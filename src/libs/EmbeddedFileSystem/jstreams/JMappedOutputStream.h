//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __JMappedOutputStream_h__
#define __JMappedOutputStream_h__

#include "JOutputStream.h"

// Class JMappedInputStream

class JSTREAM_EXPORT JMappedOutputStream : public JOutputStream
{
public:

	JMappedOutputStream( JOutputStream *stream, gint64 size, gint64 start = 0 );
	virtual ~JMappedOutputStream();

	///
	/// Set internal stream position to new value
	virtual bool seek( gint64 position );

	///
	/// Returns stream size
	inline virtual gint64 size();

	virtual gint64 write(const char* pBuffer, gint64 bufLen );

	///
	/// Returns current internal stream data pointer
	inline virtual gint64 pos() const;

	///
	/// Set internal stream pointer to beginning
	inline virtual void reset();

	///
	/// Close stream
	virtual void close();

	///
	///Flushes any buffered data
	inline virtual void flush();

	///
	/// Indicates stream status, retrurn false if write operation failed
	// otherwise return true
	inline bool isOk() const;

	///
	/// Says delete or not stream in destructor
	inline void setDeleteStream( bool isDelete );

	///
	/// allow to write over the size boundary
	inline void setResizable(bool isResizable );

	///
	/// Starts current position from which all actions will be made.
	/// This method sets the current stream position to the start.

	void setStart( gint64 start );

	///
	/// Set size for mapping.
	/// This method sets the current stream position to the start.

	void setSize( gint64 size );

	///
	/// Says delete or not stream in destructor
	inline bool getDeleteStream() const;

	///
	/// return allow to write over the size boundary or not
	inline bool getResizable() const;

	///
	/// Return starts current position from which all actions will be made.

	inline gint64 getStart() const;

private:

	JMappedOutputStream(const JMappedOutputStream& rhs);            // cannot be copied
	JMappedOutputStream& operator=(const JMappedOutputStream& rhs); // nor assigned

private:

	JOutputStream *stream_;

	gint64 start_;
	gint64 size_;
	bool bDeleteStream_;
	bool bResizable_;
	bool bStatus_;
};

#endif //__JMappedOutputStream_h__