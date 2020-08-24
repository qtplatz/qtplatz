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

#include "moltableview.hpp"
#include "scanlawdialog2.hpp"
#include "scanlawform.hpp"
#include "moltablehelper.hpp"
#include <QApplication>
#include <QBoxLayout>
#include <QClipboard>
#include <QDebug>
#include <QDialogButtonBox>
#include <QMenu>
#include <QMetaType>
#include <QMimeData>
#include <QPushButton>
#include <QSplitter>
#include <QStandardItemModel>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontrols/mspeak.hpp>
#include <adcontrols/mspeaks.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adportable/debug.hpp>
#include <adportable/polfit.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/uuid/uuid.hpp>
#include <cmath>
#include <ratio>

Q_DECLARE_METATYPE( boost::uuids::uuid )

namespace adwidgets {

    class ScanLawDialog2::impl {
    public:
        enum columns { c_id, c_formula, c_mass, c_time, c_mode, c_error };

        impl() : model1busy_( false )
               , model_( std::make_unique< QStandardItemModel >() )
               , model2_( std::make_unique< QStandardItemModel >() )  {

            model_->setColumnCount( 6 );
            model_->setHeaderData( c_id,      Qt::Horizontal, QObject::tr( "id" ) );
            model_->setHeaderData( c_formula, Qt::Horizontal, QObject::tr( "Formula" ) );
            model_->setHeaderData( c_mass,    Qt::Horizontal, QObject::tr( "<i>m/z</i>" ) );
            model_->setHeaderData( c_time,    Qt::Horizontal, QObject::tr( "Time(&mu;s)" ) );
            model_->setHeaderData( c_mode,    Qt::Horizontal, QObject::tr( "Lap#" ) );
            model_->setHeaderData( c_error,   Qt::Horizontal, QObject::tr( "Error (mDa)" ) );

            model2_->setColumnCount( 3 );
            model2_->setHeaderData( 0,      Qt::Horizontal, QObject::tr( "name" ) );
            model2_->setHeaderData( 1,      Qt::Horizontal, QObject::tr( "Accl.(V)" ) );
            model2_->setHeaderData( 2,      Qt::Horizontal, QObject::tr( "T<sub>0</sub>(&mu;s)" ) );

        }
        bool model1busy_;

        std::unique_ptr< QStandardItemModel > model_;
        std::unique_ptr< QStandardItemModel > model2_;
        boost::uuids::uuid spectrometer_uuid_;
        std::shared_ptr< adcontrols::MassSpectrometer > spectrometer_;
    };

    class ScanLawDialog2_archive {
    public:
        ScanLawDialog2_archive() : L_( 0.5 ), acclVolts_( 4500.0 ), t0_( 0.0 ) {}

        ScanLawDialog2_archive( const ScanLawDialog2& t ) : L_( t.length() )
                                                          , acclVolts_( t.acceleratorVoltage() )
                                                          , t0_( t.tDelay() ) {
            t.read( peaks_ );
        }

        void load( ScanLawDialog2& t ) {
            t.setLength( L_ );
            t.setAcceleratorVoltage( acclVolts_ );
            t.setTDelay( t0_ );
            t.impl_->model_->setRowCount( 0 );
            for ( auto& pk: peaks_ ) {
                t.addPeak( pk.spectrumIndex()
                           , QString::fromStdString( pk.formula() )
                           , pk.time()
                           , pk.mass()
                           , pk.mode() );
            }
        }

    private:
        friend class boost::serialization::access;
        template< class Archive >
        void serialize( Archive& ar, const unsigned int ) {
            ar & BOOST_SERIALIZATION_NVP( L_ );
            ar & BOOST_SERIALIZATION_NVP( acclVolts_ );
            ar & BOOST_SERIALIZATION_NVP( t0_ );
            ar & BOOST_SERIALIZATION_NVP( peaks_ );
        }
        double L_;
        double acclVolts_;
        double t0_;
        adcontrols::MSPeaks peaks_;
    };


};


using namespace adwidgets;

