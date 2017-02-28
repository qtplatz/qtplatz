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

#include "printer.hpp"
#if defined Q_OS_WIN32
#  include "msxml_transformer.hpp"
#else 
# if defined Q_OS_MAC || defined Q_OS_LINUX
#  include "libxslt_transformer.hpp"
# endif
#endif

#include <adportable/debug.hpp>
#include <xmlparser/pugixml.hpp>
#include <QPrinter>
#include <QPainter>
#include <QTextDocument>
#include <boost/filesystem.hpp>
#include <fstream>

using namespace adpublisher;

#if defined Q_OS_WIN32
using namespace adpublisher::msxml;
#else
# if defined Q_OS_MAC || defined Q_OS_LINUX
using namespace adpublisher::libxslt;
# endif
#endif

printer::printer()
{
}

// static
bool
printer::print( QPrinter& printer
                , QPainter& painter
                , QRectF& drawRect
                , const pugi::xml_document& dom
                , const char * xsltfile )
{
    boost::filesystem::path xsltpath( xsltfile );

    if ( !xsltpath.is_absolute() )
        transformer::xsltpath( xsltpath, xsltfile );

    if ( !boost::filesystem::exists( xsltpath ) ) {
        ADDEBUG() << "xslfile: '" << xsltpath.string() << "' not exist";
        return false;
    }

    // dom.save_file( "output.xml" );

    QString html;
    if ( transformer::apply_template( xsltpath, dom, html ) ) {
        QTextDocument doc;
        doc.setHtml( html );
        painter.save();

        QFont font;
        font.setPointSize( 8 );
        doc.setDefaultFont( font );
        
        painter.translate( drawRect.topLeft() );
        doc.drawContents( &painter );
        painter.restore();
        // std::ofstream o( "output.html" );
        // o << html.toStdString();
        return true;
    }
    return false;
}
