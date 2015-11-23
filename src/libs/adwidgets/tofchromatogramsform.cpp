/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "tofchromatogramsform.hpp"
#include "create_widget.hpp"
#include <adcontrols/tofchromatogramsmethod.hpp>
#include <QWidget>
#include <QBoxLayout>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>

namespace adwidgets {

    class TofChromatogramsForm::impl {
    public:
        impl() : layout_(0) {
        }
        QVBoxLayout * layout_;
    };
    
}

using namespace adwidgets;

TofChromatogramsForm::TofChromatogramsForm( QWidget * parent ) : QWidget( parent )
                                                               , impl_( new impl() )
{
    setObjectName( "TofChromatogramsForm" );
    impl_->layout_ = new QVBoxLayout( this );
    
    auto gridLayout = new QGridLayout();
    gridLayout->setObjectName( "gridLayout" );
    impl_->layout_->addLayout( gridLayout );
    
    resize( QSize( 100, 80 ) );
    
    int row = 0;
    int col = 0;
    gridLayout->addWidget( create_widget<QLabel>( "label1", "# of triggers" ), row, col++ );
    gridLayout->addWidget( create_widget<QSpinBox>( "numTriggers" ), row, col++ );
            
    row++; col = 0;
    gridLayout->addWidget( create_widget<QLabel>( "label2", "Response(s)" ), row, col++ );
    gridLayout->addWidget( create_widget<QDoubleSpinBox>( "response" ), row, col++ );

    impl_->layout_->addItem( new QSpacerItem( 40, 20, QSizePolicy::Maximum, QSizePolicy::Expanding ) );

    // impl_->layout_->addWidget( create_widget< QPushButton >( "minusButton", tr( "-" ) ) );
    // impl_->layout_->addWidget( create_widget< QPushButton >( "plusButton", tr( "+" ) ) );
    // setStyleSheet( "QPushButton#addRow { max-width: 1em; border-style: outset; border-width: 1px; border-color: beige; font: bold 14px; }"
    //                      "QPushButton#addRow:pressed { background-color: rgb(224,0,0); border-style: inset; }"
    //                      );

    impl_->layout_->addWidget( create_widget< QPushButton >( "applyButton", tr( "Apply" ) ) );

    if ( auto button = findChild< QPushButton *>( "applyButton" ) ) {
        connect( button, &QPushButton::pressed, [this] () { emit applyTriggered(); } );
    }

    if ( auto spin = findChild< QSpinBox * >( "numTriggers" ) ) {
        spin->setMaximum( 10000 );
        connect( spin, static_cast<void( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), [this] ( int ) { emit valueChanged(); } );
    }

    if ( auto spin = findChild<QDoubleSpinBox *>( "response" ) ) {
        connect( spin, static_cast<void( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), [this] ( double ) { emit valueChanged(); } );
    }

}

TofChromatogramsForm::~TofChromatogramsForm()
{
}

void
TofChromatogramsForm::OnInitialUpdate()
{
}

void
TofChromatogramsForm::getContents( adcontrols::TofChromatogramsMethod& m ) const
{
    if ( auto spin = findChild< QSpinBox * >( "numTriggers" ) )
        m.setNumberOfTriggers( spin->value() );

    //if ( auto spin = findChild< QDoubleSpinBox *>( "response" ) )
    //    ; // not in use
}

void
TofChromatogramsForm::setContents( const adcontrols::TofChromatogramsMethod& m )
{
    if ( auto spin = findChild< QSpinBox * >( "numTriggers" ) ) {
        size_t n = m.numberOfTriggers() == 0 ? 1 : m.numberOfTriggers();
        spin->setValue( int( n ) );
    }
}
