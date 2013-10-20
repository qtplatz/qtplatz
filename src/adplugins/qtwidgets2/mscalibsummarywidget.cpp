// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "mscalibsummarywidget.hpp"
#include "mscalibsummarydelegate.hpp"
#include <QStandardItemModel>
#include <QStandardItem>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adutils/processeddata.hpp>
#include <adportable/array_wrapper.hpp>
#include <qtwrapper/qstring.hpp>
#include <boost/format.hpp>
#include <QMenu>
#include <boost/any.hpp>
#include <qapplication.h>
#include <qclipboard.h>
#include <QKeyEvent>
#include <QtWidgets/QHeaderView>
#include <QPainter>
#include <algorithm>
#include <tuple>

using namespace qtwidgets2;

MSCalibSummaryWidget::~MSCalibSummaryWidget()
{
}

MSCalibSummaryWidget::MSCalibSummaryWidget(QWidget *parent) : QTableView(parent)
                                                            , pModel_( new QStandardItemModel )
                                                            , pDelegate_( new MSCalibSummaryDelegate ) 
                                                            , pCalibrantSpectrum_( new adcontrols::MassSpectrum )
                                                            , pCalibResult_( new adcontrols::MSCalibrateResult )
                                                            , inProgress_( false )
{
    this->setModel( pModel_.get() );
    this->setItemDelegate( pDelegate_.get() );
    this->setContextMenuPolicy( Qt::CustomContextMenu );
    this->setSortingEnabled( true );
    this->verticalHeader()->setDefaultSectionSize( 18 );
    QFont font;
    font.setFamily( "Consolas" );
	font.setPointSize( 8 );
    this->setFont( font );
    connect( this, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( showContextMenu( const QPoint& ) ) );
    connect( pDelegate_.get(), SIGNAL( valueChanged( const QModelIndex& ) ), this, SLOT( handleValueChanged( const QModelIndex& ) ) );
}

void
MSCalibSummaryWidget::OnCreate( const adportable::Configuration& )
{
}

void
MSCalibSummaryWidget::OnInitialUpdate()
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
    model.setHeaderData( c_mass_error2_calibrated_mDa, Qt::Horizontal, QObject::tr( "error(mDa) lsq(t0)" ) );
    model.setHeaderData( c_is_enable, Qt::Horizontal, QObject::tr( "enable" ) );
	model.setHeaderData( c_delta_mass, Qt::Horizontal, QObject::tr( "delta m/z" ) );
    model.setHeaderData( c_fcn, Qt::Horizontal, QObject::tr( "fcn" ) );
    model.setHeaderData( c_index, Qt::Horizontal, QObject::tr( "index" ) );
    this->setColumnHidden( c_index, true ); // internal index points peak on MassSpectrum
}

void
MSCalibSummaryWidget::onUpdate( boost::any& )
{
}

void
MSCalibSummaryWidget::OnFinalClose()
{
}

bool
MSCalibSummaryWidget::getContents( boost::any& any ) const
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
MSCalibSummaryWidget::getAssignedMasses( adcontrols::MSAssignedMasses& t ) const
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
MSCalibSummaryWidget::setContents( boost::any& )
{
    return false;
}


void
MSCalibSummaryWidget::getLifeCycle( adplugin::LifeCycle *& p )
{
    p = static_cast< adplugin::LifeCycle *>(this);
}

