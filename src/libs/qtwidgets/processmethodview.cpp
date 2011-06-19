/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "processmethodview.hpp"
#include <xmlparser/pugixml.hpp>
#include <adportable/configuration.hpp>
#include <qtwrapper/qstring.hpp>
#include <QMessageBox>
#include <QDeclarativeError>
#include <QDeclarativeContext>
#include <fstream>

using namespace qtwidgets;

ProcessMethodView::ProcessMethodView(QWidget *parent) : QDeclarativeView(parent)
                                                      , pConfig_( new adportable::Configuration )
{
}

ProcessMethodView::~ProcessMethodView()
{
}

void
ProcessMethodView::OnCreate( const adportable::Configuration& config )
{
    *pConfig_ = config;

    std::wstring xml = config.xml();

    pugi::xml_document dom;
    pugi::xml_parse_result result;
    if ( ! ( result = dom.load( pugi::as_utf8( xml ).c_str() ) ) )
        return;

    QDeclarativeContext * ctx = rootContext();
    ctx->setContextProperty( "configXML", qtwrapper::qstring::copy( xml ) );

#if defined DEBUG && 0
    do {
      std::ofstream of( "/Users/thondo/src/qtplatz/config.xml" );
      dom.save( of );
    } while(0);
#endif

    pugi::xpath_node node = dom.select_single_node( "//Component[@type='qml']" );
    if ( node ) {
        std::string source = node.node().attribute( "QUrl" ).value();
        setSource( QUrl( source.c_str() ) );

        QList< QDeclarativeError > errors = this->errors();
        for ( QList< QDeclarativeError >::const_iterator it = errors.begin(); it != errors.end(); ++it )
            QMessageBox::warning( this, "QDeclarativeError", it->description() );
    } else
        return;
}

void
ProcessMethodView::OnInitialUpdate()
{
}

void
ProcessMethodView::OnFinalClose()
{
}

QSize
ProcessMethodView::sizeHint() const
{
    return QSize( 300, 250 );
}

// slot
void
ProcessMethodView::getContents( adcontrols::ProcessMethod& )
{
}

// slot
void
ProcessMethodView::getLifeCycle( adplugin::LifeCycle *& p )
{
    p = static_cast< adplugin::LifeCycle *>(this);
}
