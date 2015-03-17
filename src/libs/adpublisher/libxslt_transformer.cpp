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

#include "libxslt_transformer.hpp"
#include <xmlparser/pugixml.hpp>
#include <libxslt/transform.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltutils.h>
#include <libxslt/xsltInternals.h>
#include <boost/filesystem/path.hpp>
#include <QCoreApplication>
#include <QMessageBox>
#include <QString>
#include <fstream>

using namespace adpublisher;
using namespace adpublisher::libxslt;

transformer::~transformer()
{
}

transformer::transformer()
{
}

//static
bool
transformer::apply_template( const char * xslfile, const char * xmlfile, const char * outfile )
{
    const char *params[16 + 1];
    memset( params, 0, sizeof( params ) );

    if ( auto cur = xsltParseStylesheetFile( reinterpret_cast< const xmlChar *>(xslfile ) ) ) {

        if ( auto doc = xmlParseFile( xmlfile ) ) {

            if ( auto res = xsltApplyStylesheet( cur, doc, params ) ) {

                if ( FILE *ofile = fopen( outfile, "w" ) ) {
                    xsltSaveResultToFile( ofile, res, cur );
                    fclose( ofile );
                }

                xsltFreeStylesheet( cur );
                xmlFreeDoc( res );
                xmlFreeDoc( doc );
                xmlCleanupParser();

                return true;
            }

        }
    }
    return false;
}

bool
transformer::apply_template( const char * xmlfile, const char * xslfile, QString& output )
{
    const char *params[16 + 1];
    memset( params, 0, sizeof( params ) );

    if ( auto cur = xsltParseStylesheetFile( reinterpret_cast< const xmlChar *>(xslfile ) ) ) {

        if ( auto doc = xmlParseFile( xmlfile ) ) {

            if ( auto res = xsltApplyStylesheet( cur, doc, params ) ) {

                xmlChar * doc_txt_ptr = 0;
                int doc_txt_len = 0;
                if ( res->children == 0 )
                    return false;

                if ( xsltSaveResultToString( &doc_txt_ptr, &doc_txt_len, res, cur ) >= 0 ) {
                    output = QString( reinterpret_cast< char *>( doc_txt_ptr ) );
                    free( doc_txt_ptr );
                }

                xsltFreeStylesheet( cur );
                xmlFreeDoc( res );
                xmlFreeDoc( doc );
                xmlCleanupParser();

                return true;
            }

        }
    }
    return false;
}

//static
void
transformer::xsltpath( boost::filesystem::path& path, const char * xsltfile )
{
    static auto dir = boost::filesystem::path( QCoreApplication::applicationDirPath().toStdWString() );
    dir.remove_filename(); // updir

#if defined Q_OS_MAC
    dir /= "Resources/xslt";
#else
    dir /= "share/qtplatz/Resources/xslt";
#endif

    path = ( dir / boost::filesystem::path( xsltfile ) ).generic_wstring();
}

