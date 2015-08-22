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

#include "mscalibratesummarytable.hpp"
#include "delegatehelper.hpp"
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/mspeaks.hpp>
#include <adcontrols/mspeak.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adcontrols/computemass.hpp>
#include <adlog/logger.hpp>
#include <adutils/processeddata.hpp>
#include <adportable/array_wrapper.hpp>
#include <qtwrapper/qstring.hpp>
#include <qtwrapper/font.hpp>
#include <boost/format.hpp>
#include <boost/exception/all.hpp>
#include <boost/any.hpp>
#include <qapplication.h>
#include <qclipboard.h>
#include <QDoubleSpinBox>
#include <QKeyEvent>
#include <QtWidgets/QHeaderView>
#include <QPainter>
#include <QtPrintSupport/QPrinter>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QStyledItemDelegate>
#include <QMenu>
#include <QMessageBox>
#include <algorithm>
#include <functional>
#include <tuple>
#include <set>

namespace adwidgets {
    namespace detail {
        namespace mscalibratesummarytable {

            enum {
                c_time
                , c_formula
                , c_exact_mass
                , c_mass
                , c_mass_error_mDa
                , c_mass_calibrated
                , c_mass_error_calibrated_mDa
                , c_is_enable
                , c_delta_mass // d-mass from currnet focused peak
                , c_intensity
                , c_mode // analyzer mode id, a.k.a. reflectron|linear, or number of turns on InfiTOF
                , c_fcn  // segment id
                , c_index // keep ms index for 'sort', invisible
                , c_time_normalized
                , c_number_of_columns
                , c_flags_ // -- not in use -- out of order
            };

            class ItemDelegate : public QStyledItemDelegate {
            public:
                void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
                void setEditorData(QWidget *editor, const QModelIndex &index) const override;
                void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
                bool editorEvent( QEvent * event, QAbstractItemModel *, const QStyleOptionViewItem&, const QModelIndex& ) override;
                std::function< void( const QModelIndex& ) > valueChanged_;
            };

            struct print_text {
                static void to_print_text( std::string&, const QModelIndex& index );
            };
        }
    }
}

using namespace adwidgets;
using namespace adwidgets::detail::mscalibratesummarytable;

MSCalibrateSummaryTable::~MSCalibrateSummaryTable()
{
}

MSCalibrateSummaryTable::MSCalibrateSummaryTable(QWidget *parent) : QTableView(parent)
                                                                  , inProgress_( false )
                                                                  , pModel_( new QStandardItemModel )
                                                                  , pCalibResult_( new adcontrols::MSCalibrateResult )
                                                                  , pCalibrantSpectrum_( new adcontrols::MassSpectrum )
{
    setModel( pModel_.get() );

    auto delegate = new adwidgets::detail::mscalibratesummarytable::ItemDelegate;

    delegate->valueChanged_ = [=] ( const QModelIndex& index ){ handleValueChanged( index ); };

    setItemDelegate( delegate );

    setContextMenuPolicy( Qt::CustomContextMenu );
    setSortingEnabled( true );
    verticalHeader()->setDefaultSectionSize( 18 );
    QFont font;
    setFont( qtwrapper::font::setFont( font, qtwrapper::fontSizeSmall, qtwrapper::fontTableBody ) );
    connect( this, &QTableView::customContextMenuRequested, this, &MSCalibrateSummaryTable::showContextMenu);
}

void
MSCalibrateSummaryTable::OnCreate( const adportable::Configuration& )
{
}

void
MSCalibrateSummaryTable::OnInitialUpdate()
{
    QStandardItemModel& model = *pModel_;
    QTableView& tableView = *this;

    QStandardItem * rootNode = model.invisibleRootItem();
    tableView.setRowHeight( 0, 7 );
    rootNode->setColumnCount( c_number_of_columns );

    model.setHeaderData( c_time, Qt::Horizontal, QObject::tr( "time(us)" ) );
    model.setHeaderData( c_time_normalized, Qt::Horizontal, QObject::tr( "time(us/m)" ) );
    model.setHeaderData( c_mass, Qt::Horizontal, QObject::tr( "m/z" ) );
    model.setHeaderData( c_mass_calibrated, Qt::Horizontal, QObject::tr( "m/z(calibrated)" ) );
    model.setHeaderData( c_mode, Qt::Horizontal, QObject::tr( "#turns" ) );
    model.setHeaderData( c_intensity, Qt::Horizontal, QObject::tr( "Intensity" ) );
    model.setHeaderData( c_formula, Qt::Horizontal, QObject::tr( "formula" ) );
    model.setHeaderData( c_exact_mass, Qt::Horizontal, QObject::tr( "m/z(exact)" ) );
    model.setHeaderData( c_mass_error_mDa, Qt::Horizontal, QObject::tr( "error(mDa)" ) );
    model.setHeaderData( c_mass_error_calibrated_mDa, Qt::Horizontal, QObject::tr( "error(mDa) calibrated" ) );
    model.setHeaderData( c_is_enable, Qt::Horizontal, QObject::tr( "enable" ) );
	model.setHeaderData( c_delta_mass, Qt::Horizontal, QObject::tr( "delta m/z" ) );
    model.setHeaderData( c_fcn, Qt::Horizontal, QObject::tr( "fcn" ) );
    model.setHeaderData( c_index, Qt::Horizontal, QObject::tr( "index" ) );
    this->setColumnHidden( c_index, true ); // internal index points peak on MassSpectrum
}

