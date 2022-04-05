/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "centroidwidget.hpp"
#include "create_widget.hpp"
#include <adcontrols/processmethod.hpp>
#include <adcontrols/centroidmethod.hpp>
#include <adportable/debug.hpp>
#include <adportable/is_type.hpp>
#include <adportable/configuration.hpp>
#include <adlog/logger.hpp>
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>

#include <boost/any.hpp>

using adwidgets::CentroidWidget;

/////////////////////
namespace adwidgets {

    class CentroidWidget::impl {
    public:
        adportable::Configuration config_;
        std::array< std::tuple< double, double >, 3 > peak_width_;
        impl()  {}
        ~impl() {}
    };
}

namespace {

    struct accessor {
        const QObject * pThis;
        accessor( QObject * p ) : pThis( p ) {}
        accessor( const QObject * p ) : pThis( p ) {}

        template<typename T > T find( const QString& name ) {
            return pThis->findChild< T >( name );
        }
    };

    std::tuple< size_t, size_t >& operator ++ (std::tuple< size_t, size_t >& t ) {
        std::get<0>(t)++;
        std::get<1>(t) = 0;
        return t;
    }
}

CentroidWidget::CentroidWidget(QWidget *parent) : QWidget(parent)
                                                , impl_( std::make_unique< impl >() )
{
    auto vLayout = create_widget< QVBoxLayout >( "virticalLayout", this );
    auto hLayout = create_widget< QHBoxLayout >( "horizontalLayout" );
    auto grid = create_widget< QGridLayout >( "gridlayout" );
    vLayout->addLayout( hLayout );
    hLayout->addLayout( grid );

    grid->setSpacing( 2 );
    grid->setContentsMargins(2, 0, 2, 0);

    std::tuple< size_t, size_t > xy{0,0};

    if ( auto label = add_widget( grid, create_widget< QLabel >( "label_analyzerType", "Analyzer Type" ), std::get<0>(xy), std::get<1>(xy)++ ) ) {
        label->setTextFormat(Qt::RichText);
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    }
    if ( auto combo = add_widget( grid, create_widget< QComboBox >( "comboBox_analyzerType" ), std::get<0>(xy), std::get<1>(xy)++ ) ) {
        combo->addItems( { tr( "TOF" ), tr("Proportional"), tr( "Constant" )} );
    }

    ++xy;
    if ( auto label = add_widget( grid, create_widget< QLabel >( "label_peakWidth", "Peak Width [Da]:" ), std::get<0>(xy), std::get<1>(xy)++ ) ) {
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    }

    if ( auto spin = add_widget( grid, create_widget< QDoubleSpinBox >( "doublSpinBox_peakWidth" ), std::get<0>(xy), std::get<1>(xy)++ ) ) {
        spin->setRange( 0.00001, 10.0 );
        spin->setDecimals( 4 );
        connect( spin, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, &CentroidWidget::handlePeakWidthChanged );
    }
    ++xy;
    if ( auto label = add_widget( grid, create_widget< QLabel >( "label_atMass", tr("at <i>m/z</i>:") ), std::get<0>(xy), std::get<1>(xy)++ ) ) {
        label->setTextFormat(Qt::RichText);
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    }
    if ( auto spin = add_widget( grid, create_widget< QDoubleSpinBox >( "doublSpinBox_atMass" ), std::get<0>(xy), std::get<1>(xy)++ ) ) {
        spin->setRange( 1, 10000.0 );
        spin->setDecimals( 0 );
    }

    ++xy;
    if ( auto gbx = create_widget< QGroupBox >( "groupBox_areaHeight", tr("Centroid Intensity" ) ) ) {
        grid->addWidget( gbx, std::get<0>(xy), std::get<1>(xy), 1, 3 );
        auto gLayout = create_widget< QHBoxLayout >( "groupBox_Layout" );
        gLayout->setSpacing( 2 );
        gLayout->setContentsMargins(4, 0, 4, 0);
        gLayout->addWidget( create_widget< QRadioButton >( "radioButton_height", "Height" ) );
        gLayout->addWidget( create_widget< QRadioButton >( "radioButton_area",   "Area" ) );
        if ( auto combo = add_widget( gLayout, create_widget< QComboBox >( "comboBox_areaMethod" ) ) ) {
            combo->addItems( { tr("Intens. x mDa"), tr("Intens. x Time(ns)"), tr("Width Norm."), tr("Samp. Interval")} );
        }
        gbx->setLayout( gLayout );
    }
    ++xy;
    grid->addWidget( create_widget< QCheckBox >( "checkBox_lowpassFilter", tr("Low-pass filter (MHz)")) , std::get<0>(xy), std::get<1>(xy)++ );
    if ( auto spin = add_widget( grid, create_widget< QDoubleSpinBox >( "doublSpinBox_lowpassfilter" ), std::get<0>(xy), std::get<1>(xy)++ ) ) {
        spin->setRange( 0.0, 1000.0 );
        spin->setDecimals( 0 );
    }

    ++xy;
    grid->addWidget( create_widget< QCheckBox >( "checkBox_useTimeAxis", tr("Use time axis") ), std::get<0>(xy), std::get<1>(xy)++ );
    if ( auto label = add_widget( grid, create_widget< QLabel >( "label_useTimeAxis", tr("Peak width (&mu;s)") ), std::get<0>(xy), std::get<1>(xy)++ ) ) {
        label->setTextFormat(Qt::RichText);
    }

    grid->addWidget( create_widget< QDoubleSpinBox >( "doubleSpinBox_useTimeAxis" ), std::get<0>(xy), std::get<1>(xy)++ );

    ++xy;
    hLayout->addSpacerItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
    vLayout->addSpacerItem( new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding) );

