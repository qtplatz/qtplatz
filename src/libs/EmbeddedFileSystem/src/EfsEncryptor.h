//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __EfsEncryptor_h__
#define __EfsEncryptor_h__

///
/// Class EfsEncryptor is used for encript/decrypt afs content
///

class EfsEncryptor
{
public:

	///
	/// Encrypt data block
	virtual bool encrypt( char *data, int dataSize );

	///
	/// decrypt data block
	virtual bool decrypt( char *data, int dataSize );
	
};

// Default encryptor
extern EfsEncryptor EfsDefaultEncryptor;

#endif // __EfsEncryptor_h__