void
MSCalibrateSummaryTable::onUpdate( boost::any& )
{
}

void
MSCalibrateSummaryTable::OnFinalClose()
{
}

bool
MSCalibrateSummaryTable::getContents( boost::any& any ) const
{
	// colored spectrum for GUI display ( app. no longer use this though...)
    if ( adutils::ProcessedData::is_type< adutils::MassSpectrumPtr >( any ) && pCalibrantSpectrum_ ) {
        adutils::MassSpectrumPtr ptr = boost::any_cast< adutils::MassSpectrumPtr >( any );
        *ptr = *pCalibrantSpectrum_; // deep copy
        return true;
    }

	// editted assign table
    if ( adutils::ProcessedData::is_type< std::shared_ptr< adcontrols::MSAssignedMasses > >( any ) ) {
        std::shared_ptr< adcontrols::MSAssignedMasses > ptr
            = boost::any_cast< std::shared_ptr< adcontrols::MSAssignedMasses > >( any );
        getAssignedMasses( *ptr );
        return true;
    }
    return false;
}

void
MSCalibrateSummaryTable::getAssignedMasses( adcontrols::MSAssignedMasses& t ) const
{
    QStandardItemModel& model = *pModel_;

    for ( int row = 0; row < model.rowCount(); ++row ) {
        
        QString formula = model.data( model.index( row, c_formula ) ).toString();
        std::wstring wformula = qtwrapper::wstring( formula );
        if ( ! formula.isEmpty()  ) {
            bool enable = model.index( row, c_is_enable ).data( Qt::EditRole ).toBool();
            double time = model.index( row, c_time ).data( Qt::EditRole ).toDouble();

            double exact_mass = model.index( row, c_exact_mass ).data( Qt::EditRole ).toDouble();
            double mass = model.index( row, c_mass ).data( Qt::EditRole ).toDouble();
            uint32_t mode = model.index( row, c_mode ).data( Qt::EditRole ).toInt();
            uint32_t fcn = model.index( row, c_fcn ).data( Qt::EditRole ).toInt();
            uint32_t index = model.index( row, c_index ).data( Qt::EditRole ).toInt();

            adcontrols::MSAssignedMass assigned( -1
                                                 , fcn
                                                 , index
                                                 , wformula, exact_mass, time, mass, enable, 0 /* flag */, mode );
            t << assigned;
        }
    }    
}


bool
MSCalibrateSummaryTable::setContents( boost::any& )
{
    return false;
}


void
MSCalibrateSummaryTable::getLifeCycle( adplugin::LifeCycle *& p )
{
    p = static_cast< adplugin::LifeCycle *>(this);
}

