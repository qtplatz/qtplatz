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

#include "quanreportwidget.hpp"
#include "quanconnection.hpp"
#include "quanconstants.hpp"
#include "quandocument.hpp"
#include "quanpublisher.hpp"
#include "quanquery.hpp"
#include "quanqueryform.hpp"
#include "quanresulttable.hpp"
#include <adcontrols/chemicalformula.hpp>
#include <adportable/debug.hpp>
#include <adportable/profile.hpp>
#include <adpublisher/document.hpp>
#include <adpublisher/transformer.hpp>
#include <adwidgets/progressinterface.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <xmlparser/pugixml.hpp>
#include <xmlparser/xmlhelper.hpp>
#include <xmlparser/xmlencode.hpp>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/progressmanager/progressmanager.h>
#include <extensionsystem/pluginmanager.h>
#include <utils/styledbar.h>
#include <QComboBox>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QTextBrowser>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/version.hpp>
#if BOOST_VERSION < 106000
#include <boost/uuid/uuid_io.hpp>
#endif
#include <boost/spirit/include/classic.hpp>
#include <fstream>
#include <algorithm>

#ifdef Q_OS_MAC
const QString qrcpath = ":/adpublisher/images/mac";
#else
const QString qrcpath = ":/adpublisher/images/win";
#endif

using namespace quan;

QuanReportWidget::~QuanReportWidget()
{
}

QuanReportWidget::QuanReportWidget(QWidget *parent) : QWidget(parent)
                                                    , layout_( new QVBoxLayout( this ) )
                                                    , docBrowser_( new QTextBrowser )
{
    if ( auto toolBar = new QToolBar ) {

        toolBar->setObjectName( "publisherToolBar" );

        if ( auto xslt = new QComboBox() ) {
            xslt->setObjectName( "stylesCombo" );
            QStringList list;
            adpublisher::transformer::populateStylesheets( list );
            xslt->addItems( list );
            toolBar->addWidget( xslt );

            if ( auto settings = QuanDocument::instance()->settings_ptr() ) {
                auto lastfile = settings->value( "QuanReport/XSLFILE" ).toString();
                if ( ! lastfile.isEmpty() ) {
                    int i = list.indexOf( lastfile );
                    if ( i >= 0 )
                        xslt->setCurrentText( lastfile );
                }
            }
        }
        layout_->addWidget( toolBar );

        if ( auto am = Core::ActionManager::instance() ) {

            if ( auto menuContainer = am->createMenu( Constants::PUBLISHER_FILE_MENU ) ) {
                menuContainer->menu()->setTitle( tr( "Publisher" ) );
                setupFileActions( menuContainer->menu(), toolBar );
                am->actionContainer( Core::Constants::M_FILE )->addMenu( menuContainer );
            }
        }
    }

    layout_->addWidget( docBrowser_.get() );
    layout_->setStretch( 1, 10 );
    QSizePolicy policy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}

void
QuanReportWidget::onInitialUpdate( QuanDocument * d )
{
    connect( d, &QuanDocument::onConnectionChanged, this, &QuanReportWidget::handleConnectionChanged );
}

void
QuanReportWidget::setupFileActions( QMenu * menu, QToolBar * tb )
{
    if ( tb ) {
        QAction * a;

        a = new QAction( QIcon( ":/quan/images/run.png" ), tr( "Publish" ), this );
        a->setPriority(QAction::LowPriority);
        connect(a, &QAction::triggered, this, &QuanReportWidget::filePublish );
        tb->addAction(a);
        menu->addAction( a );
    }

}

void
QuanReportWidget::filePublish()
{
    qtwrapper::waitCursor w;

    QString xslfile = currentStylesheet();

    if ( auto settings = QuanDocument::instance()->settings_ptr() )
        settings->setValue( "QuanReport/XSLFILE", xslfile );

    adwidgets::ProgressInterface progress(0, 5);
        
    Core::ProgressManager::addTask( progress.progress.future()
                                    , "Quan connecting database..."
                                    , Constants::QUAN_TASK_OPEN );

    auto future = std::async( std::launch::async, [=](){
            return publishTask( xslfile, progress );
        } );

    while ( std::future_status::ready != future.wait_for( std::chrono::milliseconds( 100 ) ) )
        QCoreApplication::instance()->processEvents();

    auto pair = future.get();

    docBrowser_->setHtml( pair.first );
    QDesktopServices::openUrl( QUrl( pair.second ) );
}

std::pair< QString, QString >
QuanReportWidget::publishTask( const QString& xslfile, adwidgets::ProgressInterface progress )
{
    if ( auto publisher = QuanDocument::instance()->publisher() ) {

        try {
            
            auto conn = QuanDocument::instance()->connection();
            ( *publisher )( conn, progress ); //, article->xml_document().get() );

        } catch ( boost::exception& ex ) {
            QMessageBox::information( this, "QuanReportWidget", ( boost::diagnostic_information( ex ) + "(1)").c_str() );
            return std::make_pair( "", "" );
        } catch ( ... ) {
            QMessageBox::information( this, "QuanReportWidget", ( boost::current_exception_diagnostic_information() + "(2)").c_str() );
            return std::make_pair( "", "" );
        }

        try {

            publisher->appendTraceData( progress );

        } catch ( boost::exception& ex ) {
            QMessageBox::information( this, "QuanReportWidget", ( boost::diagnostic_information( ex ) + "(3)").c_str() );
            return std::make_pair( "", "" );
        } catch ( ... ) {
            QMessageBox::information( this, "QuanReportWidget", ( boost::current_exception_diagnostic_information() + "(4)").c_str() );
            return std::make_pair( "", "" );
        }

        boost::filesystem::path path = publisher->filepath(); 
                
        publisher->save_file( path.string().c_str() ); // save publisher document xml
        
        QString output, method;
        adpublisher::document::apply_template( path.string().c_str(), xslfile.toStdString().c_str(), output, method );
        
        if ( !output.isEmpty() ) {
            
            std::string extension =
                ( method.isEmpty() || method == "xhtml" ) ? ".html"
                : (QString( ".%1" ).arg( method )).toStdString();

            path.replace_extension( extension );

            boost::filesystem::ofstream o( path );
            o << output.toStdString();

            return std::make_pair( output, QString::fromStdString( path.string() ) );
        }
    }
    return std::make_pair( "", "" );
}

QString
QuanReportWidget::currentStylesheet() const
{
    if ( auto combo = findChild< QComboBox * >( "stylesCombo" ) ) {
        return combo->currentText();
    }
    return QString();
}

void
QuanReportWidget::filePrintPdf()
{
}

void
QuanReportWidget::fileDebug()
{
    auto publisher = QuanDocument::instance()->publisher();
    if ( *publisher ) { // if processed
        publisher->save_file( publisher->filepath().string().c_str() );
        std::ostringstream o;
        publisher->save( o );
        docBrowser_->setText( QString::fromStdString( o.str() ) );
    }
}

void
QuanReportWidget::handleConnectionChanged()
{
    // fileDebug();
}
