//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "outputcdr.h"

using namespace acewrapper;

OutputCDR::OutputCDR()
{
}

OutputCDR::OutputCDR( ACE_Message_Block * mp ) : impl_(mp)
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