bool
MSCalibrateSummaryTable::setAssignedData( int row, int fcn, int idx, const adcontrols::MSAssignedMasses& assigned )
{
    using namespace adcontrols::metric;

    QStandardItemModel& model = *pModel_;

    auto it = std::find_if( assigned.begin(), assigned.end(), [=]( const adcontrols::MSAssignedMass& a ){
            return fcn == int(a.idMassSpectrum()) && idx == int(a.idPeak()); 
        });
    
    if ( it == assigned.end() )
        return false;
	
	double normalized_time = 0; // ( it->time() - t0 ) / pCalibrantSpectrum_->scanLaw().fLength( it->mode() );

    const adcontrols::MSCalibration& calib = pCalibResult_->calibration();
    double mass( 0 );
    if ( auto scanLaw = pCalibrantSpectrum_->scanLaw() ) {
        adcontrols::ComputeMass< adcontrols::ScanLaw > mass_calculator( *pCalibrantSpectrum_->scanLaw(), calib );
        mass = mass_calculator( it->time(), it->mode() );
    }
    else {
        mass = adcontrols::detail::compute_mass< adcontrols::MSCalibration::TIMESQUARED >::compute( it->time(), calib );
    }

	if ( calib.algorithm() == adcontrols::MSCalibration::MULTITURN_NORMALIZED ) {
		double t0 = scale_to_base( calib.compute( calib.t0_coeffs(), std::sqrt( mass ) ), calib.time_prefix() );
		double L = pCalibrantSpectrum_->scanLaw()->fLength( it->mode() );
		normalized_time = ( it->time() - t0 ) / L;
	} else {
        if ( auto scanLaw = pCalibrantSpectrum_->scanLaw() )
            normalized_time = (it->time()) / pCalibrantSpectrum_->scanLaw()->fLength( it->mode() );
        else
            normalized_time = it->time();
	}

    model.setData( model.index( row, c_time_normalized ), normalized_time );
    model.setData( model.index( row, c_formula ), qtwrapper::qstring::copy( it->formula() ) );
    model.setData( model.index( row, c_exact_mass ), it->exactMass() );
    model.setData( model.index( row, c_mass_calibrated ), mass );

    double org = it->mass();
    double err = org - it->exactMass();
    model.setData( model.index( row, c_mass_error_mDa ), err * 1000 ); // mDa

    double mz = model.index( row, c_mass_calibrated ).data( Qt::EditRole ).toDouble();
    if ( mz > 1.0 ) {
        model.setData( model.index( row, c_mass_error_calibrated_mDa ), ( mz - it->exactMass() ) * 1000 ); // mDa
        //model.setData( model.index( row, c_mass_error2_calibrated_mDa ), ( mass2 - it->exactMass() ) * 1000 ); // mDa
    }

    model.setData( model.index( row, c_mode ), it->mode() );

    model.setData( model.index( row, c_is_enable ), it->enable() );

    if ( QStandardItem * chk = model.itemFromIndex( model.index( row, c_is_enable ) ) ) {
        chk->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | chk->flags() );
        model.setData( model.index( row, c_is_enable ), it->enable() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
    }

    setRowHidden( row, false );

    return true;
}

bool
MSCalibrateSummaryTable::createModelData( const std::vector< std::pair< int, int > >& indecies )
{
    QStandardItemModel& model = *pModel_;

    const adcontrols::MSCalibration& calib = pCalibResult_->calibration();

    model.removeRows( 0, model.rowCount() );
    model.insertRows( 0, static_cast<int>(indecies.size()) );

    adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( *pCalibrantSpectrum_ );

    int row = 0;
    for ( auto idx : indecies ) {

        adcontrols::MassSpectrum& ms = segments[ idx.first ];

        model.setData( model.index( row, c_fcn ),   idx.first );
        model.setData( model.index( row, c_index ), idx.second );
        
        model.setData( model.index( row, c_mode ),  ms.getMSProperty().getSamplingInfo().mode ); // nTurns
        model.setData( model.index( row, c_mass ),  ms.getMass( idx.second ) );
        model.setData( model.index( row, c_time ),  ms.getTime( idx.second ) );

        // this will be deprecated.
        model.setData( model.index( row, c_time_normalized ), ms.getNormalizedTime( idx.second ) );

        model.setData( model.index( row, c_intensity ), ms.getIntensity( idx.second ) );

        model.setData( model.index( row, c_mass_calibrated ), calib.compute_mass( ms.getTime( idx.second ) ) ); // ms.getNormalizedTime( idx.second ) ) );

        try {
            setAssignedData( row, idx.first, idx.second, pCalibResult_->assignedMasses() );
        } catch ( boost::exception& e ) {
			QMessageBox::warning( 0, "Exception", QString::fromStdString( boost::diagnostic_information(e) ) );
        }
		setEditable( row );

        ++row;
    }
    return true;
}

void
MSCalibrateSummaryTable::setEditable( int row, bool )
{
	QStandardItemModel& model = *pModel_;

    model.item( row, c_fcn )->setEditable( false );
    model.item( row, c_index )->setEditable( false );
    model.item( row, c_time )->setEditable( false );
    model.item( row, c_time_normalized )->setEditable( false );
    model.item( row, c_mass )->setEditable( false );
    model.item( row, c_mass_calibrated )->setEditable( false );
    model.item( row, c_intensity )->setEditable( false );
    //model.item( row, c_delta_mass )->setEditable( false );
}

