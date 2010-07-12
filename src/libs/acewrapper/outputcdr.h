// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <ace/CDR_Stream.h>
#include <string>
#include <boost/noncopyable.hpp>

namespace acewrapper {

	class OutputCDR : boost::noncopyable {
	public:
		OutputCDR();
		OutputCDR( ACE_Message_Block * mb );

		operator ACE_OutputCDR& () { return impl_; }
		operator const ACE_OutputCDR& () const { return impl_; }
		const ACE_Message_Block * begin() { return impl_.begin(); }

		OutputCDR& operator << (bool);
		OutputCDR& operator << (char);
		OutputCDR& operator << (unsigned char);
		OutputCDR& operator << (short);
		OutputCDR& operator << (unsigned short);
		OutputCDR& operator << (long);
		OutputCDR& operator << (unsigned long);
		OutputCDR& operator << (long long);
		OutputCDR& operator << (unsigned long long);
		OutputCDR& operator << (float);
		OutputCDR& operator << (double);
		OutputCDR& operator << (const std::string& );
		OutputCDR& operator << (const std::wstring& );
	private:
		ACE_OutputCDR impl_;
	};

}

