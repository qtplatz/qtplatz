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

#include "quanconfigwidget.hpp"
#include "quanconfigform.hpp"
#include "quandocument.hpp"
#include "quanconstants.hpp"
#include "paneldata.hpp"
#include <utils/styledbar.h>
#include <adcontrols/quanmethod.hpp>
#include <adportable/profile.hpp>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QToolButton>
#include <QFileDialog>
#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>
#include <fstream>

using namespace quan;

QuanConfigWidget::~QuanConfigWidget()
{
}

QuanConfigWidget::QuanConfigWidget(QWidget *parent) :  QWidget(parent)
                                                    , layout_( new QGridLayout )
                                                    , form_( new QuanConfigForm )
{
    auto topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );
    topLayout->addLayout( layout_ );

    if ( auto toolBar = new Utils::StyledBar ) {
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );
        auto label = new QLabel;
        // label->setStyleSheet( "QLabel { color : blue; }" );
        label->setText( "Configuration" );
        toolBarLayout->addWidget( label );

        if ( auto btnOpen = new QToolButton ) {
            btnOpen->setIcon( QIcon( ":/quan/images/fileopen.png" ) );
            btnOpen->setToolTip( tr( "Open configuration..." ) );
            toolBarLayout->addWidget( btnOpen );
            connect( btnOpen, &QToolButton::clicked, this, [this](bool){
                    QString file;
                    if ( auto edit = findChild< QLineEdit *>( Constants::editQuanMethodFilename ) ) {
                        file = edit->text();
                        if ( file.isEmpty() )
                            file = QString::fromStdWString( adportable::profile::user_data_dir< wchar_t >() + L"/data" );
                        file = QFileDialog::getOpenFileName( this, tr("Open configuration"), file, tr("File(*.xml)"));
                        if ( !file.isEmpty() ) {
                            edit->setText( file );
                            try {
                                adcontrols::QuanMethod m;
                                QuanDocument::instance()->load( file.toStdWString(), m );
                                QuanDocument::instance()->quanMethod( m );
                            } catch ( std::exception& ex ) {
                                QMessageBox::warning( 0, "Open Quantitative Method", boost::diagnostic_information( ex ).c_str() );
                                return;
                            }

                        }
                    }
                });
        }

        if ( auto btnSave = new QToolButton ) {
            btnSave->setIcon( QIcon( ":/quan/images/filesave.png" ) );
            btnSave->setToolTip( tr( "Save configuration..." ) );
            toolBarLayout->addWidget( btnSave );
            connect( btnSave, &QToolButton::clicked, this, [this](bool){
                    QString file;
                    if ( auto edit = findChild< QLineEdit *>( Constants::editQuanMethodFilename ) ) {
                        file = edit->text();
                        if ( file.isEmpty() )
                            file = QString::fromStdWString( adportable::profile::user_data_dir< wchar_t >() + L"/data" );
                        file = QFileDialog::getSaveFileName( this, tr("Save Quantitative Method"), file, tr("File(*.xml)"));
                        if ( !file.isEmpty() ) {
                            edit->setText( file );
                            try {
                                QuanDocument::instance()->setMethodFilename( idQuanMethod, file.toStdWString() );
                                QuanDocument::instance()->save( file.toStdWString(), QuanDocument::instance()->quanMethod() );
                            } catch ( std::exception& ex ) {
                                QMessageBox::warning( 0, "Save Quantitative Method", boost::diagnostic_information( ex ).c_str() );
                                return;
                            }
                        }
                    }
                });

            auto edit = new QLineEdit;
            edit->setObjectName( Constants::editQuanMethodFilename );
            edit->setEnabled( false );
            toolBarLayout->addWidget( edit );
            // edit->setText( QString::fromStdWString( file.wstring() ) );
            layout_->addWidget( toolBar );            
        }
        
    }

    QuanDocument::instance()->register_dataChanged( [this]( int id, bool fnChanged ){ handleDataChanged( id, fnChanged ); });
    const int row = layout_->rowCount();
    layout_->addWidget( form_.get(), row, 0 );
    form_->setContents( QuanDocument::instance()->quanMethod() );

}

