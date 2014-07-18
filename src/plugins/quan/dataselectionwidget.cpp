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

#include "dataselectionwidget.hpp"
#include "dataitemselector.hpp"
#include "dataselectionform.hpp"
#include "quandocument.hpp"
#include "paneldata.hpp"
#include <adportable/profile.hpp>
#include <adcontrols/datafile.hpp>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QToolButton>
#include <QFileDialog>

using namespace quan;

DataSelectionWidget::~DataSelectionWidget()
{
}

DataSelectionWidget::DataSelectionWidget(QWidget *parent) :  QWidget(parent)
                                                          , layout_( new QGridLayout )
                                                          , dataItemSelector_( new DataItemSelector )
{
    auto topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );
    topLayout->addLayout( layout_ );

    const int row = layout_->rowCount();
    layout_->addWidget( dataSelectionBar(), row, 0 );
    layout_->addWidget( dataItemSelector_.get(), row + 1, 0 );
    layout_->addWidget( new DataSelectionForm, row + 2, 0 );
}


QWidget *
DataSelectionWidget::dataSelectionBar()
{
    if ( auto toolBar = new QWidget ) {
        // toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );

        auto label = new QLabel;
        label->setStyleSheet( "QLabel { color : blue; }" );
        label->setText( "Open reference data (optional)" );
        toolBarLayout->addWidget( label );

        auto button = new QToolButton;
        button->setIcon( QIcon( ":/quan/images/fileopen.png" ) );
        button->setToolTip( tr("Open data file...") );
        toolBarLayout->addWidget( button );
        
        auto edit = new QLineEdit;
        toolBarLayout->addWidget( edit );

        connect( button, &QToolButton::clicked, this, [&] ( bool ){

                QString name = QFileDialog::getOpenFileName( this, tr("Open data file")
                                                             , adportable::profile::user_data_dir<char>().c_str()
                                                             , tr("Data Files(*.adfs *.csv *.txt *.spc)") );
                if ( !name.isEmpty() ) {
                    if ( auto edit = findChild< QLineEdit * >() ) {
                        std::shared_ptr< adcontrols::datafile > file( adcontrols::datafile::open( name.toStdWString(), true ) );
                        if ( file ) {
                            edit->setText( name );
                            dataItemSelector_->setData( file );
                        } else {
                            QMessageBox::information( 0, "Open data file", "Can't open selected file" );
                        }
                    }
                }
            } );
        
        return toolBar;
    }
    return 0;
}