bool
MSCalibSummaryWidget::setAssignedData( int row, int fcn, int idx, const adcontrols::MSAssignedMasses& assigned )
{
    using namespace adcontrols::metric;

    QStandardItemModel& model = *pModel_;

    auto it = std::find_if( assigned.begin(), assigned.end(), [=]( const adcontrols::MSAssignedMass& a ){
            return fcn == a.idMassSpectrum() && idx == a.idPeak(); 
        });

    if ( it == assigned.end() )
        return false;

	double normalized_time = 0; // ( it->time() - t0 ) / pCalibrantSpectrum_->scanLaw().fLength( it->mode() );

    const adcontrols::MSCalibration& calib = pCalibResult_->calibration();
    double mass = calib.compute_mass( it->time() );
    double mass2 = 0;

    if ( calib.time_method() == adcontrols::MSCalibration::MULTITURN_NORMALIZED ) {
        double L = pCalibrantSpectrum_->scanLaw().fLength( it->mode() );
        do {
            const double t0 = pCalibResult_->t0(); // using fixed t0
            double T = scale_to( calib.time_prefix(), ( it->time() - t0 ) );
            normalized_time = ( it->time() - t0 ) / L;
            mass = calib.compute_mass( T / L );
        } while (0);
        do {
			const double t0 = scale_to_base( calib.compute( calib.t0_coeffs(), std::sqrt(mass) ), calib.time_prefix() );
            double T = scale_to( calib.time_prefix(), ( it->time() - t0 ) );
            normalized_time = ( it->time() - t0 ) / L;
            mass2 = calib.compute_mass( T / L );            
        } while (0);
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
        model.setData( model.index( row, c_mass_error2_calibrated_mDa ), ( mass2 - it->exactMass() ) * 1000 ); // mDa
    }

    model.setData( model.index( row, c_mode ), it->mode() );

    model.setData( model.index( row, c_is_enable ), it->enable() );

    if ( QStandardItem * chk = model.itemFromIndex( model.index( row, c_is_enable ) ) ) {
        chk->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | chk->flags() );
        model.setData( model.index( row, c_is_enable ), it->enable() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
    }

    return true;
}

bool
MSCalibSummaryWidget::createModelData( const std::vector< std::pair< int, int > >& indecies )
{
    QStandardItemModel& model = *pModel_;

    const adcontrols::MSCalibration& calib = pCalibResult_->calibration();

    model.removeRows( 0, model.rowCount() );
    model.insertRows( 0, indecies.size() ); 

    adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( *pCalibrantSpectrum_ );

    int row = 0;
    for ( auto idx : indecies ) {

        adcontrols::MassSpectrum& ms = segments[ idx.first ];

        model.setData( model.index( row, c_fcn ),   idx.first );
        model.setData( model.index( row, c_index ), idx.second );
        
        model.setData( model.index( row, c_mode ),  ms.getMSProperty().getSamplingInfo().mode ); // nTurns
        model.setData( model.index( row, c_mass ),  ms.getMass( idx.second ) );
        model.setData( model.index( row, c_time ),  ms.getTime( idx.second ) );
        model.setData( model.index( row, c_time_normalized ), ms.getNormalizedTime( idx.second ) );
        
        model.setData( model.index( row, c_intensity ), ms.getIntensity( idx.second ) );

        model.setData( model.index( row, c_mass_calibrated ), calib.compute_mass( ms.getTime( idx.second ) ) ); // ms.getNormalizedTime( idx.second ) ) );

        setAssignedData( row, idx.first, idx.second, pCalibResult_->assignedMasses() );
		setEditable( row );

        ++row;
    }
    return true;
}

void
MSCalibSummaryWidget::setEditable( int row, bool )
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
MSCalibSummaryWidget::modifyModelData( const std::vector< std::pair< int, int > >& indecies )
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

        setAssignedData( row, it->first, it->second, pCalibResult_->assignedMasses() );
    }
    return true;
}

void
MSCalibSummaryWidget::setData( const adcontrols::MSCalibrateResult& res, const adcontrols::MassSpectrum& ms )
{
    QStandardItemModel& model = *pModel_;

    if ( ! ms.isCentroid() )
        return;

    std::vector< std::pair< int, int > > indecies;
    *pCalibrantSpectrum_ = ms;
    *pCalibResult_ = res;

	double threshold = res.threshold();
    
    adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( *pCalibrantSpectrum_ );
    for ( size_t fcn = 0; fcn < segments.size(); ++fcn ) {
		adcontrols::MassSpectrum& fms = segments[ fcn ];
		for ( size_t idx = 0; idx < fms.size(); ++idx ) {
			if ( fms.getIntensity( idx ) > threshold )
				indecies.push_back( std::make_pair( fcn, idx ) );
		}
	}
    
    if ( ! ( ( model.rowCount() == indecies.size() ) && modifyModelData( indecies ) ) )
        createModelData( indecies );
}

