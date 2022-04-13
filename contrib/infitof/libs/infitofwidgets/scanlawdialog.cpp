/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "scanlawdialog.hpp"
#include "scanlawform.hpp"
#include <admtcontrols/scanlaw.hpp>
#include <admtcontrols/infitof.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontrols/mspeak.hpp>
#include <adcontrols/mspeaks.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adportable/debug.hpp>
#include <adportable/polfit.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adwidgets/moltablehelper.hpp>
#include <adwidgets/moltableview.hpp>
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
// #include <boost/archive/archive_exception.hpp>
// #include <boost/archive/xml_wiarchive.hpp>
// #include <boost/archive/xml_woarchive.hpp>
// #include <boost/serialization/access.hpp>
// #include <boost/serialization/export.hpp>
// #include <boost/serialization/nvp.hpp>
#include <boost/uuid/uuid.hpp>
#include <cmath>
#include <map>
#include <numeric>
#include <ratio>
#include <set>

Q_DECLARE_METATYPE( boost::uuids::uuid )

namespace infitofwidgets {

    class ScanLawDialog::impl {
    public:
        enum columns { c_id, c_formula, c_matched_mass, c_time, c_mode, c_error };

        impl() : model1busy_( false )
               , model_( std::make_unique< QStandardItemModel >() )
               , model2_( std::make_unique< QStandardItemModel >() )  {

            model_->setColumnCount( 6 );
            model_->setHeaderData( c_id,      Qt::Horizontal, QObject::tr( "id" ) );
            model_->setHeaderData( c_formula, Qt::Horizontal, QObject::tr( "Formula" ) );
            model_->setHeaderData( c_matched_mass,  Qt::Horizontal, QObject::tr( "<i>m/z</i>" ) );
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
        std::shared_ptr< admtcontrols::ScanLaw > scanlaw_;
    };

    // class ScanLawDialog_archive {
    // public:
    //     ScanLawDialog_archive() : L_( 0.5 ), acclVolts_( 4500.0 ), t0_( 0.0 ) {}

    //     ScanLawDialog_archive( const ScanLawDialog& t ) { //: L_( t.length() )
    //     //  , acclVolts_( t.acceleratorVoltage() )
    //     //                                                , t0_( t.tDelay() ) {
    //         //t.read( peaks_ );
    //     }

    //     void load( ScanLawDialog& t ) {
    //         //t.setLength( L_ );
    //         //t.setAcceleratorVoltage( acclVolts_ );
    //         //t.setTDelay( t0_ );
    //         //t.impl_->model_->setRowCount( 0 );
    //         //for ( auto& pk: peaks_ ) {
    //         //    t.addPeak( pk.spectrumIndex()
    //         //               , QString::fromStdString( pk.formula() )
    //         //               , pk.time()
    //         //               , pk.mass()
    //         //               , pk.mode() );
    //         //}
    //     }

    // private:
    //     // friend class boost::serialization::access;
    //     // template< class Archive >
    //     // void serialize( Archive& ar, const unsigned int ) {
    //     //     ar & BOOST_SERIALIZATION_NVP( L_ );
    //     //     ar & BOOST_SERIALIZATION_NVP( acclVolts_ );
    //     //     ar & BOOST_SERIALIZATION_NVP( t0_ );
    //     //     ar & BOOST_SERIALIZATION_NVP( peaks_ );
    //     // }
    //     double L_;
    //     double acclVolts_;
    //     double t0_;
    //     adcontrols::MSPeaks peaks_;
    // };
};

//BOOST_CLASS_EXPORT( infitofwidgets::ScanLawDialog_archive )

using namespace infitofwidgets;

ScanLawDialog::ScanLawDialog(QWidget *parent) : QDialog(parent)
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
                     double matchedMass = impl_->model_->index( _1.row(), impl::c_matched_mass ).data( Qt::EditRole ).toDouble();
                     double exactMass = adwidgets::MolTableHelper::monoIsotopicMass( _1.data( Qt::EditRole ).toString() );
                     double error = ( exactMass - matchedMass ) * std::milli::den;
                     impl_->model_->setData( impl_->model_->index( _1.row(), impl::c_error ), error, Qt::EditRole );
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

