// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <string>
// #include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>
#include <adinterface/eventlogC.h>

namespace adinterface {

    namespace EventLog {

        // typedef const boost::variant< void *, unsigned long, signed long, std::wstring > var_type;

        class LogMessageHelper {
        public:
            static std::wstring toString( const ::EventLog::LogMessage& );
            
            LogMessageHelper( const std::wstring& format = L""
                              , ::EventLog::eMSGPRIORITY pri = ::EventLog::pri_DEBUG
                              , const std::wstring& msgId = L""
                              , const std::wstring& srcId = L"");
            LogMessageHelper( const LogMessageHelper& );
            LogMessageHelper& format( const std::wstring& );
            template<class T> LogMessageHelper& operator % (const T& t) {
                msg_.args.length( msg_.args.length() + 1 );
                msg_.args[ msg_.args.length() - 1 ] = CORBA::wstring_dup( boost::lexical_cast< std::wstring >( t ).c_str() );
                return *this;
            }

            inline ::EventLog::LogMessage & get() { return msg_; }
        private:
            ::EventLog::LogMessage msg_;
        };
    }
}