void
MSCalibSummaryWidget::showContextMenu( const QPoint& pt )
{
    std::vector< QAction * > actions;
    QMenu menu;
    
    actions.push_back( menu.addAction( "Re-calc polynomials" ) );
    actions.push_back( menu.addAction( "Assign mass on spectrum" ) );
    actions.push_back( menu.addAction( "Apply calibration to current dataset" ) );
    actions.push_back( menu.addAction( "Save as default calibration" ) );
    actions.push_back( menu.addAction( "Copy summary to clipboard" ) );

    QAction * selected = menu.exec( this->mapToGlobal( pt ) );

    if ( selected == actions[ 0 ] ) {
        // emit on_recalibration_requested();      // re-calc polynomials and errors but no m/z axis on spectrum to be changed
        emit valueChanged();
    } else if ( selected == actions[ 1 ] ) {
        emit on_reassign_mass_requested();      // change source mass spectrum m/z array (both profile and centroid)
    } else if ( selected == actions[ 2 ] ) {
        emit on_apply_calibration_to_dataset(); // change whole calibration for current dataset
    } else if ( selected == actions[ 3 ] ) {
        emit on_apply_calibration_to_default(); // save calibration as system default
    } else if ( selected == actions[ 4 ] ) {
        copySummaryToClipboard();
    }
}

void
MSCalibSummaryWidget::handle_zoomed( const QRectF& rc )
{
	(void)rc;
}

void
MSCalibSummaryWidget::handle_selected( const QRectF& rc )
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
MSCalibSummaryWidget::handleEraseFormula()
{
}

void
MSCalibSummaryWidget::formulaChanged( const QModelIndex& index )
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
                model.setData( model.index( index.row(), c_is_enable ), false ); // set false by default
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
MSCalibSummaryWidget::handleValueChanged( const QModelIndex& index )
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
MSCalibSummaryWidget::currentChanged( const QModelIndex& index, const QModelIndex& prev )
{
    QStandardItemModel& model = *pModel_;
    (void)prev;
    scrollTo( index, QAbstractItemView::EnsureVisible );
	size_t row = index.row();
    size_t idx = model.index( row, c_index ).data( Qt::EditRole ).toInt();
    size_t fcn = model.index( row, c_fcn ).data( Qt::EditRole ).toInt();

    emit currentChanged( idx, fcn );

    double mass = model.index( row, c_mass ).data( Qt::EditRole ).toDouble();
    for ( int r = 0; r < model.rowCount(); ++r ) {
        double d = std::abs( model.index( r, c_mass ).data( Qt::EditRole ).toDouble() - mass );
        model.setData( model.index( r, c_delta_mass ), int( d + 0.7 ) );
    }
}

void
MSCalibSummaryWidget::handleCopyToClipboard()
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
            copy_table.append( QString("%1").arg( adcontrols::metric::scale_to_micro( model.data( idx ).toDouble() ) ) );
        else
            copy_table.append( model.data( idx ).toString() );
        prev = idx;
    }
    QApplication::clipboard()->setText( copy_table );
}

void
MSCalibSummaryWidget::copySummaryToClipboard()
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
MSCalibSummaryWidget::handlePasteFromClipboard()
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
MSCalibSummaryWidget::keyPressEvent( QKeyEvent * event )
{
    if ( event->matches( QKeySequence::Copy ) ) {
        handleCopyToClipboard();
    } else if ( event->matches( QKeySequence::Paste ) ) {
        handlePasteFromClipboard();
    } else {
        QTableView::keyPressEvent( event );
    }
}

namespace qtwidgets2 {

    class grid_render {
        const QRectF& rect_;
        double bottom_;
		QRectF boundingRect_;
        QRectF rc_;
    public:
        std::vector< std::pair< double, double > > tab_stops_;

        grid_render( const QRectF& rect )
            : bottom_( 0 ), rect_( rect ), rc_( rect ) {
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
MSCalibSummaryWidget::handlePrint( QPrinter& printer, QPainter& painter )
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
            MSCalibSummaryDelegate::to_print_text( text, model.index( row, col ) );
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

