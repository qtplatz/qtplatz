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
#include "paneldata.hpp"
#include <adcontrols/quanmethod.hpp>
#include <adportable/profile.hpp>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QToolButton>
#include <QFileDialog>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
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

    const int row = layout_->rowCount();
    layout_->addWidget( fileSelectionBar(), row, 0 );
    layout_->addWidget( form_.get(), row + 1, 0 );

    form_->setContents( *QuanDocument::instance()->quanMethod() );
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
        toolBarLayout->addWidget( edit );

        connect( btnOpen, &QToolButton::clicked, this, [&] ( bool ){
                boost::filesystem::path dir( adportable::profile::user_data_dir<wchar_t>() );
                dir /= L"data";

                QString name = QFileDialog::getOpenFileName( this
                                                             , tr("Open Quantitative Analysis Configuration file")
                                                             , QString::fromStdWString( dir.wstring() )
                                                             , tr("File(*.xml)") );
                if ( !name.isEmpty() ) {
                    adcontrols::QuanMethod m;
                    try {
                        std::ifstream inf( name.toStdString() );
                        boost::archive::xml_iarchive ar( inf );
                        ar >> BOOST_SERIALIZATION_NVP( m );
                    } catch ( std::exception& ex ) {
                        QMessageBox::warning( 0, "Open Quantitative Method", boost::diagnostic_information( ex ).c_str() );
                        return;
                    }
                    *QuanDocument::instance()->quanMethod() = m;
                    if ( auto edit = findChild< QLineEdit * >() )
                        edit->setText( name );
                }
            } );
        
        connect( btnSave, &QToolButton::clicked, this, [&] ( bool ){
                boost::filesystem::path dir( adportable::profile::user_data_dir<wchar_t>() );
                dir /= L"data";                
                QString name = QFileDialog::getSaveFileName( this 
                                                             , tr("Save Quantitative Analysis Configuration file")
                                                             , QString::fromStdWString( dir.wstring() )
                                                             , tr("File(*.xml)") );
                if ( !name.isEmpty() ) {
                    try {
                        std::ofstream outf( name.toStdString() );
                        boost::archive::xml_oarchive ar( outf );
                        ar << boost::serialization::make_nvp( "QuanMethod", *QuanDocument::instance()->quanMethod() );
                    } catch ( std::exception& ex ) {
                        QMessageBox::warning( 0, "Save Quantitative Method", boost::diagnostic_information( ex ).c_str() );
                        return;
                    }
                    if ( auto edit = findChild< QLineEdit * >() )
                        edit->setText( name );

                    if ( auto edit = findChild< QLineEdit * >() )
                        edit->setText( name );
                }
            } );
        
        return toolBar;
    }
    return 0;
}