            auto moltable = new adwidgets::MolTableView();
            moltable->setObjectName( "peakTable" );
            moltable->setContextMenuHandler( [&]( const QPoint& pt ){ handlePeakTableMenu( pt ); } );

            splitter1->addWidget( moltable );
            auto objtable = new adwidgets::MolTableView();
            objtable->setObjectName( "objText" );
            splitter1->addWidget( objtable );
            splitter1->setOrientation ( Qt::Vertical );
            splitter1->setStretchFactor( 0, 5 );
            splitter1->setStretchFactor( 1, 0 );

            if ( auto hLayout = new QHBoxLayout ) {

                hLayout->addWidget( new ScanLawForm );
                hLayout->addWidget( splitter1 );

                hLayout->setStretchFactor( splitter1, 10 );
                layout->addLayout( hLayout );
            }
        }

        if ( auto table = findChild< adwidgets::MolTableView * >("peakTable" ) ) {

            table->setModel( impl_->model_.get() );
            table->setColumnHidden( impl::c_id, true );
            table->setColumnField( impl::c_formula, adwidgets::ColumnState::f_formula, true, true ); // editable, checkable
            table->setColumnField( impl::c_time, adwidgets::ColumnState::f_time );
            table->setPrecision( impl::c_time, 5 );

            table->onInitialUpdate();
        }

        if ( auto table = findChild< adwidgets::MolTableView * >( "objText" ) ) {
            table->setModel( impl_->model2_.get() );
        }

        if ( auto buttons = findChild< QDialogButtonBox * >() ) {

            connect( buttons->button( QDialogButtonBox::Apply ), &QPushButton::clicked, this, [&](){ QDialog::accept(); } );

            connect( buttons, &QDialogButtonBox::rejected, this, [&](){ QDialog::reject(); } );
        }

        if ( auto form = findChild< ScanLawForm * >() ) {
            form->setNlaps( 0 );
            form->setAcceleratorVoltage( 4000.0 );
            form->setTDelay( 0.0 );
        }

    }

    resize( 800, 460 );
}

ScanLawDialog::~ScanLawDialog()
{
}

void
ScanLawDialog::setScanLaw( std::shared_ptr< admtcontrols::ScanLaw > scanlaw )
{
    impl_->scanlaw_ = scanlaw;
}

void
ScanLawDialog::setSpectrometerData( const boost::uuids::uuid& id, const QString& desc, std::shared_ptr< adcontrols::MassSpectrometer > sp )
{
    impl_->spectrometer_uuid_ = id;
    impl_->spectrometer_ = sp;
    //setLength( length );
}

void
ScanLawDialog::setAcceleratorVoltage( double value, bool origin )
{
    if ( auto form = findChild< ScanLawForm * >() )
        form->setAcceleratorVoltage( value, origin );
}

void
ScanLawDialog::setTDelay( double value, bool origin )
{
    if ( auto form = findChild< ScanLawForm * >() )
        form->setTDelay( value, origin );
}

void
ScanLawDialog::setL1( double length, bool origin )
{
    if ( auto form = findChild< ScanLawForm * >() )
        form->setL1( length, origin );
}

void
ScanLawDialog::setLinearLength( double l )
{
    if ( auto form = findChild< ScanLawForm * >() )
        form->setLinearLength( l );
}

void
ScanLawDialog::setOrbitalLength( double l )
{
    if ( auto form = findChild< ScanLawForm * >() )
        form->setOrbitalLength( l );
}

double
ScanLawDialog::acceleratorVoltage() const
{
    if ( auto form = findChild< ScanLawForm * >() )
        return form->acceleratorVoltage();
    return 0;
}

double
ScanLawDialog::tDelay() const
{
    if ( auto form = findChild< ScanLawForm * >() )
        return form->tDelay();
    return 0;
}

double
ScanLawDialog::L1() const
{
    if ( auto form = findChild< ScanLawForm * >() )
        return form->L1();
    return 0;
}

size_t
ScanLawDialog::peakCount() const
{
    return impl_->model_->rowCount();
}