bool
MSCalibrateSummaryTable::modifyModelData( const std::vector< std::pair< int, int > >& indecies )
{
    QStandardItemModel& model = *pModel_;

	const adcontrols::MSCalibration& calib = this->pCalibResult_->calibration();
    adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( *pCalibrantSpectrum_ );

    for ( int row = 0; row < model.rowCount(); ++row ) {

        int fcn = model.index( row, c_fcn ).data( Qt::EditRole ).toInt();
        int idx = model.index( row, c_index ).data( Qt::EditRole ).toInt();

        auto it = std::find_if( indecies.begin(), indecies.end(), [=]( const std::pair< int, int >& a ){
                return a.first == fcn &&  a.second == idx;});
        
        if ( it == indecies.end() )
            return false;

        adcontrols::MassSpectrum& ms = segments[ it->first ];

        model.setData( model.index( row, c_mode ),  ms.getMSProperty().getSamplingInfo().mode ); // nTurns
        model.setData( model.index( row, c_mass ),  ms.getMass( it->second ) );

        model.setData( model.index( row, c_time ),  ms.getTime( it->second ) );
        model.setData( model.index( row, c_time_normalized ),  ms.getNormalizedTime( it->second ) );
        model.setData( model.index( row, c_intensity ), ms.getIntensity( it->second ) );

        model.setData( model.index( row, c_mass_calibrated ), calib.compute_mass( ms.getTime( it->second ) ) );
        // ms.getNormalizedTime( it->second ) ) );
        try {
            setAssignedData( row, it->first, it->second, pCalibResult_->assignedMasses() );
        } catch ( boost::exception& e ) {
			QMessageBox::warning( 0, "Exception", QString::fromStdString( boost::diagnostic_information(e) ) );
        }
    }
    return true;
}

void
MSCalibrateSummaryTable::setData( const adcontrols::MSCalibrateResult& res, const adcontrols::MassSpectrum& ms )
{
    QStandardItemModel& model = *pModel_;

    if ( ! ms.isCentroid() )
        return;

    std::vector< std::pair< int, int > > indecies;
    *pCalibrantSpectrum_ = ms;
    *pCalibResult_ = res;

	double threshold = res.threshold();
    
    adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( *pCalibrantSpectrum_ );
    for ( int fcn = 0; fcn < signed(segments.size()); ++fcn ) {
		adcontrols::MassSpectrum& fms = segments[ fcn ];
		for ( int idx = 0; idx < signed(fms.size()); ++idx ) {
			if ( fms.getIntensity( idx ) > threshold )
				indecies.push_back( std::make_pair( fcn, idx ) );
		}
	}
    
    if ( ! ( ( model.rowCount() == int(indecies.size()) ) && modifyModelData( indecies ) ) )
        createModelData( indecies );
}

void
MSCalibrateSummaryTable::showContextMenu( const QPoint& pt )
{
    QMenu menu;

    menu.addAction( tr( "Hide" ), this, SLOT( hideRows() ) );
    menu.addAction( tr( "Show" ), this, SLOT( showRows() ) );
    
    menu.addAction( tr( "Re-calc polynomials" ), this, SLOT( recalicPolynomials() ) );
    menu.addAction( tr( "Assign mass on spectrum" ), this, SLOT( assignMassOnSpectrum() ) );
    menu.addAction( tr( "Apply calibration to the dataset (change all spectra in the dataset)" ), this, SLOT( applyCalibrationToDataset() ) );
    menu.addAction( tr( "Save as default calibration" ), this, SLOT( saveAsDefaultCalibration() ) );
    menu.addAction( tr( "Copy summary to clipboard" ), this, SLOT( copySummaryToClipboard() ) );
    menu.addAction( tr( "Add to peak table" ), this, SLOT( addSelectionToPeakTable() ) );

    menu.exec( this->mapToGlobal( pt ) );
}

void
MSCalibrateSummaryTable::handle_zoomed( const QRectF& rc )
{
	(void)rc;
}

void
MSCalibrateSummaryTable::handle_selected( const QRectF& rc )
{
    double y0 = 0;
    int row_highest = -1;
	for ( int row = 0; row < pModel_->rowCount(); ++row ) {
        QModelIndex index = pModel_->index( row, c_mass );
        double mass = index.data( Qt::EditRole ).toDouble();
        if ( rc.left() < mass && mass < rc.right() ) {
            double y = pModel_->index( row, c_intensity ).data( Qt::EditRole ).toDouble();
            if ( y > y0 ) {
                y0 = y;
                row_highest = row;
            }
        }
    }
    if ( row_highest >= 0 ) {
		setCurrentIndex( pModel_->index( row_highest, c_formula ) );
		scrollTo( pModel_->index( row_highest, c_formula ) );
    }
}

void
MSCalibrateSummaryTable::handleEraseFormula()
{
}

