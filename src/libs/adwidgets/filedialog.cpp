/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#pragma once

#include "filedialog.hpp"
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <QBoxLayout>
#include <algorithm>

namespace adwidgets {

    static int __dpi[] = { 75, 150, 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 76800, 153600 };

}

using namespace adwidgets;

FileDialog::~FileDialog()
{
}

FileDialog::FileDialog( QWidget * parent
                        , const QString& caption
                        , const QString& directory
                        , const QString& filter ) : QFileDialog( parent
                                                                 , caption
                                                                 , directory
                                                                 , filter )
{
}

void
FileDialog::setVectorCompression( const QString& text, bool checkState, const QString& fmt, int dpi )
{
    setOption( QFileDialog::DontUseNativeDialog );
    setFileMode( QFileDialog::AnyFile );
    setAcceptMode( QFileDialog::AcceptSave );

    if ( fmt == "pdf" )
        setNameFilter( tr( "PDF(*pdf);;SVG(*.svg)" ) );
    else
        setNameFilter( tr( "SVG(*.svg);;PDF(*.pdf)" ) );

    if ( auto gridLayout = dynamic_cast<QGridLayout *>(layout()) ) {

        auto cbx = new QCheckBox( this );
        cbx->setObjectName( "CbxCompressEnabled" );
        cbx->setCheckState( checkState ? Qt::Checked : Qt::Unchecked );
        cbx->setText( text );

        int row = gridLayout->rowCount();

        if ( auto layout = new QHBoxLayout ) {
            gridLayout->addWidget( cbx, row, 0 );
            gridLayout->addLayout( layout, row, 1 );

            auto combo = new QComboBox;
            combo->setObjectName( "ComboDPI" );
            for ( auto& x : __dpi )
                combo->addItem( QString( "%1" ).arg( x ) );
            auto it = std::lower_bound( __dpi, __dpi + (sizeof( __dpi ) / sizeof( __dpi[ 0 ] )), dpi );
            size_t idx = std::distance( __dpi, it );
            combo->setCurrentIndex( int( idx ) );

            layout->addWidget( new QLabel( tr("DPI:") ) );
            layout->addWidget( combo );
            layout->addItem( new QSpacerItem( 40, 0, QSizePolicy::Expanding ) );
        }
    }
}

bool
FileDialog::vectorCompression() const
{
    if ( auto cbx = findChild< QCheckBox * >( "CbxCompressEnabled" ) )
        return cbx->isChecked();
    return false;
}

int
FileDialog::dpi() const
{
    if ( auto combo = findChild< QComboBox * >( "ComboDPI" ) ) {
        int idx = combo->currentIndex();
        return __dpi[ idx ];
    }
    return 0;
}

