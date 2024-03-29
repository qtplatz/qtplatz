/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <memory>
#include <ostream>
#include "adpublisher_global.hpp"

namespace pugi { class xml_document; }

namespace adpublisher {

    class ADPUBLISHERSHARED_EXPORT document {
    public:
        document();
        document( const document& ) = delete;

        bool save_file( const char * filepath ) const;
        bool load_file( const char * filepath );
        bool save( std::ostream& ) const;
        bool save( std::string& ) const;
        bool load( const char * );

        std::shared_ptr< pugi::xml_document > xml_document();

        static bool apply_template( const char * xmlfile, const char * xsltfile, QString& output, QString& method );

    private:
        std::shared_ptr< pugi::xml_document > doc_;
    };

}