ScanLawDialog2::ScanLawDialog2(QWidget *parent) : QDialog(parent)
                                                , impl_( std::make_unique< impl >() )
{
    setContextMenuPolicy( Qt::CustomContextMenu );

    connect( this, &QDialog::customContextMenuRequested, [this]( const QPoint& pt ){
            QMenu menu;
            menu.addAction( tr( "Copy" ), this, SLOT( handleCopyToClipboard() ) );
            menu.addAction( tr( "Paste" ), this, SLOT( handlePaste() ) );
            menu.exec( mapToGlobal( pt ) );
        });

    connect( impl_->model_.get(), &QStandardItemModel::dataChanged, this
             , [this](const QModelIndex& _1, const QModelIndex& _2, const QVector<int>& _3 ) {
                 if ( _1.column() == impl::c_formula ) {
                     QSignalBlocker block( impl_->model_.get() );
                     double exactMass = MolTableHelper::monoIsotopicMass( _1.data( Qt::EditRole ).toString() );
                     impl_->model_->setData( impl_->model_->index( _1.row(), impl::c_mass ), exactMass, Qt::EditRole );
                 }
                 if ( !impl_->model1busy_ )
                     commit();
             } );

    connect( impl_->model2_.get(), &QStandardItemModel::dataChanged, this
             , [this](const QModelIndex& _1, const QModelIndex& _2, const QVector<int>& _3 ) {
                 if ( auto form = findChild< ScanLawForm * >() )
                     updateObservers( form->tDelay(), form->acceleratorVoltage() );
             } );

    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setMargin(4);
        layout->setSpacing(2);

        if ( QSplitter * splitter1 = new QSplitter ) {

            auto moltable = new MolTableView();
            moltable->setObjectName( "peakTable" );
            moltable->setContextMenuHandler( [&]( const QPoint& pt ){ handlePeakTableMenu( pt ); } );

            splitter1->addWidget( moltable );
            auto objtable = new MolTableView();
            objtable->setObjectName( "objText" );
            splitter1->addWidget( objtable );
            splitter1->setOrientation ( Qt::Vertical );
            splitter1->setStretchFactor( 0, 2 );
            splitter1->setStretchFactor( 0, 0 );

            if ( QSplitter * splitter = new QSplitter ) {
                splitter->addWidget( ( new ScanLawForm ) );
                splitter->addWidget( splitter1 );
                splitter->setStretchFactor( 0, 0 );
                splitter->setStretchFactor( 1, 1 );
                splitter->setOrientation ( Qt::Horizontal );
                layout->addWidget( splitter );
            }
        }

        if ( auto table = findChild< MolTableView * >("peakTable" ) ) {

            table->setModel( impl_->model_.get() );
            table->setColumnHidden( impl::c_id, true );
            table->setColumnField( impl::c_formula, ColumnState::f_formula, true, true ); // editable, checkable
            table->setColumnField( impl::c_mass, ColumnState::f_mass );
            table->setColumnField( impl::c_time, ColumnState::f_time );
            table->setPrecision( impl::c_time, 4 );

            table->onInitialUpdate();
        }

        if ( auto table = findChild< MolTableView * >( "objText" ) ) {
            table->setModel( impl_->model2_.get() );
        }

        if ( auto buttons = findChild< QDialogButtonBox * >() ) {

            connect( buttons->button( QDialogButtonBox::Apply ), &QPushButton::clicked, this, [&](){ QDialog::accept(); } );

            connect( buttons, &QDialogButtonBox::rejected, this, [&](){ QDialog::reject(); } );
        }

        if ( auto form = findChild< ScanLawForm * >() ) {
            form->setLength( 0.5 );
            form->setAcceleratorVoltage( 4000.0 );
            form->setTDelay( 0.0 );
            connect( form, &ScanLawForm::valueChanged, this, [this]( int id ){
                    switch( id ) {
                    case 0: handleLengthChanged(); break;
                    case 1: handleAcceleratorVoltageChanged(); break;
                    case 2: handleTDelayChanged(); break;
                    }
                });
        }

    }

    resize( 600, 300 );
}

ScanLawDialog2::~ScanLawDialog2()
{
}

void
ScanLawDialog2::setSpectrometerData( const boost::uuids::uuid& id, const QString& desc, double length )
{
    impl_->spectrometer_uuid_ = id;
    impl_->spectrometer_ = adcontrols::MassSpectrometerBroker::make_massspectrometer( id );
    setLength( length );
}

void
ScanLawDialog2::setLength( double value )
{
    // if ( auto form = findChild< ScanLawForm * >() ) {
    //     form->setLength( value );
    //     if ( impl_->spectrometer_ )
    //         impl_->spectrometer_->setScanLaw( form->acceleratorVoltage(), form->tDelay(), form->length() );
    // }
}

void
ScanLawDialog2::setAcceleratorVoltage( double value )
{
    if ( auto form = findChild< ScanLawForm * >() ) {
        form->setAcceleratorVoltage( value );
        if ( impl_->spectrometer_ )
            impl_->spectrometer_->setAcceleratorVoltage( form->acceleratorVoltage(), form->tDelay() );
    }
}

