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
#include <algorithm>

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
    model.setHeaderData( c_mass, Qt::Horizontal, QObject::tr( "m/z" ) );
    model.setHeaderData( c_mass_calibrated, Qt::Horizontal, QObject::tr( "m/z(calibrated)" ) );
    model.setHeaderData( c_mode, Qt::Horizontal, QObject::tr( "#turns" ) );
    model.setHeaderData( c_fcn, Qt::Horizontal, QObject::tr( "fcn" ) );
    model.setHeaderData( c_intensity, Qt::Horizontal, QObject::tr( "Intensity" ) );
    model.setHeaderData( c_formula, Qt::Horizontal, QObject::tr( "formula" ) );
    model.setHeaderData( c_exact_mass, Qt::Horizontal, QObject::tr( "m/z(exact)" ) );
    model.setHeaderData( c_mass_error_mDa, Qt::Horizontal, QObject::tr( "error(mDa)" ) );
    model.setHeaderData( c_mass_error_calibrated_mDa, Qt::Horizontal, QObject::tr( "error(mDa) calibrated" ) );
    model.setHeaderData( c_is_enable, Qt::Horizontal, QObject::tr( "enable" ) );
    model.setHeaderData( c_flags, Qt::Horizontal, QObject::tr( "exclude" ) );
}

void
MSCalibSummaryWidget::OnUpdate( boost::any& )
{
}

void
MSCalibSummaryWidget::OnFinalClose()
{
}