void
MSCalibrateSummaryTable::formulaChanged( const QModelIndex& index )
{
	QStandardItemModel& model = *pModel_;

    if ( index.column() == c_formula ) {

        std::wstring formula = qtwrapper::wstring( index.data( Qt::EditRole ).toString() );
        std::wstring adduct_lose;
        if ( ! formula.empty() ) {
            std::wstring::size_type pos = formula.find_first_of( L"+-" );
            int sign = 1;
            if ( pos != std::wstring::npos ) {
                sign = formula.at( pos ) == L'+' ? 1 : -1;
                adduct_lose = formula.substr( pos + 1 );
                formula = formula.substr( 0, pos );
            }
            
            adcontrols::ChemicalFormula cformula;
            double exactMass = cformula.getMonoIsotopicMass( formula );
            if ( ! adduct_lose.empty() ) {
                double a = cformula.getMonoIsotopicMass( adduct_lose );
                exactMass += a * sign;
            }

            // update exact mass
            model.setData( model.index( index.row(), c_exact_mass ), exactMass );
            double mass = model.index( index.row(), c_mass ).data( Qt::EditRole ).toDouble();
            model.setData( model.index( index.row(), c_mass_error_mDa ), mass - exactMass );            
            double calib_mass = model.index( index.row(), c_mass_error_calibrated_mDa ).data( Qt::EditRole ).toDouble();
            if ( calib_mass > 1.0 ) {
                model.setData( model.index( index.row(), c_mass_error_calibrated_mDa ), calib_mass - exactMass );
            }

            do {
                model.setData( model.index( index.row(), c_is_enable ), true ); // set enabled by default
                QStandardItem * chk = model.itemFromIndex( model.index( index.row(), c_is_enable ) );
                if ( chk ) {
                    chk->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | chk->flags() );
                    chk->setEditable( true );
                    model.setData( model.index( index.row(), c_is_enable ), Qt::Unchecked, Qt::CheckStateRole );
                }
            } while ( 0 );
        }
    }
}

void
MSCalibrateSummaryTable::handleValueChanged( const QModelIndex& index )
{
    if ( inProgress_ )
        return;

    if ( index.column() == c_formula ) {
        formulaChanged( index );
        emit valueChanged();
	} else if ( index.column() == c_is_enable ) {
        emit valueChanged();
	} else if ( index.column() == c_mode ) {
		emit valueChanged();
	}
}

void
MSCalibrateSummaryTable::currentChanged( const QModelIndex& index, const QModelIndex& prev )
{
    QStandardItemModel& model = *pModel_;
    (void)prev;
    scrollTo( index, QAbstractItemView::EnsureVisible );
	int row = index.row();
    int idx = model.index( row, c_index ).data( Qt::EditRole ).toInt();
    int fcn = model.index( row, c_fcn ).data( Qt::EditRole ).toInt();

    emit currentChanged( idx, fcn );

    double mass = model.index( row, c_mass ).data( Qt::EditRole ).toDouble();
    for ( int r = 0; r < model.rowCount(); ++r ) {
        double d = std::abs( model.index( r, c_mass ).data( Qt::EditRole ).toDouble() - mass );
        model.setData( model.index( r, c_delta_mass ), int( d + 0.7 ) );
    }
}

void
MSCalibrateSummaryTable::recalcPolynomials()
{
    emit valueChanged();
}

void
MSCalibrateSummaryTable::assignMassOnSpectrum()
{
    emit on_reassign_mass_requested();      // change source mass spectrum m/z array (both profile and centroid)
}

void
MSCalibrateSummaryTable::applyCalibrationToDataset()
{
    emit on_apply_calibration_to_dataset(); // change whole calibration for current dataset    
}

void
MSCalibrateSummaryTable::saveAsDefaultCalibration()
{
    emit on_apply_calibration_to_default(); // save calibration as system default    
}

void
MSCalibrateSummaryTable::handleCopyToClipboard()
{
    QStandardItemModel& model = *pModel_;
    QModelIndexList list = selectionModel()->selectedIndexes();

    qSort( list );
    if ( list.size() < 1 )
        return;

    QString copy_table;
    QModelIndex prev = list.first();
	int i = 0;
    for ( auto idx: list ) {
		if ( i++ > 0 )
			copy_table.append( prev.row() == idx.row() ? '\t' : '\n' );
        if ( idx.column() == c_time || idx.column() == c_time_normalized )
            copy_table.append( QString::number( adcontrols::metric::scale_to_micro( model.data( idx ).toDouble() ), 'g', 14 ) );
        else
            copy_table.append( model.data( idx ).toString() );
        prev = idx;
    }
    QApplication::clipboard()->setText( copy_table );
}