void
ScanLawDialog::addPeak( uint32_t id, const QString& formula, double time, double matchedMass, int mode )
{
    auto row = impl_->model_->rowCount();
    auto& model = *impl_->model_;

    impl_->model1busy_ = true;

    model.setRowCount( row + 1 );
    model.setData( model.index( row, impl::c_id ), id, Qt::EditRole );
    model.setData( model.index( row, impl::c_formula ), formula, Qt::EditRole );
    double exact_mass = adwidgets::MolTableHelper::monoIsotopicMass( formula, "" );
    model.setData( model.index( row, impl::c_matched_mass), matchedMass, Qt::EditRole );
    model.setData( model.index( row, impl::c_time), time * std::micro::den, Qt::EditRole );
    model.setData( model.index( row, impl::c_mode), mode );
    model.setData( model.index( row, impl::c_error), ( exact_mass - matchedMass ) * std::milli::den, Qt::EditRole );

    if ( auto item = model.item( row, impl::c_formula ) ) {
        item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
        model.setData( model.index( row, impl::c_formula ), Qt::Checked, Qt::CheckStateRole );
        item->setEditable( true );
    }
    if ( auto item = model.item( row, impl::c_matched_mass ) )
        item->setEditable( false );
    if ( auto item = model.item( row, impl::c_time ) )
        item->setEditable( false );
    if ( auto item = model.item( row, impl::c_error ) )
        item->setEditable( false );

    impl_->model1busy_ = false;
}

void
ScanLawDialog::updateMasses()
{
    auto scanlaw = impl_->scanlaw_;
    auto& model = *impl_->model_;

    ADDEBUG() << "scanlaw: " << scanlaw->kAcceleratorVoltage() << ", " << scanlaw->tDelay() << "s";

    for ( int row = 0; row < model.rowCount(); ++row ) {
        using adwidgets::MolTableHelper;
        double exactMass = MolTableHelper::monoIsotopicMass( model.index( row, impl::c_formula ).data( Qt::EditRole ).toString() );
        double time = model.index( row, impl::c_time ).data( Qt::EditRole ).toDouble() / std::micro::den;
        int mode = model.index( row, impl::c_mode ).data( Qt::EditRole ).toInt();
        double mass = scanlaw->getMass( time, mode );
        double error = exactMass - mass;
#if 0
        {
            auto formula = model.index( row, impl::c_formula ).data().toString().toStdString();
            ADDEBUG() << formula << " : " << model.index( row, impl::c_matched_mass ).data().toDouble()
                      << " --> " << mass << " " << error * 1000;
        }
#endif
        model.setData( model.index( row, impl::c_matched_mass), mass, Qt::EditRole );
        model.setData( model.index( row, impl::c_error), ( exactMass - mass ) * std::milli::den, Qt::EditRole );
    }
}

void
ScanLawDialog::updateObservers( double t0, double acclVolts )
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
ScanLawDialog::commit()
{
    if ( auto moltable = findChild< adwidgets::MolTableView * >( "peakTable" ) )
        moltable->resizeColumnsToContents();
    if ( auto objtable = findChild< adwidgets::MolTableView * >( "objText" ) )
        objtable->resizeColumnsToContents();
    return true;
}

bool
ScanLawDialog::read( int row, adcontrols::MSPeaks& peaks ) const
{
    const auto& m = *impl_->model_;
    // const size_t rowCount = m.rowCount();

    if ( 0 <= row && row < m.rowCount() ) {

        auto formula = m.index( row, impl::c_formula ).data( Qt::EditRole ).toString();
        double exactMass = adwidgets::MolTableHelper::monoIsotopicMass( formula );

        if ( exactMass > 0.7 ) {
            peaks << adcontrols::MSPeak( formula.toStdString()
                                         , m.index( row, impl::c_matched_mass ).data( Qt::EditRole ).toDouble()
                                         , m.index( row, impl::c_time ).data( Qt::EditRole ).toDouble() / std::micro::den // time
                                         , m.index( row, impl::c_mode ).data( Qt::EditRole ).toInt()   // mode
                                         , m.index( row, impl::c_id ).data( Qt::EditRole ).toInt() // spectrum index
                                         , exactMass );
            return true;
        }
    }

    return false;
}

