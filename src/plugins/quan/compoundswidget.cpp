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

#include "compoundswidget.hpp"
#include "compoundstable.hpp"
#include "quandocument.hpp"
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/quancompounds.hpp>
#include <adportable/profile.hpp>
#include <adlog/logger.hpp>
#include <utils/styledbar.h>
#include <QFileDialog>
#include <QGridLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QMessageBox>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <fstream>

using namespace quan;

CompoundsWidget::~CompoundsWidget()
{
    commit();
    auto& c = QuanDocument::instance()->quanCompounds();
    if ( c.size() > 0 ) {
        boost::filesystem::path file( adportable::profile::user_data_dir< wchar_t >() + L"/data/quancomponents_default.xml" );
        try {
            save( file, c );
        } catch ( ... ) {
            // ignore error
        }
    }
}

CompoundsWidget::CompoundsWidget(QWidget *parent) : QWidget(parent)
                                                  , layout_( new QGridLayout )
                                                  , table_( new CompoundsTable )
{
    auto topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );
    topLayout->addLayout( layout_ );

    QuanDocument::instance()->register_dataChanged( [this](int id){ handleDataChanged( id ); } );

    if ( auto toolBar = new Utils::StyledBar ) {
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );
        if ( auto btnOpen = new QToolButton ) {
            btnOpen->setIcon( QIcon( ":/quan/images/fileopen.png" ) );
            btnOpen->setToolTip( tr( "Open compoinent file..." ) );
            toolBarLayout->addWidget( btnOpen );
            
            connect( btnOpen, &QToolButton::clicked, this, [this](bool){
                    QString file;
                    if ( auto edit = findChild< QLineEdit *>() ) {
                        file = edit->text();
                        if ( file.isEmpty() )
                            file = QString::fromStdWString( adportable::profile::user_data_dir< wchar_t >() + L"/data" );
                        file = QFileDialog::getOpenFileName( this, tr("Open compounds file"), file, tr("File(*.xml)"));
                        try {
                            adcontrols::QuanCompounds m;
                            if ( load( file.toStdWString(), m ) )
                                QuanDocument::instance()->quanCompounds( m );
                        } catch ( std::exception& ex ) {
                            QMessageBox::warning( 0, "Open Quantitative Method", boost::diagnostic_information( ex ).c_str() );
                            return;
                        }
                    }
                });
        }
        if ( auto btnSave = new QToolButton ) {
            btnSave->setIcon( QIcon( ":/quan/images/filesave.png" ) );
            btnSave->setToolTip( tr( "Save compoinents..." ) );
            toolBarLayout->addWidget( btnSave );
            connect( btnSave, &QToolButton::clicked, this, [this](bool){
                    QString file;
                    if ( auto edit = findChild< QLineEdit *>() ) {
                        file = edit->text();
                        if ( file.isEmpty() )
                            file = QString::fromStdWString( adportable::profile::user_data_dir< wchar_t >() + L"/data" );
                        file = QFileDialog::getSaveFileName( this, tr("Save compounds"), file, tr("File(*.xml)"));
                        if ( !file.isEmpty() ) {
                            edit->setText( file );
                            try {
                                commit();
                                save( file.toStdWString(), QuanDocument::instance()->quanCompounds() );
                            } catch ( std::exception& ex ) {
                                QMessageBox::warning( 0, "Open Quantitative Method", boost::diagnostic_information( ex ).c_str() );
                                return;
                            }
                        }
                    }
                });
        }
        if ( auto edit = new QLineEdit ) {
            toolBarLayout->addWidget( edit );
            toolBarLayout->addWidget( new Utils::StyledSeparator );

            boost::filesystem::path file( adportable::profile::user_data_dir< wchar_t >() + L"/data/quancomponents_default.xml" );
            if ( boost::filesystem::exists( file ) ) {
                try {
                    adcontrols::QuanCompounds c;
                    if ( load( file, c ) )
                        QuanDocument::instance()->quanCompounds( c );
                    edit->setText( QString::fromStdWString( file.wstring() ) );
                } catch ( ... ) {
                    ADERROR() << boost::current_exception_diagnostic_information();
                }
            }
        }
        layout_->addWidget( toolBar );
    }
    const int row = layout_->rowCount();
    layout_->addWidget( table_.get(), row, 0 );
}


bool
CompoundsWidget::load( const boost::filesystem::path& path, adcontrols::QuanCompounds& c )
{
    try {
        boost::filesystem::ifstream inf( path );
        boost::archive::xml_iarchive ar( inf );
        ar >> BOOST_SERIALIZATION_NVP( c );
    } catch ( std::exception& ex ) {
        ADERROR() << boost::diagnostic_information( ex );
        throw ex;
    }
    return true;
}

bool
CompoundsWidget::save( const boost::filesystem::path& path, const adcontrols::QuanCompounds& c )
{
    try {
        boost::filesystem::ofstream outf( path );
        boost::archive::xml_oarchive ar( outf );
        ar << boost::serialization::make_nvp( "QuanCompounds", c );
    } catch ( std::exception& ex ) {
        ADERROR() << boost::diagnostic_information( ex );
        throw ex;
    }
    return false;
}

void
CompoundsWidget::commit()
{
    adcontrols::QuanCompounds c; // uuid is bing updated.
    table_->getContents( c );
    QuanDocument::instance()->quanCompounds( c );
}

void
CompoundsWidget::handleDataChanged( int id )
{
    if ( id == idQuanMethod ) {
        table_->handleQuanMethod( QuanDocument::instance()->quanMethod() );
    }
    else if ( id == idQuanCompounds ) {
        table_->setContents( QuanDocument::instance()->quanCompounds() );
    }
}