QWidget *
QuanConfigWidget::fileSelectionBar()
{
    if ( auto toolBar = new QWidget ) {
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );

        auto label = new QLabel;
        label->setStyleSheet( "QLabel { color : blue; }" );
        label->setText( "Configuration" );
        toolBarLayout->addWidget( label );

        auto btnOpen = new QToolButton;
        btnOpen->setIcon( QIcon( ":/quan/images/fileopen.png" ) );
        btnOpen->setToolTip( tr("Open configuration...") );
        toolBarLayout->addWidget( btnOpen );

        auto btnSave = new QToolButton;
        btnSave->setIcon( QIcon( ":/quan/images/filesave.png" ) );
        btnSave->setToolTip( tr("Save configuration...") );
        toolBarLayout->addWidget( btnSave );
        
        auto edit = new QLineEdit;
        edit->setObjectName( "editQuanMethodName" );
        toolBarLayout->addWidget( edit );
        boost::filesystem::path dir( adportable::profile::user_data_dir<wchar_t>() );
        dir /= L"data/quan.xml";
        edit->setText( QString::fromStdWString( dir.wstring() ) );

        connect( btnOpen, &QToolButton::clicked, this, [&] ( bool ){
                QString dir;
                if ( auto edit = findChild< QLineEdit * >( Constants::editQuanMethodFilename ) )
                    dir = edit->text();
                if ( dir.isEmpty() ) {
                    boost::filesystem::path path( adportable::profile::user_data_dir<wchar_t>() );
                    dir = QString::fromStdWString( ( path / L"data/quant.xml" ).wstring() );
                }

                QString name = QFileDialog::getOpenFileName( this
                                                             , tr("Open Quantitative Analysis Configuration file")
                                                             , dir, tr("File(*.xml)") );
                if ( !name.isEmpty() ) {
                    adcontrols::QuanMethod m;
                    QuanDocument::instance()->load( name.toStdWString(), m );
                    QuanDocument::instance()->quanMethod( m );
                    QuanDocument::instance()->setMethodFilename( idQuanMethod, name.toStdWString() );
                    if ( auto edit = findChild< QLineEdit * >( Constants::editQuanMethodFilename ) )
                        edit->setText( name );
                }
            } );
        
        connect( btnSave, &QToolButton::clicked, this, [&] ( bool ){
                QString dir;
                if ( auto edit = findChild< QLineEdit * >( Constants::editQuanMethodFilename ) )
                    dir = edit->text();
                if ( dir.isEmpty() ) {
                    boost::filesystem::path path( adportable::profile::user_data_dir<wchar_t>() );
                    dir = QString::fromStdWString( ( path / L"data/quant.xml" ).wstring() );
                }
                QString name = QFileDialog::getSaveFileName( this 
                                                             , tr("Save Quantitative Analysis Configuration file")
                                                             , dir, tr("File(*.xml)") );
                if ( !name.isEmpty() ) {
                    QuanDocument::instance()->setMethodFilename( idQuanMethod, name.toStdWString() );
                    QuanDocument::instance()->save( name.toStdWString(), QuanDocument::instance()->quanMethod() );
                    if ( auto edit = findChild< QLineEdit * >( Constants::editQuanMethodFilename ) )
                        edit->setText( name );
                }
            } );
        
        return toolBar;
    }
    return 0;
}

void
QuanConfigWidget::commit()
{
    adcontrols::QuanMethod m;
    form_->getContents( m );
    QuanDocument::instance()->quanMethod( m );
}

void
QuanConfigWidget::handleDataChanged( int id, bool fnChanged )
{
    if ( id == idQuanMethod && fnChanged ) {
        if ( auto edit = findChild< QLineEdit * >( Constants::editQuanMethodFilename ) ) {
            edit->setText( QString::fromStdWString( QuanDocument::instance()->quanMethod().quanMethodFilename() ) );
        }
    }
}

