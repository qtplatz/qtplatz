/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#pragma once

#include "adcontrols_global.h"
#include <cstdint>
#include <memory>

namespace boost { 
    namespace uuids { struct uuid; }
    namespace serialization { class access; }
}

namespace adcontrols {

    class idAudit;

    class ADCONTROLSSHARED_EXPORT SampleRun {
    public:
        ~SampleRun();
        SampleRun();
        SampleRun( const SampleRun& );

        static const wchar_t * dataClass() { return L"adcontrols::SampleRun"; }

        const boost::uuids::uuid& uuid() const;
        const idAudit& ident() const;

        double methodTime() const;
        void methodTime( double ) const;

        size_t replicates() const;
        void replicates( size_t );

        const wchar_t * dataDirectory() const;
        void dataDirectory( const wchar_t * );

        const wchar_t * filePrefix() const; // RUN_0001
        void filePrefix( const wchar_t * file );

        const char * description() const;
        void description( const char * );

        size_t runno() const;
        size_t next_run();

        static bool archive( std::ostream&, const SampleRun& );
        static bool restore( std::istream&, SampleRun& );
        static bool xml_archive( std::wostream&, const SampleRun& );
        static bool xml_restore( std::wistream&, SampleRun& );

    private:
#   if  defined _MSC_VER
#   pragma warning(disable:4251)
#   endif
        class impl;
        std::unique_ptr< impl > impl_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
    };

    typedef std::shared_ptr<SampleRun> SampleRunPtr;   

}




