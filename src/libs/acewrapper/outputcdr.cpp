//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "outputcdr.h"

using namespace acewrapper;

OutputCDR::~OutputCDR()
{
    delete pImpl_;
}

OutputCDR::OutputCDR( ACE_OutputCDR& cdr ) : pImpl_(0), impl_(cdr)
{
}

OutputCDR& OutputCDR::operator << ( char t )
{
  impl_.write_char( t );
  return *this;
}

OutputCDR& OutputCDR::operator << ( unsigned char t )
{
  impl_.write_char( t );
  return *this;
}

OutputCDR& OutputCDR::operator << ( short t )
{
  impl_.write_short( t );
  return *this;
}

OutputCDR& OutputCDR::operator << ( unsigned short t )
{
  impl_.write_ushort( t );
  return *this;
}

OutputCDR& OutputCDR::operator << ( long t )
{
  impl_.write_long( t );
  return *this;
}

OutputCDR& OutputCDR::operator << ( unsigned long t )
{
  impl_.write_ulong( t );
  return *this;
}

OutputCDR& OutputCDR::operator << ( long long t )
{
  impl_.write_longlong( t );
  return *this;
}

OutputCDR& OutputCDR::operator << ( unsigned long long t )
{
  impl_.write_ulonglong( t );
  return *this;
}

OutputCDR& OutputCDR::operator << ( float t )
{
  impl_.write_float( t );  
  return *this;
}

OutputCDR& OutputCDR::operator << ( double t )
{
  impl_.write_double( t );  
  return *this;
}

OutputCDR& OutputCDR::operator << ( const std::string& t )
{
  impl_.write_string( t.size(), t.c_str() );
  return *this;
}

OutputCDR& OutputCDR::operator << ( const std::wstring& t )
{
  impl_.write_wstring( t.size(), t.c_str() );
  return *this;
}

bool
OutputCDR::write( const bool * p, size_t o )
{
	return impl_.write_boolean_array( p, o );
}

bool
OutputCDR::write( const char * p, size_t o )
{
	return impl_.write_char_array( p, o );
}

bool
OutputCDR::write( const unsigned char * p, size_t o )
{
	return impl_.write_octet_array( p, o );
}

bool
OutputCDR::write( const short * p, size_t o )
{
	return impl_.write_short_array( p, o );
}

bool
OutputCDR::write( const unsigned short * p, size_t o )
{
	return impl_.write_ushort_array( p, o );
}

bool
OutputCDR::write( const long * p, size_t o )
{
	return impl_.write_long_array( reinterpret_cast<const ACE_CDR::Long *>(p), o );
}

bool
OutputCDR::write( const unsigned long * p, size_t o )
{
	return impl_.write_ulong_array( reinterpret_cast<const ACE_CDR::ULong *>(p), o );
}

bool
OutputCDR::write( const long long * p, size_t o )
{
	return impl_.write_longlong_array( p, o );
}

bool
OutputCDR::write( const unsigned long long * p, size_t o )
{
	return impl_.write_ulonglong_array( p, o );
}

bool
OutputCDR::write( const float * p, size_t o )
{
	return impl_.write_float_array( p, o );
}

bool
OutputCDR::write( const double * p, size_t o )
{
	return impl_.write_double_array( p, o );
}

bool
OutputCDR::write( const long double * p, size_t o )
{
	return impl_.write_longdouble_array( reinterpret_cast<const ACE_CDR::LongDouble *>(p), o );
}

