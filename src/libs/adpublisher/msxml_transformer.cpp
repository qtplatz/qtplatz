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

#include "msxml_transformer.hpp"
#include <pugixml.hpp>
#include <atlbase.h>
#include <atlcom.h>
#include <comdef.h>
#pragma warning(push)
#pragma warning(disable:4192)
#import <msxml6.dll> named_guids
#pragma warning(pop)
#include <fstream>
#include <QCoreApplication>
#include <QMessageBox>
#include <QString>
#include <boost/filesystem/path.hpp>
#include <sstream>


namespace adpublisher {
    namespace msxml {

        class singleton {
            static singleton * instance_;
        public:
            static singleton * instance() {
                if ( instance_ == 0 )
                    instance_ = new singleton;
                return instance_;
            }
            static void dispose() {
                delete instance_;
            }
        private:
            singleton() {
                ::CoInitialize( 0 );
                atexit( dispose );
            }
            ~singleton() {
                ::CoUninitialize();
            }
        };

        singleton * singleton::instance_( 0 );

    }
}

using namespace adpublisher;
using namespace adpublisher::msxml;

bool
transformer::apply_template( const std::filesystem::path& xslfile
                             , const std::filesystem::path& xmlfile, const std::filesystem::path& outfile )
{
    auto p = singleton::instance();
    (void)p;

    MSXML2::IXMLDOMDocument3Ptr xml, xsl;

    if ( CoCreateInstance( CLSID_DOMFreeThreadedDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&xml ) != S_OK )
        return false;

    if ( CoCreateInstance( CLSID_DOMFreeThreadedDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&xsl ) != S_OK )
        return false;

    if ( xml->load( _bstr_t( xmlfile.string().c_str() ) ) == VARIANT_TRUE ) {
        if ( xsl->load( _bstr_t( xslfile.string().c_str() ) ) == VARIANT_TRUE ) {

            _bstr_t out = xml->transformNode( xsl );

            MSXML2::IXMLDOMDocument3Ptr oxml;
            if ( CoCreateInstance( CLSID_DOMFreeThreadedDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&oxml ) != S_OK )
                return false;

            oxml->loadXML( out );
            oxml->save( _bstr_t( outfile.string().c_str() ) );

            return true;

        }
    }
    return false;
}

bool
transformer::apply_template( const std::filesystem::path& xmlfile, const std::filesystem::path& xslfile, QString& output )
{
    auto p = singleton::instance();
    (void)p;

    MSXML2::IXMLDOMDocument3Ptr xml, xsl;

    auto hr = CoCreateInstance( CLSID_DOMFreeThreadedDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&xml );
    if ( hr != S_OK )
        return false;

    hr = CoCreateInstance( CLSID_DOMFreeThreadedDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&xsl );
    if ( hr != S_OK )
        return false;

    if ( xml->load( _bstr_t( xmlfile.string().c_str() ) ) == VARIANT_TRUE ) {
        if ( xsl->load( _bstr_t( xslfile.string().c_str() ) ) == VARIANT_TRUE ) {

            try {
                _bstr_t out = xml->transformNode( xsl );
                output = QString::fromUtf16( reinterpret_cast<const ushort *>(static_cast<const wchar_t *>(out)) );
            }
            catch ( _com_error& ex ) {
                QMessageBox::warning( 0, "apply_template", QString::fromWCharArray( ex.ErrorMessage() ) );
            }

            return true;

        }
    }
    return false;
}

// in-memory transform
//static
bool
transformer::apply_template( const std::filesystem::path& xsltfile, const pugi::xml_document& dom, QString& output )
{
    auto p = singleton::instance();
    (void)p;

    MSXML2::IXMLDOMDocument3Ptr xml, xsl;

    auto hr = CoCreateInstance( CLSID_DOMFreeThreadedDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&xml );
    if ( hr != S_OK )
        return false;

    hr = CoCreateInstance( CLSID_DOMFreeThreadedDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&xsl );
    if ( hr != S_OK )
        return false;

    std::ostringstream xmlstr;
    dom.save( xmlstr );

    if ( xml->loadXML( _bstr_t( xmlstr.str().c_str() ) ) == VARIANT_TRUE ) {
        if ( xsl->load( _bstr_t( xsltfile.string().c_str() ) ) == VARIANT_TRUE ) {

            try {
                _bstr_t out = xml->transformNode( xsl );
                output = QString::fromUtf16( reinterpret_cast<const ushort *>(static_cast<const wchar_t *>(out)) );
            }
            catch ( _com_error& ex ) {
                QMessageBox::warning( 0, "apply_template", QString::fromWCharArray( ex.ErrorMessage() ) );
            }

            return true;

        }
    }
    return false;
}

//static
void
transformer::xsltpath( std::filesystem::path& path, const char * xsltfile )
{
    static const std::filesystem::path dir =
        std::filesystem::path( QCoreApplication::applicationDirPath().toStdWString() )
        /= std::filesystem::path( "/../share/qtplatz/xslt" );

    path = (dir / xsltfile).generic_wstring();
}