void
ScanLawDialog::addObserver( const boost::uuids::uuid& uuid, const QString& objtext, double va, double t0, bool checked )
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
ScanLawDialog::checkedObservers() const
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
ScanLawDialog::estimateAcceleratorVoltage( const adcontrols::MSPeaks& peaks )
{
    double va(0), t0(0);

    if ( auto form = findChild< ScanLawForm * >() ) {
        using namespace infitof::Constants;
        impl_->scanlaw_ =
            std::make_shared< admtcontrols::ScanLaw >(
                form->acceleratorVoltage()
                , form->tDelay() / std::micro::den
                , form->L1()
                , FLIGHT_LENGTH_L2
                , FLIGHT_LENGTH_L3
                , FLIGHT_LENGTH_LG
                , FLIGHT_LENGTH_L4
                , FLIGHT_LENGTH_LT
                , FLIGHT_LENGTH_EXIT );
    }

    auto& law = *impl_->scanlaw_;

    ADDEBUG() << "############# ACCL-V ##################################";

    if ( peaks.size() == 1 ) {

        const adcontrols::MSPeak& pk = peaks[ 0 ];
        va = law.acceleratorVoltage( pk.exact_mass(), pk.time(), pk.mode(), 0.0 );
        t0 = 0.0;
        ADDEBUG() << "scanlaw: " << va << ", " << t0;
        law.setAcceleratorVoltage(va);
        law.setTDelay(t0);
        if ( auto form = findChild< ScanLawForm * >() ) {
            form->setAcceleratorVoltage(va);
            form->setTDelay( t0 * std::micro::den );
            updateMasses();
        }
        return true;

    } else if ( peaks.size() >= 2 ) {

        std::vector<double> x, y, coeffs;

        for ( auto& pk : peaks ) {
            x.push_back( std::sqrt( pk.exact_mass() ) * law.fLength( pk.mode() ) );
            y.push_back( pk.time() );
        }

        if ( adportable::polfit::fit( x.data(), y.data(), x.size(), 2, coeffs ) ) {

            t0 = coeffs[ 0 ];
            double t1 = adportable::polfit::estimate_y( coeffs, 1.0 ); // estimate tof for m/z = 1.0, 1mL
            va = adportable::TimeSquaredScanLaw::acceleratorVoltage( 1.0, t1, 1.0, t0 );

            ADDEBUG() << "scanlaw: " << va << ", " << t0;
            law.setAcceleratorVoltage(va);
            law.setTDelay(t0);

            if ( auto form = findChild< ScanLawForm * >() ) {
                form->setAcceleratorVoltage(va);
                form->setTDelay( t0 * std::micro::den );
                updateMasses();
            }
            return true;
        }
    }
    return false;
}

bool
ScanLawDialog::estimateL1( const std::vector< std::string >& /* formulae */, const adcontrols::MSPeaks& selectedPeaks )
{
    std::map< std::string, std::vector< adcontrols::MSPeak > > formula_peaks;

    for ( auto& pk: selectedPeaks )
        formula_peaks[ pk.formula() ].emplace_back( pk );

    std::map< std::string, std::vector< double > > formula_coeffs;
    std::vector< double > y0;
    /// ------------ compute linear regression for each formula -----
    for ( auto& formula_peak: formula_peaks ) {

        std::vector< double > x, y, coeffs;
        for ( auto& pk: formula_peak.second ) {
            x.emplace_back( pk.mode() * impl_->scanlaw_->orbital_length() );
            y.emplace_back( pk.time() );
        }
        if (( x.size() >= 2 ) && ( adportable::polfit::fit( x.data(), y.data(), x.size(), 2, coeffs ) ) ) {
            y0.emplace_back( coeffs[ 0 ] / coeffs[ 1 ] );
            formula_coeffs[ formula_peak.first ] = std::move( coeffs );
        }
    }

    double linear_length = std::accumulate( y0.begin(), y0.end(), 0.0 ) / y0.size();

    //using namespace admtcontrols::infitof;
    using namespace infitof::Constants;

    double L1 = linear_length - FLIGHT_LENGTH_EXIT - FLIGHT_LENGTH_L3;

    if ( auto form = findChild< ScanLawForm * >() ) {
        form->setL1( L1 );
        impl_->scanlaw_->setLength( 0, L1 );
        updateMasses();
        return true;
    }

    return false;
}

