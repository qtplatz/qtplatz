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

#ifndef MSXML_TRANSFORMER_HPP
#define MSXML_TRANSFORMER_HPP

#include <filesystem>
class QString;

namespace pugi { class xml_document; }

namespace adpublisher {

    namespace msxml {

        class transformer {
        public:
            transformer();
            ~transformer();

            static void xsltpath( std::filesystem::path& path, const char * xsltfile );
            static bool apply_template( const std::filesystem::path& xsltfile, const std::filesystem::path&, const std::filesystem::path& outfile );
            static bool apply_template( const std::filesystem::path& xsltfile, const std::filesystem::path&, QString& );

            // in-memory transform
            static bool apply_template( const std::filesystem::path& xsltfile, const pugi::xml_document&, QString& );
        };

    }

}

#endif // MSXML_TRANSFORMER_HPP