void
ScanLawDialog2::setTDelay( double value )
{
    if ( auto form = findChild< ScanLawForm * >() ) {
        form->setTDelay( value );
        if ( impl_->spectrometer_ )
            impl_->spectrometer_->setAcceleratorVoltage( form->acceleratorVoltage(), form->tDelay() );
    }
}

double
ScanLawDialog2::length() const
{
    if ( auto form = findChild< ScanLawForm * >() ) {
        return form->length();
    }
    return 0;
}

double
ScanLawDialog2::acceleratorVoltage() const
{
    if ( auto form = findChild< ScanLawForm * >() )
        return form->acceleratorVoltage();
    return 0;
}

double
ScanLawDialog2::tDelay() const
{
    if ( auto form = findChild< ScanLawForm * >() )
        return form->tDelay();
    return 0;
}

size_t
ScanLawDialog2::peakCount() const
{
    return impl_->model_->rowCount();
}

void
ScanLawDialog2::addPeak( uint32_t id, const QString& formula, double time, double matchedMass, int mode )
{
    auto row = impl_->model_->rowCount();
    auto& model = *impl_->model_;

    impl_->model1busy_ = true;

    model.setRowCount( row + 1 );
    model.setData( model.index( row, impl::c_id ), id, Qt::EditRole );
    model.setData( model.index( row, impl::c_formula ), formula, Qt::EditRole );
    double exact_mass = MolTableHelper::monoIsotopicMass( formula, "" );
    model.setData( model.index( row, impl::c_mass), exact_mass, Qt::EditRole );
    model.setData( model.index( row, impl::c_time), time * std::micro::den, Qt::EditRole );
    model.setData( model.index( row, impl::c_mode), mode );
    model.setData( model.index( row, impl::c_error), ( exact_mass - matchedMass ) * std::milli::den, Qt::EditRole );

    if ( auto item = model.item( row, impl::c_formula ) ) {
        item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
        model.setData( model.index( row, impl::c_formula ), Qt::Checked, Qt::CheckStateRole );
        item->setEditable( true );
    }

    impl_->model1busy_ = false;
}

void
ScanLawDialog2::updateMassError()
{
    auto& model = *impl_->model_;

    if ( auto form = findChild< ScanLawForm * >() ) {
        adportable::TimeSquaredScanLaw scanlaw( form->acceleratorVoltage()
                                                , form->tDelay() / std::micro::den, form->length() );

        for ( int row = 0; row < model.rowCount(); ++row ) {
            double time = model.index( row, impl::c_time ).data( Qt::EditRole ).toDouble() / std::micro::den;
            double exact_mass = model.index( row, impl::c_mass ).data( Qt::EditRole ).toDouble();
            double mass = scanlaw.getMass( time, 0 );
            model.setData( model.index( row, impl::c_error), ( exact_mass - mass ) * std::milli::den, Qt::EditRole );
        }
    }
}

void
ScanLawDialog2::updateObservers( double t0, double acclVolts )
{
    auto& model = *impl_->model2_;

    for ( int row = 0; row < model.rowCount(); ++row ) {
        if ( model.index( row, 0 ).data( Qt::CheckStateRole ).toBool() ) {
            model.setData( model.index( row, 1 ), acclVolts, Qt::EditRole );
            model.setData( model.index( row, 2 ), t0, Qt::EditRole );
        } else {
            double va = model.index( row, 1 ).data( Qt::UserRole + 1 ).toDouble();
            double t0 = model.index( row, 2 ).data( Qt::UserRole + 1 ).toDouble(); // (microseconds)
            model.setData( model.index( row, 1 ),  va, Qt::EditRole ); // copy from backup
            model.setData( model.index( row, 2 ),  t0, Qt::EditRole ); // copy from backup (microseconds)
        }
    }
}

bool
ScanLawDialog2::commit()
{
    if ( auto moltable = findChild< MolTableView * >( "peakTable" ) )
        moltable->resizeColumnsToContents();

    adcontrols::MSPeaks peaks;

    if ( read( peaks ) ) {

        if ( auto form = findChild< ScanLawForm * >() ) {

            double t0, acclVolts;

            if ( estimateAcceleratorVoltage( t0, acclVolts, peaks ) ) {
                form->setAcceleratorVoltage( acclVolts );
                form->setTDelay( t0 * std::micro::den );

                updateMassError();

                return true;
            }
        }
    }
    return false;
}