#if defined Q_OS_MACOS
    setStyleSheet( "* { font-size: 12pt; }" );
#else
    setStyleSheet( "* { font-size: 9pt; }" );
#endif

}

CentroidWidget::~CentroidWidget()
{
}

void
CentroidWidget::OnCreate( const adportable::Configuration& config )
{
    impl_->config_ = config;
}

void
CentroidWidget::OnInitialUpdate()
{
    for ( auto& spin: findChildren< QDoubleSpinBox * >() ) {
        connect( spin, qOverload< double >(&QDoubleSpinBox::valueChanged), this, [&]{ emit valueChanged(); } );
    }

    accessor a(this);
    if ( auto combo = a.find< QComboBox * >( "comboBox_analyzerType" ) ) {
        connect( combo, qOverload< int >(&QComboBox::currentIndexChanged), this, &CentroidWidget::handleAnalyzerTypeChanged );
    }

    // connect( this, &CentroidWidget::valueChanged, this, [&]{ ADDEBUG() << "got valueChanged signal"; } );
    setValue( adcontrols::CentroidMethod{} ); // set ctor value to UI
}

void
CentroidWidget::OnFinalClose()
{
}

bool
CentroidWidget::getContents( boost::any& any ) const
{
    // is std::shared_ptr< adcontrols::ProcessMethod >
    if ( adportable::a_type< std::shared_ptr< adcontrols::ProcessMethod > >::is_a( any ) ) {
        if ( auto pm = boost::any_cast<std::shared_ptr< adcontrols::ProcessMethod >>( any ) ) {
            auto m = getValue();
            pm->appendMethod<>( m );
            // pm->appendMethod< adcontrols::CentroidMethod >( *this );
            return true;
        }

    } else if ( adportable::a_type< adcontrols::ProcessMethod >::is_pointer( any ) ) {
        adcontrols::ProcessMethod* pm = boost::any_cast<adcontrols::ProcessMethod*>( any );
        auto m = getValue(); // const_cast<CentroidWidget *>( this )->update_data();
        pm->appendMethod<>( m );
        // pm->appendMethod< adcontrols::CentroidMethod >( *this );
        return true;
    }
    return false;
}