void
ScanLawDialog::handleCopyToClipboard()
{
	QString selected_text;

    // ScanLawDialog_archive x( *this );

	selected_text.append( QString::number( acceleratorVoltage(), 'e', 14 ) );
	selected_text.append( '\t' );
	selected_text.append( QString::number( tDelay(), 'e', 14 ) );

	QMimeData * md = new QMimeData();
	md->setText( selected_text );
#if 0
    std::wostringstream os;
    try {
        boost::archive::xml_woarchive ar ( os );
        ar & boost::serialization::make_nvp( "scanlaw", x );
        QString xml( QString::fromStdWString( os.str() ) );
        md->setData( QLatin1String( "application/scanlaw-xml" ), xml.toUtf8() );
    } catch ( std::exception& ex ) {
        (void)ex;
        //BOOST_THROW_EXCEPTION( ex );
    }
#endif
	QApplication::clipboard()->setMimeData( md );
}

void
ScanLawDialog::handlePaste()
{
    auto md = QApplication::clipboard()->mimeData();
    auto data = md->data( "application/scanlaw-xml" );
    if ( !data.isEmpty() ) {
        // QString utf8( QString::fromUtf8( data ) );
        // std::wistringstream is( utf8.toStdWString() );
        // boost::archive::xml_wiarchive ar( is );
        // try {
        //     ScanLawDialog_archive x;
        //     ar & boost::serialization::make_nvp( "scanlaw", x );
        //     x.load( *this );
        // } catch ( std::exception& ex ) {
        //     (void)ex;
        //     //BOOST_THROW_EXCEPTION( ex );
        // }
    }
}

void
ScanLawDialog::handlePeakTableMenu( const QPoint& pt )
{
    QMenu menu;

    std::set< int > selected_rows;
    std::map< QString, std::set< int > > formula_nlaps;

    if ( auto table = findChild< adwidgets::MolTableView * >( "peakTable" ) ) {
#if 0
        // select selected
        QModelIndexList list = table->selectionModel()->selectedIndexes();
        if ( list.size() > 0 ) {
            for ( auto index: list ) {
                selected_rows.insert( index.row() );
                auto formula = table->model()->index( index.row(), impl::c_formula ).data( Qt::EditRole ).toString();
                int nlaps = table->model()->index( index.row(), impl::c_mode ).data( Qt::EditRole ).toInt();
                formula_nlaps[ formula ].insert( nlaps );
            }
        }
#endif
        // select checked
        for ( int row = 0; row < table->model()->rowCount(); ++row ) {
            if ( table->model()->index( row, impl::c_formula ).data( Qt::CheckStateRole ) == Qt::Checked ) {
                selected_rows.insert( row );
                auto formula = table->model()->index( row, impl::c_formula ).data( Qt::EditRole ).toString();
                int nlaps = table->model()->index( row, impl::c_mode ).data( Qt::EditRole ).toInt();
                formula_nlaps[ formula ].insert( nlaps );
            }
        }
    }

    adcontrols::MSPeaks peaks1;
    for ( auto row: selected_rows )
        read( row, peaks1 );

    QString list1, list2;
    for ( auto item: formula_nlaps )
        list1 += list1.isEmpty() ? item.first : QString( ", %1" ).arg( item.first );

    std::vector< std::string > formulae;
    std::for_each( formula_nlaps.begin(), formula_nlaps.end(), [&](const std::pair< QString, std::set<int> >& item ){
            if ( item.second.size() >= 2 ) {
                list2 += list2.isEmpty() ? item.first : QString( ", %1" ).arg( item.first );
                formulae.emplace_back( item.first.toStdString() );
            }
        });

    QString title1( tr( "Calibrate accelerator voltage using %1" ).arg( list1 ) );
    QString title2( tr( "Calibrate dimension using %1" ).arg( list2 ) );

    menu.addAction( title1, [peaks1,this](){ estimateAcceleratorVoltage( peaks1 ); } );
    menu.addAction( title2, [formulae,peaks1,this](){ estimateL1( formulae, peaks1 ); } );

    // menu.addAction( tr( "Add peak" ), this, SLOT( handleAddPeak() ) );
    if ( auto table = findChild< adwidgets::MolTableView * >( "peakTable" ) )
        menu.exec( table->mapToGlobal( pt ) );
}

void
ScanLawDialog::handleAddPeak()
{
    auto row = impl_->model_->rowCount();
    auto& model = *impl_->model_;

    int mode = 0;
    double mass = adwidgets::MolTableHelper::monoIsotopicMass( "H" );
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