void
ScanLawDialog2::handleLengthChanged()
{
    adcontrols::MSPeaks peaks;

    if ( read( peaks ) ) {

        if ( auto form = findChild< ScanLawForm * >() ) {

            double t0, acclVolts;

            if ( estimateAcceleratorVoltage( t0, acclVolts, peaks ) ) {
                form->setAcceleratorVoltage( acclVolts );
                form->setTDelay( t0 * std::micro::den );

                updateMassError();
                updateObservers( t0, acclVolts );
            }
        }
    }
}

void
ScanLawDialog2::handleAcceleratorVoltageChanged()
{
    adcontrols::MSPeaks peaks;

    if ( read( peaks ) ) {

        if ( auto form = findChild< ScanLawForm * >() ) {

            double t0, L;

            if ( estimateLength( t0, L, peaks ) ) {

                form->setLength( L );
                form->setTDelay( t0 * std::micro::den );
                form->setLengthPrecision( 6 );

                updateMassError();
                updateObservers( t0, form->acceleratorVoltage() );
            }
        }
    }
}

void
ScanLawDialog2::handleTDelayChanged()
{
}

bool
ScanLawDialog2::read( adcontrols::MSPeaks& peaks ) const
{
    const auto& m = *impl_->model_;
    //const size_t rowCount = m.rowCount();

    peaks.clear();

    for ( int row = 0; row < impl_->model_->rowCount(); ++row ) {

        auto formula = m.index( row, impl::c_formula ).data( Qt::EditRole ).toString();
        double exact_mass = m.index( row, impl::c_mass ).data( Qt::EditRole ).toDouble();

        if ( m.index( row, impl::c_formula ).data( Qt::CheckStateRole ).toBool() ) {

            if ( ! formula.isEmpty() && exact_mass > 0.5 ) {

                peaks << adcontrols::MSPeak( formula.toStdString()
                                             , 0.0 // mass (observed)
                                             , m.index( row, impl::c_time ).data( Qt::EditRole ).toDouble() / std::micro::den // time
                                             , m.index( row, impl::c_mode ).data( Qt::EditRole ).toInt()   // mode
                                             , m.index( row, impl::c_id ).data( Qt::EditRole ).toInt() // spectrum index
                                             , m.index( row, impl::c_mass ).data( Qt::EditRole ).toDouble() );
            }
        }
    }

    return peaks.size();
}