void
MSCalibrateSummaryTable::addSelectionToPeakTable()
{
    QModelIndexList list = selectionModel()->selectedIndexes();
    if ( list.size() < 1 )
        return;

    std::set< int > rows;

    for ( auto index: list )
        rows.insert( index.row() );

    QStandardItemModel& model = *pModel_;
    adcontrols::MSPeaks peaks;
    for ( int row: rows ) {
        double time = model.index( row, c_time ).data( Qt::EditRole ).toDouble();
        double mass = model.index( row, c_mass ).data( Qt::EditRole ).toDouble();
        std::string formula = model.index( row, c_formula ).data().toString().toStdString();
        if ( !formula.empty() )
            mass = model.index( row, c_exact_mass ).data( Qt::EditRole ).toDouble();
        int mode = model.index( row, c_mode ).data().toInt();

        const adcontrols::ScanLaw* law = pCalibrantSpectrum_->scanLaw();
        adcontrols::MSPeak peak( time, mass, mode, law->fLength( mode ) );
        peak.formula( formula );

        //peak.spectrumId( pCalibrantSpectrum_->uuid() );
        peaks << peak;
    }
    // std::string device;
    // adportable::serializer< adcontrols::MSPeaks >::serialize( peaks, device );
    // QByteArray d( device.data(), device.size() );
	// emit on_add_selection_to_peak_table( QString::fromStdWString(adcontrols::MSPeaks::dataClass()), d );
    emit on_add_selection_to_peak_table( peaks );
}

void
MSCalibrateSummaryTable::copySummaryToClipboard()
{
    QStandardItemModel& model = *pModel_;

    QString text;

    if ( pCalibResult_ ) {
        const adcontrols::MSCalibration& calib = pCalibResult_->calibration();
        text.append( "Calibration date:\t" );
        text.append( calib.date().c_str() );
        text.append( "\tid\t" );
        text.append( qtwrapper::qstring( calib.calibId() ) );
        text.append( '\n' );
        text.append( "SQRT( m/z ) = " );
        for ( size_t i = 0; i < calib.coeffs().size(); ++i ) {
            QString term = i ? ( boost::format( "%c*X^%d" ) % char( 'a' + i ) % ( i ) ).str().c_str() : "a";
            text.append( '\t' );
            text.append( term );
        }
        text.append( '\n' );
        for ( size_t i = 0; i < calib.coeffs().size(); ++i ) {
            QString term = ( boost::format( "%.14lf" ) % calib.coeffs()[ i ] ).str().c_str();
            text.append( '\t' );
            text.append( term );
        }
        text.append( '\n' );
    }

    for ( int col = 0; col < model.columnCount(); ++col ) {
        text.append( model.headerData( col, Qt::Horizontal ).toString() );
        text.append( '\t' );
    }
    text.append( '\n' );

    for ( int row = 0; row < model.rowCount(); ++row ) {
        for ( int col = 0; col < model.columnCount(); ++col ) {
            if ( col == c_time || col == c_time_normalized )
                text.append( QString("%1").arg( adcontrols::metric::scale_to_micro( model.data( model.index( row, col ) ).toDouble() ) ) );
            else
                text.append( model.data( model.index( row, col ) ).toString() );
            text.append( '\t' );
        }
        text.append( '\n' );
    }
    QApplication::clipboard()->setText( text );
}

void
MSCalibrateSummaryTable::handlePasteFromClipboard()
{
    QString pasted = QApplication::clipboard()->text();
	QStringList texts = pasted.split( "\n" );

    QModelIndexList list = selectionModel()->selectedIndexes();
    qSort( list );
    if ( list.size() < 1 )
        return;
    QModelIndex index = list.at( 0 );
    if ( index.column() == c_formula ) {
        int row = index.row();
        for ( auto text: texts ) {
            pModel_->setData( index, text );
            formulaChanged( index );
			index = pModel_->index( ++row, c_formula );
        }
        emit valueChanged();
    }
}

void
MSCalibrateSummaryTable::keyPressEvent( QKeyEvent * event )
{
    if ( event->matches( QKeySequence::Copy ) ) {
        handleCopyToClipboard();
    } else if ( event->matches( QKeySequence::Paste ) ) {
        handlePasteFromClipboard();
    } else {
        QTableView::keyPressEvent( event );
    }
}

void
MSCalibrateSummaryTable::hideRows()
{
    QStandardItemModel& model = *pModel_;
    for ( int row = 0; row < model.rowCount(); ++row ) {
        bool hide = model.data( model.index( row, c_formula ) ).toString().isEmpty();
        setRowHidden( row, hide );
    }

}

void
MSCalibrateSummaryTable::showRows()
{
    QStandardItemModel& model = *pModel_;    
    for ( int row = 0; row < model.rowCount(); ++row ) {
        if ( isRowHidden( row ) )
            setRowHidden( row, false );
    }
}

namespace adwidgets {

    class grid_render {
        const QRectF& rect_;
        double bottom_;
		QRectF boundingRect_;
        QRectF rc_;
    public:
        std::vector< std::pair< double, double > > tab_stops_;

