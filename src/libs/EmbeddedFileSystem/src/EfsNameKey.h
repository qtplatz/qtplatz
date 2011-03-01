//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __EfsNameKey_h__
#define __EfsNameKey_h__

///
/// Class EfsNameKey
///

class EfsNameKey
{
public:

	EfsNameKey();
	EfsNameKey( const char* name );

	// Methods
	void setName( const char* name );

	inline static long make_hash( const char *d );

	bool operator==( const EfsNameKey &rhv ) const;
	bool operator<( const EfsNameKey &rhv ) const;
	bool operator>( const EfsNameKey &rhv ) const;

	// Data
	int hash_;
	char name_[ EfsDefaultMaxFilenameSize ];
};

#endif // __EfsNameKey_h__