void
ScanLawDialog2::addObserver( const boost::uuids::uuid& uuid, const QString& objtext, double va, double t0, bool checked )
{
    auto& model = *impl_->model2_;
    int row = model.rowCount();

    model.setRowCount( row + 1 );

    model.setData( model.index( row, 0 ), objtext, Qt::EditRole );
    model.setData( model.index( row, 0 ), QVariant::fromValue( uuid ), Qt::UserRole + 1 );
    model.setData( model.index( row, 1 ), va, Qt::EditRole );
    model.setData( model.index( row, 1 ), va, Qt::UserRole + 1 ); // save original value
    model.setData( model.index( row, 2 ), t0 * std::micro::den, Qt::EditRole );
    model.setData( model.index( row, 2 ), t0 * std::micro::den, Qt::UserRole + 1 ); // save original value

    if ( auto item = model.item( row, 0 ) ) {
        item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
        model.setData( model.index( row, 0 ), checked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
    }
}

QVector< QString >
ScanLawDialog2::checkedObservers() const
{
    QVector< QString > list;

    auto& model = *impl_->model2_;

    for ( int row = 0; row < model.rowCount(); ++row ) {
        if ( model.index( row, 0 ).data( Qt::CheckStateRole ).toBool() )
            list << model.index( row, 0 ).data( Qt::EditRole ).toString();
    }
    return list;
}

//////////////
bool
ScanLawDialog2::estimateAcceleratorVoltage( double& t0, double& v, const adcontrols::MSPeaks& peaks ) const
{
    const double L = findChild< ScanLawForm * >()->length();

    if ( peaks.size() < 1 )
        return false;

    if ( peaks.size() == 1 ) {
        t0 = 0.0;
        const adcontrols::MSPeak& pk = peaks[ 0 ];
        double l = impl_->spectrometer_ ? impl_->spectrometer_->scanLaw()->fLength( pk.mode() ) : L;
        v = adportable::TimeSquaredScanLaw::acceleratorVoltage( pk.exact_mass(), pk.time(), l, t0 );
        return true;

    } else if ( peaks.size() >= 2 ) {

        std::vector<double> x, y, coeffs;

        for ( auto& pk : peaks ) {
            double l = impl_->spectrometer_ ? impl_->spectrometer_->scanLaw()->fLength( pk.mode() ) : L;
            qDebug() << "l=" << l << " mode=" << pk.mode();
            x.push_back( std::sqrt( pk.exact_mass() ) * l );
            y.push_back( pk.time() );
        }

        if ( adportable::polfit::fit( x.data(), y.data(), x.size(), 2, coeffs ) ) {

            t0 = coeffs[ 0 ];
            double t1 = adportable::polfit::estimate_y( coeffs, 1.0 ); // estimate tof for m/z = 1.0, 1mL
            v = adportable::TimeSquaredScanLaw::acceleratorVoltage( 1.0, t1, 1.0, t0 );

            return true;
        }
    }
    return false;
}

bool
ScanLawDialog2::estimateLength( double& t0, double& L, const adcontrols::MSPeaks& peaks ) const
{
    if ( peaks.size() < 1 )
        return false;

    const double V = findChild< ScanLawForm * >()->acceleratorVoltage();

    if ( peaks.size() == 1 ) {
        t0 = 0.0;
        const adcontrols::MSPeak& pk = peaks[ 0 ];
        L = sqrt( ( adportable::kTimeSquaredCoeffs * V * pk.time() * pk.time() ) / pk.exact_mass() );
        return true;

    } else if ( peaks.size() >= 2 ) {

        std::vector<double> x, y, coeffs;

        for ( auto& pk : peaks ) {
            x.push_back( std::sqrt( pk.exact_mass() ) ); // assume L = 1.0m
            y.push_back( pk.time() );
        }

        if ( adportable::polfit::fit( x.data(), y.data(), x.size(), 2, coeffs ) ) {

            t0 = coeffs[ 0 ];
            double t1 = adportable::polfit::estimate_y( coeffs, 1.0 );   // estimate tof for m/z = 1.0, for 1m

            double t = t1 - t0;
            L = sqrt( V * adportable::kTimeSquaredCoeffs * ( t * t ) );

            return true;
        }

    }
    return false;

}

void
ScanLawDialog2::handleCopyToClipboard()
{
	QString selected_text;

    ScanLawDialog2_archive x( *this );

	selected_text.append( QString::number( acceleratorVoltage(), 'e', 14 ) );
	selected_text.append( '\t' );
	selected_text.append( QString::number( tDelay(), 'e', 14 ) );

	QMimeData * md = new QMimeData();
	md->setText( selected_text );

    std::wostringstream os;
    try {
        boost::archive::xml_woarchive ar ( os );
        ar & boost::serialization::make_nvp( "scanlaw", x );
        QString xml( QString::fromStdWString( os.str() ) );
        md->setData( QLatin1String( "application/scanlaw-xml" ), xml.toUtf8() );
    } catch ( std::exception& ex ) {
        BOOST_THROW_EXCEPTION( ex );
    }

	QApplication::clipboard()->setMimeData( md );
}

void
ScanLawDialog2::handlePaste()
{
    auto md = QApplication::clipboard()->mimeData();
    auto data = md->data( "application/scanlaw-xml" );
    if ( !data.isEmpty() ) {
        QString utf8( QString::fromUtf8( data ) );
        std::wistringstream is( utf8.toStdWString() );
        boost::archive::xml_wiarchive ar( is );
        try {
            ScanLawDialog2_archive x;
            ar & boost::serialization::make_nvp( "scanlaw", x );
            x.load( *this );
        } catch ( std::exception& ex ) {
            BOOST_THROW_EXCEPTION( ex );
        }
    }
}

void
ScanLawDialog2::handlePeakTableMenu( const QPoint& pt )
{
    QMenu menu;
    menu.addAction( tr( "Add peak" ), this, SLOT( handleAddPeak() ) );
    if ( auto table = findChild< MolTableView * >( "peakTable" ) )
        menu.exec( table->mapToGlobal( pt ) );
}

void
ScanLawDialog2::handleAddPeak()
{
    auto row = impl_->model_->rowCount();
    auto& model = *impl_->model_;

    int mode = 0;
    double mass = MolTableHelper::monoIsotopicMass( "H" );
    double time = 1.0e-6;
    if ( impl_->spectrometer_ ) {
        auto scanLaw = impl_->spectrometer_->scanLaw();
        time = scanLaw->getTime( mass, mode );
    }

    addPeak( -1, "H", time, mass, mode );

    for ( int col = impl::c_formula; col < impl::c_error; ++col ) {
        if ( auto item = model.item( row, col ) )
            item->setEditable( true );
    }
}

//////////////