bool
MSCalibSummaryWidget::getContents( boost::any& any ) const
{
	// colored spectrum for GUI display
    if ( adutils::ProcessedData::is_type< adutils::MassSpectrumPtr >( any ) && pCalibrantSpectrum_ ) {
        adutils::MassSpectrumPtr ptr = boost::any_cast< adutils::MassSpectrumPtr >( any );
        *ptr = *pCalibrantSpectrum_; // deep copy

        // QStandardItemModel& model = *pModel_;
        size_t row = currentIndex().row();
        if ( row  < indecies_.size() )
            adcontrols::segments_helper::set_color( *ptr, indecies_[ row ].first, indecies_[ row ].second, 2 );

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
            if ( model.index( row, c_is_enable ).data( Qt::EditRole ).toBool() ) {
                double time = model.index( row, c_time ).data( Qt::EditRole ).toDouble() / 1.0e6; // us -> s
                double exact_mass = model.index( row, c_exact_mass ).data( Qt::EditRole ).toDouble();
                double mass = model.index( row, c_mass ).data( Qt::EditRole ).toDouble();
                bool flag = model.index( row, c_flags ).data( Qt::EditRole ).toBool();
                uint32_t mode = model.index( row, c_mode ).data( Qt::EditRole ).toInt();
				uint32_t fcn = model.index( row, c_fcn ).data( Qt::EditRole ).toInt();
                assert( fcn == indecies_[ row ].first );
				adcontrols::MSAssignedMass assigned( -1, fcn, indecies_[ row ].second
                                                     , wformula, exact_mass, time, mass, true, unsigned( flag ), mode );
                t << assigned;
            }
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

void
MSCalibSummaryWidget::setData( const adcontrols::MSCalibrateResult& res, const adcontrols::MassSpectrum& ms )
{
    QStandardItemModel& model = *pModel_;
    model.removeRows( 0, model.rowCount() );

    if ( ! ms.isCentroid() )
        return;

	indecies_.clear();
    *pCalibrantSpectrum_ = ms;
    pCalibResult_.reset( new adcontrols::MSCalibrateResult( res ) );

	double threshold = res.threshold();

    adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( *pCalibrantSpectrum_ );
    for ( size_t fcn = 0; fcn < segments.size(); ++fcn ) {
		adcontrols::MassSpectrum& fms = segments[ fcn ];
		for ( size_t idx = 0; idx < fms.size(); ++idx ) {
			if ( fms.getIntensity( idx ) > threshold )
				indecies_.push_back( std::make_pair( fcn, idx ) );
		}
	}

    model.insertRows( 0, indecies_.size() );
    adcontrols::MSCalibration calib = res.calibration();
    
    size_t row = 0;
    for ( auto idx: indecies_ ) {
        adcontrols::MassSpectrum& fms = segments[ idx.first ];
		const adcontrols::MSProperty& msprp = fms.getMSProperty();
		double mass = fms.getMass( idx.second );
		double time = fms.getTime( idx.second ); // s
		model.setData( model.index( row, c_fcn ), idx.first );
		model.setData( model.index( row, c_mode ), msprp.getSamplingInfo().mode ); // nTurns
		model.setData( model.index( row, c_mass ), mass );
        model.setData( model.index( row, c_time ), time * 1e6);
		model.setData( model.index( row, c_intensity ), fms.getIntensity( idx.second ) );
        double mq = calib.compute( calib.coeffs(), time );
        if ( mq > 0.0 )
            model.setData( model.index( row, c_mass_calibrated ), mq * mq );
		++row;
    }

    // add assined peak info.
    for ( auto assigned: res.assignedMasses() ) { // it = assigned.begin(); it != assigned.end(); ++it ) {
		auto rowIt = std::find_if( indecies_.begin(), indecies_.end(), [=]( const std::pair< uint32_t, uint32_t >& index ){ 
			return index.first == assigned.idMassSpectrum() && index.second == assigned.idPeak(); } );
        if ( rowIt != indecies_.end() ) {
            size_t row = std::distance( indecies_.begin(), rowIt );
			model.setData( model.index( row, c_formula ), qtwrapper::qstring::copy( assigned.formula() ) );
            model.setData( model.index( row, c_exact_mass ), assigned.exactMass() );
            model.setData( model.index( row, c_mass_error_mDa ), ( assigned.mass() - assigned.exactMass() ) * 1000 ); // mDa
            double mz = model.index( row, c_mass_calibrated).data( Qt::EditRole ).toDouble();
            model.setData( model.index( row, c_mass_error_calibrated_mDa ), ( mz - assigned.exactMass() ) * 1000 ); // mDa
            model.setData( model.index( row, c_mode ), assigned.mode() );

            do {
                model.setData( model.index( row, c_is_enable ), assigned.enable() );
                QStandardItem * chk = model.itemFromIndex( model.index( row, c_is_enable ) );
                if ( chk ) {
                    chk->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
                    chk->setEditable( true );
                    model.setData( model.index( row, c_is_enable ), assigned.enable() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
                }
            } while ( 0 );

            do {
                model.setData( model.index( row, c_flags ), bool( assigned.flags() ) );
                QStandardItem * chk = model.itemFromIndex( model.index( row, c_flags ) );
                if ( chk ) {
                    chk->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
                    chk->setEditable( true );
                    bool f = bool( assigned.flags() );
                    model.setData( model.index( row, c_flags ), f ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
                }
            } while ( 0 );

        }
    }
    
}

void
MSCalibSummaryWidget::showContextMenu( const QPoint& pt )
{
    std::vector< QAction * > actions;
    QMenu menu;
    
    actions.push_back( menu.addAction( "update m/z axis on spectrum" ) );
    actions.push_back( menu.addAction( "calculate polynomials and m/z errors" ) );
    actions.push_back( menu.addAction( "apply calibration to current dataset" ) );
    actions.push_back( menu.addAction( "save as default calibration" ) );

    QAction * selected = menu.exec( this->mapToGlobal( pt ) );

    if ( selected == actions[ 0 ] ) {
        emit on_reassign_mass_requested();      // change source mass spectrum m/z array (both profile and centroid)
    } else if ( selected == actions[ 1 ] ) {
        emit on_recalibration_requested();      // re-calc polynomials and errors but no m/z axis on spectrum to be changed
    } else if ( selected == actions[ 2 ] ) {
        emit on_apply_calibration_to_dataset(); // change whole calibration for current dataset
    } else if ( selected == actions[ 3 ] ) {
        emit on_apply_calibration_to_default(); // save calibration as system default
    }
}

void
MSCalibSummaryWidget::handle_zoomed( const QRectF& rc )
{
    handle_selected( rc ); // focus to base peak
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
		scrollTo( pModel_->index( row_highest, c_formula ), QAbstractItemView::PositionAtTop );
    }
}

void
MSCalibSummaryWidget::handleEraseFormula()
{
    // QModelIndex index = this->currentIndex();
    //model_->removeRows( index.row(), 1 );
    //emit lineDeleted( index.row() );
}

void
MSCalibSummaryWidget::handleValueChanged( const QModelIndex& index )
{
    QStandardItemModel& model = *pModel_;

    if ( inProgress_ )
        return;

    if ( index.column() == c_formula ) {
        std::wstring formula = qtwrapper::wstring( index.data( Qt::EditRole ).toString() );
        if ( ! formula.empty() ) {
            adcontrols::ChemicalFormula cformula;
            double exactMass = cformula.getMonoIsotopicMass( formula );
            // update exact mass
            model.setData( model.index( index.row(), c_exact_mass ), exactMass );
            model.setData( model.index( index.row(), c_is_enable ), true );
            model.setData( model.index( index.row(), c_flags ), true );
            emit valueChanged();
        }
	} else if ( index.column() == c_mode ) {
		emit valueChanged();
	}
}

void
MSCalibSummaryWidget::currentChanged( const QModelIndex& index, const QModelIndex& prev )
{
    (void)prev;
    scrollTo( index, QAbstractItemView::EnsureVisible );
	size_t row = index.row();
	if ( row < indecies_.size() ) {
		emit currentChanged( indecies_[ row ].second, indecies_[ row ].first );
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
    QString heading;

    if ( pCalibResult_ ) {
        const adcontrols::MSCalibration& calib = pCalibResult_->calibration();
        heading.append( "Calibration date:\t" );
        heading.append( calib.date().c_str() );
        heading.append( "\tid\t" );
        heading.append( qtwrapper::qstring( calib.calibId() ) );
        heading.append( '\n' );
        heading.append( "SQRT( m/z ) = " );
        for ( size_t i = 0; i < calib.coeffs().size(); ++i ) {
            QString term = i ? ( boost::format( "%c*X^%d" ) % char( 'a' + i ) % ( i ) ).str().c_str() : "a";
            heading.append( '\t' );
            heading.append( term );
        }
        heading.append( '\n' );
        for ( size_t i = 0; i < calib.coeffs().size(); ++i ) {
            QString term = ( boost::format( "%.14lf" ) % calib.coeffs()[ i ] ).str().c_str();
            heading.append( '\t' );
            heading.append( term );
        }
        heading.append( '\n' );
    }
    
    // QModelIndex last = list.last();
    QModelIndex prev = list.first();

    do {
        for ( size_t column = prev.column(); column < c_number_of_columns; ++column ) {
            QString text = model.headerData( column, Qt::Horizontal ).toString();
            heading.append( text );
            if ( column < c_number_of_columns )
                heading.append( '\t' );
        }
        heading.append( '\n' );
    } while ( 0 );

    copy_table.append( heading );

    list.removeFirst();
    for ( int i = 0; i < list.size(); ++i ) {
        QString text = model.data( prev ).toString();
        copy_table.append( text );
        QModelIndex index = list.at( i );
        if ( index.row() == prev.row() )
            copy_table.append( '\t' );
        else
            copy_table.append( '\n' );
        prev = index;
    }
    copy_table.append( model.data( list.last() ).toString() );
    QApplication::clipboard()->setText( copy_table );
}

void
MSCalibSummaryWidget::keyPressEvent( QKeyEvent * event )
{
    if ( event->matches( QKeySequence::Copy ) ) {
        handleCopyToClipboard();
    } else {
        QTableView::keyPressEvent( event );
    }
}

