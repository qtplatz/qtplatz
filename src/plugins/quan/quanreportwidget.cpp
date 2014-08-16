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
#include "quanmethodcomplex.hpp"
#include "quanquery.hpp"
#include "quanqueryform.hpp"
#include "quanresulttable.hpp"
#include <adportable/profile.hpp>
#include <adpublisher/doceditor.hpp>
#include <adpublisher/document.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <coreplugin/actionmanager/actionmanager.h>
#include <utils/styledbar.h>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QStandardItemModel>
#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>
#include <fstream>
#include <algorithm>

using namespace quan;

QuanReportWidget::~QuanReportWidget()
{
}

QuanReportWidget::QuanReportWidget(QWidget *parent) : QWidget(parent)
                                                    , layout_( new QVBoxLayout( this ) )
                                                    , docEditor_( new adpublisher::docEditor )
{
    if ( auto toolBar = new Utils::StyledBar ) {

        layout_->addWidget( toolBar );
        
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 2 );
        toolBarLayout->setSpacing( 2 );

        if ( auto btnOpen = new QToolButton ) {
            btnOpen->setIcon( QIcon( ":/quan/images/fileopen.png" ) );
            btnOpen->setToolTip( tr("Import Report Format...") );
            toolBarLayout->addWidget( btnOpen );
            connect( btnOpen, &QToolButton::clicked, this, [this] ( bool ){ importDocTemplate(); } );
        }
        if ( auto btnSave = new QToolButton ) {
            btnSave->setDefaultAction( Core::ActionManager::instance()->command( Constants::QUAN_METHOD_SAVE )->action() );
            btnSave->setToolTip( tr("Save Quan Method...") );
            toolBarLayout->addWidget( btnSave );
        }

        auto edit = new QLineEdit;
        edit->setObjectName( Constants::editQuanMethodName );
        toolBarLayout->addWidget( edit );

        if ( auto btnExport = new QToolButton ) {
            btnExport->setIcon( QIcon( ":/quan/images/filesave.png" ) );
            btnExport->setToolTip( tr("Export XML...") );
            toolBarLayout->addWidget( btnExport );
            connect( btnExport, &QToolButton::clicked, this, [this]( bool ){ exportDocTemplate(); } );
        }

    } // end toolbar
    //QSizePolicy policy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    //docEditor_->setSizePolicy( policy );
    //auto w = new QWidget;
    //docEditor_.reset( new adpublisher::docEditor( w ) );
    layout_->addWidget( docEditor_.get() );
    layout_->setStretch( 1, 10 );
    QSizePolicy policy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    docEditor_->setSizePolicy( policy );
}

void
QuanReportWidget::importDocTemplate()
{
    QString name = QFileDialog::getOpenFileName( this
                                                 , tr( "Import ReportFormat..." )
                                                 , QuanDocument::instance()->lastMethodDir()
                                                 , tr( "Quan Method Files(*.qmth);;XML Files(*.xml)" ) );
    if ( !name.isEmpty() ) {
        QuanMethodComplex m;
        QuanDocument::instance()->load( name.toStdWString(), m );
        QuanDocument::instance()->method( m.docTemplate() );
    }
}

void
QuanReportWidget::exportDocTemplate()
{
    boost::filesystem::path path( QuanDocument::instance()->lastMethodDir().toStdWString() );
    path.remove_filename();
    path /= "reportTemplate.xml";

    QString name = QFileDialog::getSaveFileName( this
                                                 , tr( "Export doc template..." )
                                                 , QString::fromStdWString( path.wstring() )
                                                 , tr( "XML Files(*.xml)" ) );
    if ( !name.isEmpty() ) {
        auto doc = docEditor_->document();
        doc->save_file( name.toUtf8() );
    }
}