bool
CentroidWidget::setContents( boost::any&& any )
{
    // is std::shared_ptr< adcontrols::ProcessMethod >
    if ( adportable::a_type< std::shared_ptr< const adcontrols::ProcessMethod > >::is_a( any ) ) {
        if ( auto pm = boost::any_cast<std::shared_ptr< const adcontrols::ProcessMethod >>( any ) ) {
            if ( const adcontrols::CentroidMethod * t = pm->find< adcontrols::CentroidMethod >() ) {
                setValue( *t );
                // *impl_->pMethod_ = *t;
                // update_data( *this );
                return true;
            }
        }

    } else if ( adportable::a_type< adcontrols::ProcessMethod >::is_a( any ) ) {

        const adcontrols::ProcessMethod& pm = boost::any_cast<adcontrols::ProcessMethod&>( any );
        if ( const adcontrols::CentroidMethod * t = pm.find< adcontrols::CentroidMethod >() ) {
            setValue( *t );
            //*impl_->pMethod_ = *t;
            // update_data( *this );
            return true;
        }
    }
    return false;
}

void
CentroidWidget::setValue( const adcontrols::CentroidMethod & m )
{
    using adcontrols::CentroidMethod;

    impl_->peak_width_[ CentroidMethod::ePeakWidthTOF ]          = m.peak_width( CentroidMethod::ePeakWidthTOF );
    impl_->peak_width_[ CentroidMethod::ePeakWidthProportional ] = m.peak_width( CentroidMethod::ePeakWidthProportional );
    impl_->peak_width_[ CentroidMethod::ePeakWidthConstant ]     = m.peak_width( CentroidMethod::ePeakWidthConstant );

    QSignalBlocker block( this );

    accessor a( this );

    if ( auto combo = a.find< QComboBox * >( "comboBox_analyzerType" ) ) {
        combo->setCurrentIndex( int( m.peakWidthMethod() ) );
    }
    handleAnalyzerTypeChanged( int( m.peakWidthMethod() ) );
#if 0
    // ---->
    if ( auto spin = a.find< QDoubleSpinBox * >( "doublSpinBox_atMass" ) ) {
        spin->setEnabled( m.peakWidthMethod() == CentroidMethod::ePeakWidthTOF );
    }
    if ( auto spin = a.find< QDoubleSpinBox * >( "doublSpinBox_peakWidth" ) ) {
        double width, mass;
        std::tie( width, mass ) = m.peak_width( m.peakWidthMethod() );
        spin->setValue( width );
        if ( auto spin = a.find< QDoubleSpinBox * >( "doublSpinBox_atMass" ) ) {
            if ( spin->isEnabled() )
                spin->setValue( mass );
        }
    }
#endif
    if ( auto radio = a.find< QRadioButton * >( "radioButton_area" ) ) {
        radio->setChecked( m.centroidAreaIntensity() );
    }
    if ( auto combo = a.find< QComboBox * >( "comboBox_areaMethod" ) ) {
        combo->setCurrentIndex( int( m.areaMethod() ) );
    }

    if ( auto cbx = a.find< QCheckBox * >( "checkBox_lowpassFilter" ) ) {
        cbx->setChecked( m.noiseFilterMethod() == CentroidMethod::eDFTLowPassFilter );
    }

    if ( auto spin = a.find< QDoubleSpinBox * >( "doublSpinBox_lowpassfilter" ) ) {
        spin->setValue( m.cutoffFreqHz() / 1.0e6 ); // MHz
    }

    if ( auto cbx = a.find< QCheckBox * >( "checkBox_useTimeAxis" ) ) {
        cbx->setChecked( std::get< 0 >( m.peak_process_on_time() ) ); //processOnTimeAxis() );
    }

    if ( auto spin = a.find< QDoubleSpinBox * >( "doubleSpinBox_useTimeAxis" ) ) {
        spin->setValue( std::get< 1 >( m.peak_process_on_time() ) * std::micro::den );
    }
}

