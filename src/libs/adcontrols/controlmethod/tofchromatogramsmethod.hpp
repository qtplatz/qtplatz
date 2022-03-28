// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "../adcontrols_global.h"
#include <boost/json/fwd.hpp>
#include <boost/json/value_to.hpp>
#include <string>
#include <map>
#include <vector>

namespace boost {
    namespace serialization { class access; }
    namespace uuids { struct uuid; }
}

namespace adcontrols {

    namespace xic {
        enum eIntensityAlgorishm : unsigned int;
    }

    class TofChromatogramMethod;

    class ADCONTROLSSHARED_EXPORT TofChromatogramsMethod {
    public:
        ~TofChromatogramsMethod();
        TofChromatogramsMethod();
        TofChromatogramsMethod( const TofChromatogramsMethod& );

        static const char * modelClass() { return "TofChromatograms"; }
        static const char * itemLabel() { return "Chromatograms.1"; }
        static const boost::uuids::uuid& clsid();

        typedef std::vector< TofChromatogramMethod >::iterator iterator;
        typedef std::vector< TofChromatogramMethod >::const_iterator const_iterator;

        size_t size() const;
        void clear();

        TofChromatogramsMethod& operator << ( const TofChromatogramMethod& );

        iterator begin();
        iterator end();

        const_iterator begin() const;
        const_iterator end() const;

        size_t numberOfTriggers() const;
        void setNumberOfTriggers( size_t );

        bool refreshHistogram() const;
        void setRefreshHistogram( bool );
        std::tuple< bool, xic::eIntensityAlgorishm > tic() const;
        void setTIC( std::tuple< bool, xic::eIntensityAlgorishm >&& );

        static bool archive( std::ostream&, const TofChromatogramsMethod& );
        static bool restore( std::istream&, TofChromatogramsMethod& );
        static bool xml_archive( std::wostream&, const TofChromatogramsMethod& );
        static bool xml_restore( std::wistream&, TofChromatogramsMethod& );

    private:
        class impl;
        impl * impl_;
        friend ADCONTROLSSHARED_EXPORT void tag_invoke( boost::json::value_from_tag, boost::json::value&, const TofChromatogramsMethod& );
        friend ADCONTROLSSHARED_EXPORT TofChromatogramsMethod
        tag_invoke( boost::json::value_to_tag< TofChromatogramsMethod >&, const boost::json::value& jv );
    };

    ADCONTROLSSHARED_EXPORT
    void tag_invoke( boost::json::value_from_tag, boost::json::value&, const TofChromatogramsMethod& );

    ADCONTROLSSHARED_EXPORT
    TofChromatogramsMethod tag_invoke( boost::json::value_to_tag< TofChromatogramsMethod >&, const boost::json::value& jv );

}