        grid_render( const QRectF& rect ) : rect_( rect ), bottom_( 0 ), rc_( rect ) {
        }

        void add_tab( double width ) {
            if ( tab_stops_.empty() )
                tab_stops_.push_back( std::make_pair( rect_.left(), width ) );
            else
                tab_stops_.push_back( std::make_pair( tab_stops_.back().second + tab_stops_.back().first, width ) );
        }

        void operator()( QPainter& painter, int col, const QString& text ) {
            if ( tab_stops_[col].second == 0 ) // no width
                return;
            rc_.setRect( tab_stops_[ col ].first, rc_.y(), tab_stops_[col].second, rect_.bottom() - rc_.y() );
            painter.drawText( rc_, Qt::TextWordWrap, text, &boundingRect_ );
            bottom_ = std::max<double>( boundingRect_.bottom(), bottom_ );
        }

        bool new_line( QPainter& painter ) {
            rc_.moveTo( rect_.x(), bottom_ + rect_.height() * 0.005);
            bottom_ = 0;
            if ( rc_.y() > rect_.bottom() ) {
                draw_horizontal_line( painter ); // footer separator
                return true;
            }
            return false;
        }

        void new_page( QPainter& painter ) {
            rc_.setRect( tab_stops_[ 0 ].first, rect_.top(), tab_stops_[ 0 ].second, rect_.height() );
            draw_horizontal_line( painter ); // header separator
        }

        void draw_horizontal_line( QPainter& painter ) {
            painter.drawLine( rect_.left(), rc_.top(), rect_.right(), rc_.top() );
        }
    };
}

void
MSCalibrateSummaryTable::handlePrint( QPrinter& printer, QPainter& painter )
{
    const QStandardItemModel& model = *pModel_;
    printer.newPage();
	const QRect rect( printer.pageRect().x() + printer.pageRect().width() * 0.05
                      , printer.pageRect().y() + printer.pageRect().height() * 0.05
                      , printer.pageRect().width() * 0.9, printer.pageRect().height() * 0.8 );
    
    const int rows = model.rowCount();
    const int cols = model.columnCount();

    grid_render render( rect );

    for ( int col = 0; col < cols; ++col ) {
        double width = 0;
        switch( col ) {
        case c_time:                      width = rect.width() / 180 * 16; break;
        case c_time_normalized:           width = rect.width() / 180 * 16; break;
        case c_formula:                   width = rect.width() / 180 * 28; break;
        case c_exact_mass:                width = rect.width() / 180 * 18; break;
        case c_mass:                      width = rect.width() / 180 * 18; break;
        case c_intensity:                 width = rect.width() / 180 * 12; break;
        case c_mass_error_mDa:            width = rect.width() / 180 * 14; break;
        case c_mass_calibrated:           width = rect.width() / 180 * 18; break;
        case c_mass_error_calibrated_mDa: width = rect.width() / 180 * 14; break;
        case c_is_enable:                 width = rect.width() / 180 * 8; break;
        case c_mode:                      width = rect.width() / 180 * 8; break;
        case c_fcn:                       width = rect.width() / 180 * 6; break;
        case c_delta_mass:                width = 0; break;
        case c_index:                     width = 0; break;
        }
        render.add_tab( width );
    }

    render.new_page( painter );
    for ( int col = 0; col < cols; ++col )
        render( painter, col, model.headerData( col, Qt::Horizontal ).toString() );
    render.new_line( painter );
    render.draw_horizontal_line( painter );

    std::string text;
    for ( int row = 0; row < rows; ++row ) {
        for ( int col = 0; col < cols; ++col ) {
            print_text::to_print_text( text, model.index( row, col ) );
            render( painter, col, text.c_str() );
        }
        if ( render.new_line( painter ) ) {
            printer.newPage();
            render.new_page( painter );
            for ( int col = 0; col < cols; ++col )
                render( painter, col, model.headerData( col, Qt::Horizontal ).toString() );
            render.new_line( painter );
            render.draw_horizontal_line( painter );  
        }
    }
    render.draw_horizontal_line( painter );
}

///////////////////////////////////////

