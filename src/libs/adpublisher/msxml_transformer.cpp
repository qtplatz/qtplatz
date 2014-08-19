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
#include <xmlparser/pugixml.hpp>
#include <atlbase.h>
#include <atlcom.h>
#include <comdef.h>
#import <msxml6.dll> named_guids
#include <fstream>
#include <QString>
#include <boost/filesystem/path.hpp>

namespace adpublisher {
    namespace msxml {
        transformer * transformer::instance_( 0 );
    }
}

using namespace adpublisher;
using namespace adpublisher::msxml;

transformer::~transformer()
{
    ::CoUninitialize();
}

transformer::transformer()
{
    ::CoInitialize( 0 );
}

transformer *
transformer::instance()
{
    if ( instance_ == 0 )
        instance_ = new transformer;
    return instance_;
}

bool
transformer::apply_template( const char * xslfile, const char * xmlfile, const char * outfile )
{
    MSXML2::IXMLDOMDocument3Ptr xml, xsl;
    
    if ( CoCreateInstance( CLSID_DOMFreeThreadedDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&xml ) != S_OK )
        return false;
    
    if ( CoCreateInstance( CLSID_DOMFreeThreadedDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&xsl ) != S_OK )
        return false;

    if ( xml->load( _bstr_t( xmlfile ) ) == VARIANT_TRUE ) {
        if ( xsl->load( _bstr_t( xslfile ) ) == VARIANT_TRUE ) {
            _bstr_t out = xml->transformNode( xsl );

            std::ofstream of( outfile );
            of << out;
            
            return true;

        }
    }
    return false;
}

bool
transformer::apply_template( const char * xmlfile, const char * xslfile, QString& output )
{
    MSXML2::IXMLDOMDocument3Ptr xml, xsl;
    
    auto hr = CoCreateInstance( CLSID_DOMFreeThreadedDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&xml );
    if ( hr != S_OK )
        return false;
    
    hr = CoCreateInstance( CLSID_DOMFreeThreadedDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&xsl );
    if ( hr != S_OK )
        return false;

    if ( xml->load( _bstr_t( xmlfile ) ) == VARIANT_TRUE ) {
        if ( xsl->load( _bstr_t( xslfile ) ) == VARIANT_TRUE ) {

            xml->save( L"C:/Users/Toshi/Documents/data/QUAN/xmlfile.xml" );

            _bstr_t out = xml->transformNode( xsl );

            output = QString::fromUtf16( reinterpret_cast<const ushort *>(static_cast<const wchar_t *>(out)) );

            return true;

        }
    }
    return false;
}

