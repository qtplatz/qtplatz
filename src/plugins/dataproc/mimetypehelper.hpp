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

#ifndef MIMEHELPER_HPP
#define MIMEHELPER_HPP

#include <QtCore>
#include <coreplugin/mimedatabase.h>
#include <xmlparser/pugixml.hpp>

namespace dataproc {

    class mimeTypeHelper {
    public:
        static bool add( const char * xml, int len, QString*& emsg ) {
            if ( xml ) {
                QBuffer io;
                io.setData( xml, len );
                io.open( QIODevice::ReadOnly );
                return  Core::MimeDatabase::addMimeTypes( &io, emsg );
            }
            return false;
        }
        static bool populate( QStringList& vec, const char * xml ) {
            pugi::xml_document doc;
            if ( doc.load( xml ) ) {
                pugi::xpath_node_set list = doc.select_nodes( "/mime-info/mime-type" );
                for ( auto it = list.begin(); it != list.end(); ++it )
                    vec << it->node().attribute( "type" ).value();
				return true;
			}
			return false;
        }
    };
}


#endif // MIMEHELPER_HPP
