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
//#include <QDeclarativeError>
//#include <QDeclarativeContext>
//#include <QDeclarativeEngine>
#include <QCoreApplication>
#include <fstream>
#include "centroidmethodmodel.hpp"
#include "isotopemethodmodel.hpp"

using namespace qtwidgets;

ProcessMethodView::ProcessMethodView(QWidget *parent) : QWidget(parent)
                                                      , pConfig_( new adportable::Configuration )
                                                      , pCentroidModel_( new CentroidMethodModel )
                                                      , pIsotopeModel_( new IsotopeMethodModel )
                                                      , pElementalCompModel_( new ElementalCompModel )
{
    pIsotopeModel_->appendFormula( adcontrols::IsotopeMethod::Formula( L"", L"C13NH12NH2O", L"H", 1, 1.0) );
    pIsotopeModel_->appendFormula( adcontrols::IsotopeMethod::Formula( L"", L"C13NH12NH2O", L"CH3COOH", 1, 1.0) );
}

ProcessMethodView::~ProcessMethodView()
{
    delete pConfig_;
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

    qmlRegisterType< CentroidMethodModel >( "com.scienceliaison.qml", 1, 0, "CentroidModel" );
	// qmlRegisterType< IsotopeMethodModel > ( "com.scienceliaison.qml", 1, 0, "IsotopeModel" );
    qmlRegisterType< ElementalCompModel > ( "com.scienceliaison.qml", 1, 0, "ElementalCompModel" );
	// qmlRegisterType< MSCalibrateModel > ( "com.scienceliaison.qml", 1, 0, "MSCalibrateModel" );

    QDeclarativeContext * ctx = rootContext();
    ctx->setContextProperty( "configXML", qtwrapper::qstring::copy( xml ) );
    ctx->setContextProperty( "centroidModel", pCentroidModel_.get() );
	// ctx->setContextProperty( "isotopeModel", pIsotopeModel_.get() );
    ctx->setContextProperty( "elementalCompModel", pElementalCompModel_.get() );
	// ctx->setContextProperty( "msCalibrateModel", pMSCalibrateModel_.get() );
    setResizeMode( QDeclarativeView::SizeRootObjectToView );

#if defined DEBUG && 0
    do {
      std::ofstream of( "/Users/thondo/src/qtplatz/config.xml" );
      dom.save( of );
    } while(0);
#endif

    QString qmlpath;
#ifdef Q_OS_MAC
    qmlpath = QCoreApplication::applicationDirPath() + "/../Resources";
#else
    qmlpath = QCoreApplication::applicationDirPath() + "/../share";
#endif

    // engine()->addImportPath( QCoreApplication::applicationDirPath() + "/../imports" );
    // QML_IMPORT_PATH

    pugi::xpath_node node = dom.select_single_node( "//Component[@type='qml']" );
    if ( node ) {
        QString source = qmlpath + node.node().attribute( "QUrl" ).value();
        setSource( QUrl::fromLocalFile( source ) );

        QList< QDeclarativeError > errors = this->errors();
        for ( QList< QDeclarativeError >::const_iterator it = errors.begin(); it != errors.end(); ++it )
            QMessageBox::warning( this, "QDeclarativeError", it->toString() + " file: " + source );
    }
}

void
ProcessMethodView::OnInitialUpdate()
{
}

void
ProcessMethodView::OnFinalClose()
{
}

bool
ProcessMethodView::getContents( boost::any& ) const
{
    return false;
}

bool
ProcessMethodView::setContents( boost::any& )
{
    return false;
}


QSize
ProcessMethodView::sizeHint() const
{
    return QSize( 300, 250 );
}

// slot
void
ProcessMethodView::getContents( adcontrols::ProcessMethod& m )
{
    m.appendMethod( pCentroidModel_->method() );
	// m.appendMethod( pIsotopeModel_->method() );
    //ctx->setContextProperty( "elementalCompModel", pElementalCompModel_.get() );
    //ctx->setContextProperty( "msCalibrateModel", pMSCalibrateModel_.get() );
}

// slot
void
ProcessMethodView::getLifeCycle( adplugin::LifeCycle *& p )
{
    p = static_cast< adplugin::LifeCycle *>(this);
}
