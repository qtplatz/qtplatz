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
#include <adportable/debug.hpp>
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
#include <sstream>

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
transformer::apply_template( const boost::filesystem::path& xslfile
                             , const boost::filesystem::path& xmlfile, const boost::filesystem::path& outfile )
{
    const char *params[16 + 1];
    memset( params, 0, sizeof( params ) );

    if ( auto cur = xsltParseStylesheetFile( reinterpret_cast< const xmlChar *>( xslfile.string().c_str() ) ) ) {

        if ( auto doc = xmlParseFile( xmlfile.string().c_str() ) ) {
            
            if ( auto res = xsltApplyStylesheet( cur, doc, params ) ) {

                if ( FILE *ofile = fopen( outfile.string().c_str(), "w" ) ) {
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
transformer::apply_template( const boost::filesystem::path& xmlfile
                             , const boost::filesystem::path& xslfile, QString& output )
{
    const char *params[16 + 1];
    memset( params, 0, sizeof( params ) );

    if ( auto cur = xsltParseStylesheetFile( reinterpret_cast< const xmlChar *>( xslfile.string().c_str() ) ) ) {

        if ( auto doc = xmlParseFile( xmlfile.string().c_str() ) ) {

            if ( auto res = xsltApplyStylesheet( cur, doc, params ) ) {

                xmlChar * doc_txt_ptr = 0;
                int doc_txt_len = 0;
                if ( res->children == 0 )
                    return false;

                if ( xsltSaveResultToString( &doc_txt_ptr, &doc_txt_len, res, cur ) >= 0 ) {
                    output = QString( reinterpret_cast< char * >( doc_txt_ptr ) );
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

// in-memory transform
//static
bool
transformer::apply_template( const boost::filesystem::path& xsltfile, const pugi::xml_document& dom, QString& output )
{
    const char *params[16 + 1];
    memset( params, 0, sizeof( params ) );

    if ( auto cur = xsltParseStylesheetFile( reinterpret_cast< const xmlChar *>( xsltfile.string().c_str() ) ) ) {

        std::ostringstream xml;
        dom.save( xml );

        if ( auto doc = xmlParseMemory( xml.str().c_str(), xml.str().size() ) ) {

            if ( auto res = xsltApplyStylesheet( cur, doc, params ) ) {

                xmlChar * doc_txt_ptr = 0;
                int doc_txt_len = 0;
                if ( res->children == 0 )
                    return false;

                if ( xsltSaveResultToString( &doc_txt_ptr, &doc_txt_len, res, cur ) >= 0 ) {
                    output = QString( reinterpret_cast< char * >( doc_txt_ptr ) );
                    free( doc_txt_ptr );
                }

                xsltFreeStylesheet( cur );
                xmlFreeDoc( res );
                xmlFreeDoc( doc );
                xmlCleanupParser();

                return true;
            } else
                ADDEBUG() << "Error: xsltApplyStylesheet";
        } else
            ADDEBUG() << "Error: xsltParseStylesheetMemory: " << xml.str();
    } else
        ADDEBUG() << "Error: xsltParseStylesheetFile(" << xsltfile.string() << ")";
    return false;
}

//static
void
transformer::xsltpath( boost::filesystem::path& path, const char * xsltfile )
{
    static const auto dir = boost::filesystem::path( QCoreApplication::applicationDirPath().toStdWString() ).remove_filename();
#if defined Q_OS_MAC
    static constexpr auto xsltdir = "Resources/xslt";
#else
    static constexpr auto xsltdir = "share/qtplatz/xslt";
#endif    

    path = dir / xsltdir / boost::filesystem::path( xsltfile );
}

