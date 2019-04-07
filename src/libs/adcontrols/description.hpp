// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#ifndef DESCRIPTION_H
#define DESCRIPTION_H

#include "adcontrols_global.h"
#include <string>
#include <time.h>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>

namespace adcontrols {

    template< typename T > struct description_archive;

    class ADCONTROLSSHARED_EXPORT description;
    class  description {
    public:
        ~description();
        description();
        description( const description& );
        [[deprecated]] description( const wchar_t * key, const wchar_t * text );
        [[deprecated]] description( const std::wstring& key, const std::wstring& text );
        description( std::pair< std::string, std::string >&& keyValue );

        inline bool operator == ( const description& t ) const;

        [[deprecated]] std::wstring text() const;
        [[deprecated]] std::wstring key() const;
        std::pair< std::string, std::string > keyValue() const;
        void setKey( const std::string& );
        void setValue( const std::string& );

        const char * xml() const;
        void xml( const char * u );

    private:
        uint64_t posix_time_;
        std::pair< std::string, std::string > keyValue_;
        std::string xml_;

        friend struct description_archive< description >;
        friend struct description_archive< const description >;

        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive& ar, const unsigned int version );
    };

}

BOOST_CLASS_VERSION(adcontrols::description, 3);

#endif // DESCRIPTION_H
