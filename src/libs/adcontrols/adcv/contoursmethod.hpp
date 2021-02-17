/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "../adcontrols_global.h"
#include <boost/system/error_code.hpp>
#include <boost/optional.hpp>
#include <utility>
#include <string>

namespace adcontrols {
    namespace adcv {

        class ADCONTROLSSHARED_EXPORT ContoursMethod {
        public:
        enum BlurAlgo { NoBlur, Blur, GaussianBlur };

        explicit ContoursMethod();
        ContoursMethod( const ContoursMethod& );
        ~ContoursMethod();

        void setSizeFactor( int );
        void setBlurSize( int );
        void setCannyThreshold( std::pair< int, int >&& );
        void setMinSizeThreshold( unsigned );
        void setMaxSizeThreshold( unsigned );
        void setKernelSize( unsigned );

        int sizeFactor() const;
        int blurSize() const;
        std::pair< int, int > cannyThreshold() const;
        unsigned minSizeThreshold() const;
        unsigned maxSizeThreshold() const;
        unsigned kernelSize() const;

        void setBlur( BlurAlgo );
        BlurAlgo blur() const;

        std::string to_json() const;
        static std::string to_json( const ContoursMethod& );

        static boost::optional< ContoursMethod > from_json( const std::string&, boost::system::error_code& );

        private:
        int resize_;
        int blurSize_;
        std::pair< int, int > cannyThreshold_;
        std::pair< unsigned, unsigned > szThreshold_;
        unsigned kernelSize_;
        BlurAlgo blur_;
        };
    }
}