adcontrols::CentroidMethod
CentroidWidget::getValue() const
{
    using adcontrols::CentroidMethod;
	CentroidMethod m;
    accessor a( this );

    if ( auto combo = a.find< QComboBox * >( "comboBox_analyzerType" ) ) {
        m.peakWidthMethod( static_cast< CentroidMethod::ePeakWidthMethod >( combo->currentIndex() ) );
    }
    if ( auto spin = a.find< QDoubleSpinBox * >( "doublSpinBox_atMass" ) ) {
        double mass = spin->value();
        if ( auto spin = a.find< QDoubleSpinBox * >( "doublSpinBox_peakWidth" ) ) {
            double width = spin->value();
            m.set_peak_width( { width, mass }, m.peakWidthMethod() );
        }
    }
    if ( auto radio = a.find< QRadioButton * >( "radioButton_area" ) ) {
        m.centroidAreaIntensity( radio->isChecked() );
    }

    if ( auto combo = a.find< QComboBox * >( "comboBox_areaMethod" ) ) {
        m.areaMethod( static_cast< CentroidMethod::eAreaMethod >( combo->currentIndex() ) );
    }

    if ( auto cbx = a.find< QCheckBox * >( "checkBox_lowpassFilter" ) ) {
        auto filter = cbx->isChecked() ? CentroidMethod::eDFTLowPassFilter : CentroidMethod::eNoFilter;
        if ( auto spin = a.find< QDoubleSpinBox * >( "doublSpinBox_lowpassfilter" ) ) {
            m.set_noise_filter( { filter, spin->value() * 1.0e6 } );
        }
    }

    if ( auto cbx = a.find< QCheckBox * >( "checkBox_useTimeAxis" ) ) {
        // m.setProcessOnTimeAxis( cbx->isChecked() );
        if ( auto spin = a.find< QDoubleSpinBox * >( "doubleSpinBox_useTimeAxis" ) ) {
            m.set_peak_process_on_time( { cbx->isChecked(), spin->value() / std::micro::den } );
            // m.setRsInSeconds( spin->value() / std::micro::den );
        }
    }
    return m;
}

void
CentroidWidget::getContents( adcontrols::ProcessMethod& pm )
{
    pm.appendMethod< adcontrols::CentroidMethod >( getValue() );
}

QSize
CentroidWidget::sizeHint() const
{
    return QSize( 274, 196 );
}

void
CentroidWidget::update()
{
}

void
CentroidWidget::on_doubleSpinBox_peakwidth_valueChanged(double arg1)
{
    (void)arg1;
    if ( ! isScoped() )
        emit valueChanged();
}

void
CentroidWidget::on_doubleSpinBox_centroidfraction_valueChanged(double arg1)
{
	(void)arg1;
    if ( ! isScoped() )
        emit valueChanged();
}

void
CentroidWidget::on_noiseFilterMethod_stateChanged(int arg1)
{
	(void)arg1;
    if ( ! isScoped() )
        emit valueChanged();
}

void
CentroidWidget::on_cutoffMHz_valueChanged(int arg1)
{
	(void)arg1;
    if ( ! isScoped() )
        emit valueChanged();
}

void
CentroidWidget::handlePeakWidthChanged( double value )
{
    if ( auto combo = accessor( this ).find< QComboBox * >( "comboBox_analyzerType" ) ) {
        int index = combo->currentIndex();
        if ( index >= 0 && index < impl_->peak_width_.size() )
            std::get< 0 >( impl_->peak_width_[ index ] ) = value;
    }
}

void
CentroidWidget::handleAnalyzerTypeChanged( int index )
{
    using adcontrols::CentroidMethod;
    CentroidMethod::ePeakWidthMethod idx = CentroidMethod::ePeakWidthMethod( index );
    accessor a(this);

    if ( auto spin = a.find< QDoubleSpinBox * >( "doublSpinBox_atMass" ) ) {
        spin->setEnabled( idx == CentroidMethod::ePeakWidthTOF );
        if ( idx == CentroidMethod::ePeakWidthTOF ) {
            spin->setValue( std::get< 1 >( impl_->peak_width_[ idx ] ) );
        }
    }
    if ( auto spin = a.find< QDoubleSpinBox * >( "doublSpinBox_peakWidth" ) ) {
        if ( index >= 0 && index < impl_->peak_width_.size() )
            spin->setValue( std::get< 0 >( impl_->peak_width_[ idx ] ) );
    }
    if ( auto label = a.find< QLabel * >( "label_peakWidth" ) ) {
        switch ( idx ) {
        case CentroidMethod::ePeakWidthTOF:            label->setText( "Peak Width [Da]:" ); break;
        case CentroidMethod::ePeakWidthProportional:   label->setText( "Proportional [ppm]:" ); break;
        case CentroidMethod::ePeakWidthConstant:       label->setText( "Constant [Da]:" ); break;
        }
    }
}
