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

#include "xchromatogramsform.hpp"
#include "create_widget.hpp"
#include <adcontrols/constants.hpp>
#include <adcontrols/controlmethod/tofchromatogramsmethod.hpp>
#include <adcontrols/controlmethod/tofchromatogrammethod.hpp>
#include <QWidget>
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QSpacerItem>
#include <QLineEdit>

namespace adwidgets {

    class XChromatogramsForm::impl {
    public:
        impl() : layout_(0) {
        }
        QVBoxLayout * layout_;
    };

}

using namespace adwidgets;

XChromatogramsForm::XChromatogramsForm( QWidget * parent ) : QWidget( parent )
                                                               , impl_( new impl() )
{
    setObjectName( "XChromatogramsForm" );
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
    gridLayout->addWidget( create_widget<QLabel>( "labelRefresh", "Refresh histogram" ), row, col++ );
    gridLayout->addWidget( create_widget<QCheckBox>( "cbxRefresh" ), row, col++ );

    row++; col = 0;
    gridLayout->addWidget( create_widget<QLabel>( "TIC", "TIC" ), row, col++ );
    if ( auto cb = create_widget< QComboBox >( "algo" ) ) {
        cb->addItems( QStringList{ "Area", "Counts", "None" } );
        gridLayout->addWidget( cb, row, col++ );
    }
    row++; col = 0;
    gridLayout->addWidget( create_widget<QLabel>( "MSCalib", "MS Calibration" ), row, col++ );
    if ( auto edit = create_widget< QLineEdit >("mscalibname") ) {
        edit->setReadOnly( true );
        gridLayout->addWidget( edit, row, col++ );
    }
    row++; col = 0;
    gridLayout->addItem( new QSpacerItem( 20, 20, QSizePolicy::Maximum, QSizePolicy::Maximum ), row, col );

#if 0
    impl_->layout_->addItem( new QSpacerItem( 40, 20, QSizePolicy::Maximum, QSizePolicy::Expanding ) );
    impl_->layout_->addWidget( create_widget< QPushButton >( "applyButton", tr( "Apply" ) ) );

    if ( auto button = findChild< QPushButton *>( "applyButton" ) ) {
        connect( button, &QPushButton::pressed, [this] () { emit valueChanged(); } );
    }
#endif

    if ( auto cb = findChild< QComboBox * >( "algo" ) ) {
        connect( cb, qOverload< int >(&QComboBox::currentIndexChanged), this, [&](int){ emit valueChanged(); } );
    }

    if ( auto spin = findChild< QSpinBox * >( "numTriggers" ) ) {
        spin->setMaximum( 10000 );
        connect( spin, static_cast<void( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), [this] ( int ) { emit valueChanged(); } );
    }

    if ( auto cbx = findChild<QCheckBox *>( "cbxRefresh" ) ) {
        connect( cbx, &QCheckBox::stateChanged, [this] ( double ) { emit valueChanged(); } );
    }

}

XChromatogramsForm::~XChromatogramsForm()
{
}

void
XChromatogramsForm::OnInitialUpdate()
{
}

void
XChromatogramsForm::getContents( adcontrols::TofChromatogramsMethod& m ) const
{
    if ( auto spin = findChild< QSpinBox * >( "numTriggers" ) )
        m.setNumberOfTriggers( spin->value() );

    if ( auto cbx = findChild< QCheckBox * >( "cbxRefresh" ) )
        m.setRefreshHistogram( cbx->isChecked() );

    if ( auto cb = findChild< QComboBox * >( "algo" ) ) {
        if ( cb->currentIndex() == 0 )
            m.setTIC( { true, adcontrols::xic::ePeakAreaOnProfile } );
        else if ( cb->currentIndex() == 1 )
            m.setTIC( { true, adcontrols::xic::eCounting } );
        else
            m.setTIC( { false, adcontrols::xic::ePeakAreaOnProfile } );
    }
}

void
XChromatogramsForm::setContents( const adcontrols::TofChromatogramsMethod& m )
{
    QSignalBlocker block( this );
    if ( auto spin = findChild< QSpinBox * >( "numTriggers" ) ) {
        // QSignalBlocker block( spin );
        size_t n = m.numberOfTriggers() == 0 ? 1 : m.numberOfTriggers();
        spin->setValue( int( n ) );
    }
    if ( auto cbx = findChild< QCheckBox * >( "cbxRefresh" ) ) {
        // QSignalBlocker block( cbx );
        cbx->setChecked( m.refreshHistogram() );
    }
    if ( auto cb = findChild< QComboBox * >( "algo" ) ) {
#if __cplusplus >= 201703L
        auto [enable, algo] = m.tic();
#else
        auto enable = std::get<0>( m.tic() );
        auto algo = std::get<1>( m.tic() );
#endif
        if ( enable ) {
            if ( algo == adcontrols::xic::ePeakAreaOnProfile )
                cb->setCurrentIndex( 0 );
            if ( algo == adcontrols::xic::eCounting )
                cb->setCurrentIndex( 1 );
        } else {
            cb->setCurrentIndex( 2 );
        }
    }
}

void
XChromatogramsForm::setDigitizerMode( bool digitizer )
{
    if ( auto spin = findChild< QSpinBox * >( "numTriggers" ) )
        spin->setEnabled( digitizer );
}

void
XChromatogramsForm::setCalibrationFilename( QString&& stem, QString&& filename )
{
    if ( auto edit = findChild< QLineEdit * >( "mscalibname" ) ) {
        edit->setText( stem );
        edit->setToolTip( filename );
    }
}
