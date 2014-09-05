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

#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QCoreApplication>
#include <QFileDialog>
#include <QSettings>
#include <QToolButton>

#include <adpublisher/document.hpp>
#include <adpublisher/doceditor.hpp>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <sstream>
#include <fstream>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
                                        , ui( new Ui::MainWindow )
                                        , docEditor_( new adpublisher::docEditor )
                                        , doc_( std::make_shared< adpublisher::document >() )
{
    ui->setupUi( this );
    setCentralWidget( docEditor_.get() );

    setToolButtonStyle( Qt::ToolButtonFollowStyle );
    ui->actionOpen->setIcon( QIcon( ":/adpublisher/images/win/fileopen.png" ) );
    ui->actionSave_As->setIcon( QIcon( ":/adpublisher/images/win/filesave.png" ) );
    ui->actionApply->setIcon( QIcon( ":/publisher/run.png" ) );

    docEditor_->setupTextActions( ui->menuBar->addMenu( tr( "Format" ) ) );
    docEditor_->setupEditActions( ui->menuBar->addMenu( tr( "Edit" ) ) );

    if ( auto btn = new QToolButton ) {
        btn->setDefaultAction( ui->actionOpen );
        ui->mainToolBar->addWidget( btn );
    }
    if ( auto btn = new QToolButton ) {
        btn->setDefaultAction( ui->actionSave_As );
        ui->mainToolBar->addWidget( btn );
    }

    if ( auto btn = new QToolButton ) {
        btn->setDefaultAction( ui->actionApply );
        ui->mainToolBar->addWidget( btn );
    }
    
    ui->actionApply->setEnabled( false );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void
MainWindow::onInitialUpdate( std::shared_ptr< QSettings >& settings )
{
    settings_ = settings;
    std::string filename = settings->fileName().toStdString();
    std::vector< QString > list;
    getRecentFiles( "Stylesheets", "DIRS", list, "DIR" );

    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::handleOpenFile );
    connect(ui->actionSave_As, &QAction::triggered, this, &MainWindow::handleSaveTemplateAs );
    connect(ui->actionApply, &QAction::triggered, this, &MainWindow::handleApplyStylesheet );
}

void
MainWindow::addRecentFiles( const QString& group, const QString& pfx, const QString& value, const QString& key )
{
    addRecentFiles( *settings_, group, pfx, value, key );
}

void
MainWindow::addRecentFiles( QSettings& settings
                            , const QString& group, const QString& pfx, const QString& value, const QString& key )
{
    std::vector< QString > list;
    getRecentFiles( settings, group, pfx, list, key );

    boost::filesystem::path path = boost::filesystem::path( value.toStdWString() ).generic_wstring();
    auto it = std::remove_if( list.begin(), list.end(), [path] ( const QString& a ){ return path == a.toStdWString(); } );
    if ( it != list.end() )
        list.erase( it, list.end() );

    settings.beginGroup( group );

    settings.beginWriteArray( pfx );
    settings.setArrayIndex( 0 );
    settings.setValue( key, QString::fromStdWString( path.generic_wstring() ) );
    for ( size_t i = 0; i < list.size() && i < 7; ++i ) {
        settings.setArrayIndex( int(i + 1) );
        settings.setValue( key, list[ i ] );
    }
    settings.endArray();

    settings.endGroup();
}

void
MainWindow::getRecentFiles( const QString& group, const QString& pfx, std::vector<QString>& list, const QString& key ) const
{
    getRecentFiles( *settings_, group, pfx, list, key );
}

void
MainWindow::getRecentFiles( QSettings& settings, const QString& group, const QString& pfx, std::vector<QString>& list, const QString& key )
{
    settings.beginGroup( group );

    int size = settings.beginReadArray( pfx );
    for ( int i = 0; i < size; ++i ) {
        settings.setArrayIndex( i );
        list.push_back( settings.value( key ).toString() );
    }
    settings.endArray();

    settings.endGroup();
}

QString
MainWindow::recentFile( const QString& group, const QString& pfx, const QString& key ) const
{
    QString value;

    settings_->beginGroup( group );
    
    if ( int size = settings_->beginReadArray( pfx ) ) {
        (void)size;
        settings_->setArrayIndex( 0 );
        value = settings_->value( key ).toString();
    }
    settings_->endArray();
    
    settings_->endGroup();

    return value;
}

void
MainWindow::handleOpenFile()
{
    auto name = QFileDialog::getOpenFileName( this
                                              , tr( "Open QtPlatz publisher xml")
                                              , recentFile( "RecentFiles", "Files" )
                                              , tr( "QtPlatz publisher xml(*.xml)" ) );
    if ( !name.isEmpty() ) {
        addRecentFiles( "RecentFiles", "Files", name );

        auto doc = std::make_shared< adpublisher::document>();

        if ( doc->load_file( name.toStdString().c_str() ) ) {

            setWindowTitle( name );
            xmlpath_ = name.toStdString();

            ui->actionApply->setEnabled( true );
            processed_.clear();

            docEditor_->setDocument( doc );
        }
    }
}

void
MainWindow::handleApplyStylesheet()
{
    processed_.clear();

    QString method;
    QString xslpath = docEditor_->currentStylesheet();

    if ( adpublisher::document::apply_template( xmlpath_.c_str(), xslpath.toStdString().c_str(), processed_, method ) ) {
        docEditor_->setOutput( processed_ );
    }
}

void
MainWindow::handleSaveProcessedAs()
{
    if ( xmlpath_.empty() || processed_.isEmpty() )
        return;

    boost::filesystem::path outfile( xmlpath_ );
    outfile.replace_extension( ".html" );
    
    auto name = QFileDialog::getSaveFileName( this
                                              , tr( "Save Result" )
                                              , QString::fromStdString( outfile.string() )
                                              , tr( "HTML Files(*.html);;XML Files(*.xml)" ) );

    if ( !name.isEmpty() ) {
        std::ofstream of( name.toStdString() );
        of << processed_.toStdString();
    }
}

void
MainWindow::handleSaveTemplateAs()
{
    docEditor_->fileSave();
}

