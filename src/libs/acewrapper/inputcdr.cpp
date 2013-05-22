// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include "inputcdr.hpp"

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
  ACE_CDR::LongLong x;
  impl_.read_longlong( x );
  t = x;
  return *this;
}

InputCDR& InputCDR::operator >> ( unsigned long long& t )
{
    ACE_CDR::ULongLong x;
  impl_.read_ulonglong( x );
  t = x;
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
	return impl_.read_longlong_array( reinterpret_cast<ACE_CDR::LongLong *>(p), o );
}

bool
InputCDR::read( unsigned long long * p, size_t o )
{
	return impl_.read_ulonglong_array( reinterpret_cast<ACE_CDR::ULongLong *>(p), o );
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

