//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "inputcdr.h"

using namespace acewrapper;

//InputCDR::InputCDR( ACE_Message_Block * mb ) : impl_( mb )
InputCDR::InputCDR( ACE_InputCDR& cdr ) : impl_(cdr)
{
}

InputCDR& InputCDR::operator >> ( bool& t )
{
  impl_.read_boolean( t );
  return *this;
}

InputCDR& InputCDR::operator >> ( char& t )
{
  impl_.read_char( t );
  return *this;
}

InputCDR& InputCDR::operator >> ( unsigned char& t )
{
	ACE_CDR::Char x;
	impl_.read_char( x );
	t = x;
  return *this;
}

InputCDR& InputCDR::operator >> ( short& t )
{
  impl_.read_short( t );
  return *this;
}

InputCDR& InputCDR::operator >> ( unsigned short& t )
{
  impl_.read_ushort( t );
  return *this;
}

InputCDR& InputCDR::operator >> ( long& t )
{
	ACE_CDR::Long x;
	impl_.read_long( x );
	t = x;
	return *this;
}

InputCDR& InputCDR::operator >> ( unsigned long& t )
{
	ACE_CDR::ULong x;
	impl_.read_ulong( x );
	t = x;
	return *this;
}

InputCDR& InputCDR::operator >> ( long long& t )
{
  impl_.read_longlong( t );
  return *this;
}

InputCDR& InputCDR::operator >> ( unsigned long long& t )
{
  impl_.read_ulonglong( t );
  return *this;
}

InputCDR& InputCDR::operator >> ( float& t )
{
  impl_.read_float( t );  
  return *this;
}

InputCDR& InputCDR::operator >> ( double& t )
{
  impl_.read_double( t );  
  return *this;
}

InputCDR& InputCDR::operator >> ( std::string& t )
{
	ACE_CDR::Char * p;
	impl_.read_string( p );
	t = std::string(p);
	return *this;
}

InputCDR& InputCDR::operator >> ( std::wstring& t )
{
	ACE_CDR::WChar * p;
	impl_.read_wstring( p );
	t = std::wstring( p );
	return *this;
}

bool
InputCDR::read( bool * p, size_t o )
{
	return impl_.read_boolean_array( p, o );
}

bool
InputCDR::read( char * p, size_t o )
{
	return impl_.read_char_array( p, o );
}

bool
InputCDR::read( unsigned char * p, size_t o )
{
	return impl_.read_octet_array( p, o );
}

bool
InputCDR::read( short * p, size_t o )
{
	return impl_.read_short_array( p, o );
}

bool
InputCDR::read( unsigned short * p, size_t o )
{
	return impl_.read_ushort_array( p, o );
}

bool
InputCDR::read( long * p, size_t o )
{
	return impl_.read_long_array( reinterpret_cast<ACE_CDR::Long *>(p), o );
}

bool
InputCDR::read( unsigned long * p, size_t o )
{
	return impl_.read_ulong_array( reinterpret_cast<ACE_CDR::ULong *>(p), o );
}

bool
InputCDR::read( long long * p, size_t o )
{
	return impl_.read_longlong_array( p, o );
}

bool
InputCDR::read( unsigned long long * p, size_t o )
{
	return impl_.read_ulonglong_array( p, o );
}

bool
InputCDR::read( float * p, size_t o )
{
	return impl_.read_float_array( p, o );
}

bool
InputCDR::read( double * p, size_t o )
{
	return impl_.read_double_array( p, o );
}

bool
InputCDR::read( long double * p, size_t o )
{
	return impl_.read_longdouble_array( reinterpret_cast<ACE_CDR::LongDouble *>(p), o );
}

