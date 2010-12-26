// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

// datafile should be corresponding to single sample

#pragma once

#include "adcontrols_global.h"
#include <string>
#include <boost/any.hpp>

namespace adcontrols {
    
    class dataSubscriber;

    class ADCONTROLSSHARED_EXPORT datafile { // visitable
    public:
        datafile(void) {}
        virtual ~datafile(void) {}

        typedef datafile * (*factory_type)(void);
        virtual factory_type factory() = 0;
        //------
        const std::wstring& filename() const;
        bool readonly() const;
        // ----- virtual methods -----
        virtual void accept( dataSubscriber& ) = 0; // visitable
        virtual boost::any fetch( const std::wstring& path, const std::wstring& dataType ) = 0;
        //---------

        static bool access( const std::wstring& filename );
        static datafile * open( const std::wstring& filename, bool readonly = false );
        static void close( datafile *& );

    private:
        std::wstring filename_;
        bool readonly_;
    };

}

