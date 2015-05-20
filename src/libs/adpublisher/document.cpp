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

#include "document.hpp"

#if defined Q_OS_WIN32
#  include "msxml_transformer.hpp"
#else 
# if defined Q_OS_MAC || defined Q_OS_LINUX
#  include "libxslt_transformer.hpp"
# endif
#endif
#include <adportable/profile.hpp>
#include <xmlparser/pugixml.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/filesystem.hpp>
#include <QApplication>
#include <QFileInfo>

//#include <QXmlQuery>

using namespace adpublisher;

document::document() : doc_( std::make_shared< pugi::xml_document >() )
{
    if ( auto comment = doc_->append_child( pugi::node_comment ) ) {
        comment.set_value( "Copyright(C) 2010-2015, MS-Cheminformatics LLC, All rights reserved." );
    }
    if ( auto article = doc_->append_child( "article" ) ) {

        if ( auto title = article.append_child( "title" ) ) {
            title.append_attribute( "lang" ) = "en-us";
            title.text().set( "Quantitative analysis report." );
        }

        if ( auto author = article.append_child( "author" ) ) {
            author.append_attribute( "lang" ) = "en-us";
            author.text().set( adportable::profile::user_login_name<char>().c_str() );
        }

        if ( auto sec = article.append_child( "section" ) ) {
            sec.append_attribute( "id" ) = "sec_1";
            sec.append_attribute( "lang" ) = "en-us";

            if ( auto title = sec.append_child( "title" ) ) {
                title.append_attribute( "lang" ) = "en-us";
                title.text().set( "Introduction" );
            }
            
            if ( auto para = sec.append_child( "paragraph" ) ) {
                para.append_attribute( "lang" ) = "en-us";
                para.text().set( "Introduction to be added..." );
            }
        }

        if ( auto sec = article.append_child( "section" ) ) {
            sec.append_attribute( "id" ) = "sec_2";
            sec.append_attribute( "lang" ) = "en-us";

            if ( auto title = sec.append_child( "title" ) ) {
                title.text().set( "Methods" );
                title.append_attribute( "lang" ) = "en-us";
            }
            if ( auto para = sec.append_child( "paragraph" ) ) {
                para.append_attribute( "lang" ) = "en-us";
                para.text().set( "Column: 0.32mmID 10mL, 3&mu;m\nCarrier: He, Flow rage: 5mL/min\nDetector: InfiTOF, ..." );
            }
        }

        if ( auto sec = article.append_child( "section" ) ) {
            sec.append_attribute( "id" ) = "sect_3";
            sec.append_attribute( "lang" ) = "en-us";
            if ( auto title = sec.append_child( "title" ) ) {
                title.text().set( "Results" );
                title.append_attribute( "lang" ) = "en-us";
            }
            
            if ( auto sec = article.append_child( "results" ) ) {
            }
        }
    }
}

bool
document::save_file( const char * filepath ) const
{
    return doc_->save_file( filepath );
}

bool
document::load_file( const char * filepath )
{
    return doc_->load_file( filepath );
}

bool
document::save( std::ostream& o ) const
{
    doc_->save( o );
    return true;
}

bool
document::save( std::string& ar ) const
{
    boost::iostreams::back_insert_device< std::string > inserter( ar );
    boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > device( inserter );
    doc_->save( device );
    return true;
}

bool
document::load( const char * xml )
{
    return doc_->load( xml );
}

std::shared_ptr< pugi::xml_document >
document::xml_document()
{
    return doc_;
}

//static
bool
document::apply_template( const char * xmlfile, const char * xsltfile, QString& output, QString& method )
{
#if defined Q_OS_WIN32
    using namespace msxml;
#else
    using namespace libxslt;
#endif

    if ( !boost::filesystem::exists( xmlfile ) )
        return false;

    boost::filesystem::path xslt( xsltfile );
    if ( !xslt.is_absolute() )
        transformer::xsltpath( xslt, xsltfile );

    if ( !boost::filesystem::exists( xslt ) )
        return false;

    pugi::xml_document doc;
    if ( doc.load_file( xsltfile ) ) {
        if ( auto node = doc.select_single_node( "//xsl:output[@method]" ) ) {
            method = node.node().attribute( "method" ).value();
            if ( method == "xml" ) {
                std::string media = node.node().attribute( "media-type" ).value();
                if ( media == "text/xhtml" )
                    method = "xhtml";
            }
        }
    }
    return transformer::apply_template( xmlfile, xslt.string().c_str(), output );
}