void
ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    using namespace adcontrols::metric;

    QStyleOptionViewItem op( option );
    initStyleOption( &op, index );
    auto align = Qt::AlignRight | Qt::AlignVCenter;
    op.displayAlignment = align;

    switch( index.column() ) {
    case c_time:
        painter->drawText( option.rect, align, ( boost::format("%.5lf") % scale_to_micro( index.data( Qt::EditRole ).toDouble() ) ).str().c_str() );
        break;
    case c_time_normalized:
        painter->drawText( option.rect, align, ( boost::format("%.5lf") % scale_to_micro( index.data( Qt::EditRole ).toDouble() ) ).str().c_str() );
        break;
    case c_exact_mass:
		if ( ! index.model()->data( index.model()->index( index.row(), c_formula ), Qt::EditRole ).toString().isEmpty() )
			painter->drawText( option.rect, align, ( boost::format("%.7lf") % index.data( Qt::EditRole ).toDouble() ).str().c_str() );
		break;
    case c_mass:
	case c_mass_calibrated:
        painter->drawText( option.rect, align, ( boost::format("%.7lf") % index.data( Qt::EditRole ).toDouble() ).str().c_str() );
        break;
    case c_intensity:
        painter->drawText( option.rect, align, ( boost::format("%.1lf") % index.data( Qt::EditRole ).toDouble() ).str().c_str() );
        break;
    case c_mass_error_mDa:
    case c_mass_error_calibrated_mDa:
		if ( ! index.model()->data( index.model()->index( index.row(), c_formula ), Qt::EditRole ).toString().isEmpty() )
			painter->drawText( option.rect, align, ( boost::format("%.3lf") % index.data( Qt::EditRole ).toDouble() ).str().c_str() );
		break;
    case c_formula:
        do {
            std::wstring formula = adcontrols::ChemicalFormula::formatFormulae( index.data().toString().toStdWString() );
            DelegateHelper::render_html2( painter, op, QString::fromStdWString( formula ) );
        } while ( 0 );
        break;
    case c_delta_mass:
    case c_is_enable:
    case c_flags_:
    case c_mode:
    case c_fcn:
    default:
        QStyledItemDelegate::paint( painter, op, index );
    }
}

void
ItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    using namespace adcontrols::metric;

    if ( index.column() == c_time
         || index.column() == c_time_normalized ) {
        double seconds = index.data().toDouble();
        static_cast< QDoubleSpinBox * >( editor )->setValue( scale_to_micro( seconds ) );
    } else {
        QStyledItemDelegate::setEditorData( editor, index );
    }
}

void
ItemDelegate::setModelData( QWidget *editor
                                  , QAbstractItemModel *model
                                  , const QModelIndex &index) const
{
    using namespace adcontrols::metric;

    if ( index.column() == c_time || index.column() == c_time_normalized ) {
        QDoubleSpinBox * spin = static_cast< QDoubleSpinBox *>(editor);
        double microseconds = spin->value();
        model->setData( index, scale_to_base( microseconds, micro ) );
    } else {
        QStyledItemDelegate::setModelData( editor, model, index );
    }
    if ( valueChanged_ )
        valueChanged_( index );
}


bool
ItemDelegate::editorEvent( QEvent * event
                            , QAbstractItemModel * model
                            , const QStyleOptionViewItem& option
                            , const QModelIndex& index )
{
    bool res = QStyledItemDelegate::editorEvent( event, model, option, index );
    if ( event->type() == QEvent::MouseButtonRelease && model->flags(index) & Qt::ItemIsUserCheckable ) {
        QVariant st = index.data( Qt::CheckStateRole );
        if ( index.data( Qt::EditRole ).type() == QVariant::Bool ) {
            model->setData( index, ( st == Qt::Checked ) ? true : false );
            if ( valueChanged_ )
                valueChanged_( index );
        }
    }
    return res;
}


void
print_text::to_print_text( std::string& text, const QModelIndex &index )
{
    using namespace adcontrols::metric;

    text.clear();
    switch( index.column() ) {
    case c_time:
    case c_time_normalized:
        text = ( boost::format("%.7lf") % scale_to_micro( index.data( Qt::EditRole ).toDouble() ) ).str();
        break;
    case c_exact_mass:
		if ( ! index.model()->data( index.model()->index( index.row(), c_formula ), Qt::EditRole ).toString().isEmpty() )
			text = ( boost::format("%.7lf") % index.data( Qt::EditRole ).toDouble() ).str();
        break;
    case c_mass:
	case c_mass_calibrated:
        text = ( boost::format("%.7lf") % index.data( Qt::EditRole ).toDouble() ).str();
        break;
    case c_intensity:
        text = ( boost::format("%.1lf") % index.data( Qt::EditRole ).toDouble() ).str();
        break;
    case c_mass_error_mDa:
    case c_mass_error_calibrated_mDa:
		if ( ! index.model()->data( index.model()->index( index.row(), c_formula ), Qt::EditRole ).toString().isEmpty() )
			text = ( boost::format("%.3lf") % index.data( Qt::EditRole ).toDouble() ).str();
		break;
    case c_formula:
    case c_is_enable:
    case c_flags_:
    case c_mode:
    case c_fcn:
    default:
        text = index.data( Qt::DisplayRole ).toString().toStdString();
    }
}

