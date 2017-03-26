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
#include "quanprogress.hpp"
#include <adcontrols/chemicalformula.hpp>
#include <adportable/debug.hpp>
#include <adportable/profile.hpp>
#include <adpublisher/document.hpp>
#include <adpublisher/transformer.hpp>
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
        auto tbLayout = new QHBoxLayout( toolBar );
        tbLayout->setMargin( 0 );
        tbLayout->setSpacing( 0 );
        tbLayout->addWidget( new Utils::StyledSeparator );

        if ( auto xslt = new QComboBox( toolBar ) ) {
            xslt->setObjectName( "stylesCombo" );
            QStringList list;
            adpublisher::transformer::populateStylesheets( list );
            xslt->addItems( list );
            toolBar->addWidget( xslt );
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
#if 0
#ifndef QT_NO_PRINTER
        a = new QAction( QIcon::fromTheme("exportpdf", QIcon(qrcpath + "/exportpdf.png")), tr("&Export PDF..."), this );
        a->setPriority(QAction::LowPriority);
        a->setShortcut(Qt::CTRL + Qt::Key_D);
        connect(a, SIGNAL(triggered()), this, SLOT(filePrintPdf()));
        tb->addAction(a);
        menu->addAction( a );
#endif
#endif
    }

}

void
QuanReportWidget::filePublish()
{
    ProgressHandler progress;
    qtwrapper::waitCursor w;
    
    Core::ProgressManager::addTask( progress.progress.future(), "Quan connecting database...", Constants::QUAN_TASK_OPEN );

    if ( auto publisher = QuanDocument::instance()->publisher() ) {

        try {
            
            auto conn = QuanDocument::instance()->connection();
            ( *publisher )( conn, progress ); //, article->xml_document().get() );

        } catch ( boost::exception& ex ) {
            QMessageBox::information( this, "QuanReportWidget", ( boost::diagnostic_information( ex ) + "(1)").c_str() );
            return;
        } catch ( ... ) {
            QMessageBox::information( this, "QuanReportWidget", ( boost::current_exception_diagnostic_information() + "(2)").c_str() );
            return;
        }

        try {

            publisher->appendTraceData( progress );

        } catch ( boost::exception& ex ) {
            QMessageBox::information( this, "QuanReportWidget", ( boost::diagnostic_information( ex ) + "(3)").c_str() );
            return;
        } catch ( ... ) {
            QMessageBox::information( this, "QuanReportWidget", ( boost::current_exception_diagnostic_information() + "(4)").c_str() );
            return;
        }

        boost::filesystem::path path = publisher->filepath(); 
                
        publisher->save_file( path.string().c_str() ); // save publisher document xml
        
        QString xslfile = currentStylesheet();
        QString output, method;
        adpublisher::document::apply_template( path.string().c_str(), xslfile.toStdString().c_str(), output, method );
        
        if ( !output.isEmpty() ) {
            
            docBrowser_->setHtml( output );
            
            std::string extension =
                ( method.isEmpty() || method == "xhtml" ) ? ".html"
                : (QString( ".%1" ).arg( method )).toStdString();

            path.replace_extension( extension );

            boost::filesystem::ofstream o( path );
            o << output.toStdString();

            QDesktopServices::openUrl( QUrl( QString::fromStdWString( path.wstring() ) ) ) ;
        }
    }
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
    fileDebug();
}
