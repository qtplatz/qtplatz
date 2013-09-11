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

namespace qtwidgets2 {

    enum {
        c_formula
        , c_fcn  // subspectrum id
		, c_mode // analyzer mode id, a.k.a. reflectron|linear, or number of turns on InfiTOF
        , c_exact_mass
        , c_time
        , c_mass
        , c_intensity
        , c_mass_error_mDa
        , c_is_enable
        , c_flags
        , c_number_of_columns
    };
}

using namespace qtwidgets2;

MSCalibSummaryWidget::~MSCalibSummaryWidget()
{
}

MSCalibSummaryWidget::MSCalibSummaryWidget(QWidget *parent) : QTableView(parent)
                                                            , pModel_( new QStandardItemModel )
                                                            , pDelegate_( new MSCalibSummaryDelegate ) 
                                                            , inProgress_( false )
{
    this->setModel( pModel_.get() );
    this->setItemDelegate( pDelegate_.get() );
    this->setContextMenuPolicy( Qt::CustomContextMenu );
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
    
    model.setHeaderData( c_mode, Qt::Horizontal, QObject::tr( "#turns" ) );
    model.setHeaderData( c_fcn, Qt::Horizontal, QObject::tr( "fcn" ) );
    model.setHeaderData( c_mass, Qt::Horizontal, QObject::tr( "m/z(calibrated)" ) );
    model.setHeaderData( c_time, Qt::Horizontal, QObject::tr( "time(us)" ) );
    model.setHeaderData( c_intensity, Qt::Horizontal, QObject::tr( "Intensity" ) );
    model.setHeaderData( c_formula, Qt::Horizontal, QObject::tr( "formula" ) );
    model.setHeaderData( c_exact_mass, Qt::Horizontal, QObject::tr( "m/z(exact)" ) );
    model.setHeaderData( c_mass_error_mDa, Qt::Horizontal, QObject::tr( "error(mDa)" ) );
    model.setHeaderData( c_is_enable, Qt::Horizontal, QObject::tr( "enable" ) );
    model.setHeaderData( c_flags, Qt::Horizontal, QObject::tr( "flags" ) );
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
    if ( adutils::ProcessedData::is_type< adutils::MassSpectrumPtr >( any ) && pCalibrantSpectrum_ ) {
        adutils::MassSpectrumPtr ptr = boost::any_cast< adutils::MassSpectrumPtr >( any );
        *ptr = *pCalibrantSpectrum_;

        QStandardItemModel& model = *pModel_;
        for ( int row = 0; row < model.rowCount(); ++row ) {
            QString formula = model.data( model.index( row, c_formula ) ).toString();
            ptr->setColor( row, formula.isEmpty() ? 0 : 1 );
        }
        ptr->setColor(currentIndex().row(), 2 );
        return true;
    }

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
                double mass = model.index( row, c_mass ).data( Qt::EditRole ).toDouble();
                double exact_mass = model.index( row, c_exact_mass ).data( Qt::EditRole ).toDouble();
                bool flag = model.index( row, c_flags ).data( Qt::EditRole ).toBool();
                uint32_t mode = model.index( row, c_mode ).data( Qt::EditRole ).toInt();
				uint32_t fcn = model.index( row, c_fcn ).data( Qt::EditRole ).toInt();
				(void)fcn;
				adcontrols::MSAssignedMass assigned( -1, indecies_[ row ], wformula, exact_mass, time, mass, true, unsigned( flag ), mode );
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
	for ( size_t fcn = 0; fcn < ms.numSegments(); ++fcn ) {
		const adcontrols::MassSpectrum& fms = ( fcn == 0 ) ? ms : ms[ fcn - 1 ];
		adportable::array_wrapper< const double > intens( fms.getIntensityArray(), fms.size() );
		for ( size_t idx = 0; idx < fms.size(); ++idx )
			indecies_.push_back( std::make_pair( fcn, idx ) );
	}

    pCalibrantSpectrum_.reset( new adcontrols::MassSpectrum );
    pCalibrantSpectrum_->clone( ms ); // shallow copy
	pCalibrantSpectrum_->clearSegments();
    pCalibrantSpectrum_->resize( indecies_.size() );

    model.insertRows( 0, indecies_.size() );
    std::vector< unsigned char > color_table( indecies_.size() );
    
    for ( size_t row = 0; row < indecies_.size(); ++row ) {
        const std::pair<size_t, size_t>& idx = indecies_[ row ]; // <fcn, idx> pair for original centroid spectrum (before RA threasholded)
		const adcontrols::MassSpectrum& fms = ( idx.first == 0 ) ? ms : ms[ idx.first - 1 ];
		model.setData( model.index( row, c_mass ), fms.getMass( idx.second ) );
        model.setData( model.index( row, c_time ), fms.getTime( idx.second ) * 1.0e6 ); // in microseconds
		model.setData( model.index( row, c_intensity ), fms.getIntensity( idx.second ) );

        pCalibrantSpectrum_->setMass( row, fms.getMass( idx.second ) );
        pCalibrantSpectrum_->setTime( row, fms.getTime( idx.second ) );
		pCalibrantSpectrum_->setIntensity( row, fms.getIntensity( idx.second ) );
		int color = fms.getColor( idx.second );
		color_table[ row ] = color > 0 ? color : 0;
    }
    pCalibrantSpectrum_->setColorArray( color_table.data() );

    const adcontrols::MSAssignedMasses& assigned = res.assignedMasses();

    for ( auto it = assigned.begin(); it != assigned.end(); ++it ) {
		/*
        auto index = std::lower_bound( indecies_.begin(), indecies_.end(), it->idMassSpectrum() );
        
		size_t row = std::distance( indecies_.begin(), index );

        model.setData( model.index( row, c_formula ), qtwrapper::qstring::copy( it->formula() ) );
        model.setData( model.index( row, c_exact_mass ), it->exactMass() );
        model.setData( model.index( row, c_mass_error_mDa ), ( it->mass() - it->exactMass() ) * 1000 ); // mDa
        model.setData( model.index( row, c_is_enable ), it->enable() );
        model.setData( model.index( row, c_flags ), bool( it->flags() ) );
        model.setData( model.index( row, c_mode ), it->mode() );
		*/
    }
}

void
MSCalibSummaryWidget::showContextMenu( const QPoint& pt )
{
    QMenu menu;

    if ( this->indexAt( pt ).isValid() ) {
        QModelIndex index = this->currentIndex();
        if ( index.column() == c_formula )
            menu.addAction( "Erase", this, SLOT( handleEraseFormula() ) );
    }
    menu.addAction( "Update peak assign", this, SLOT( handleUpdatePeakAssign() ) );
    menu.addAction( "Update calibration", this, SLOT( handleUpdateCalibration() ) );
    menu.addAction( "Clear formulae", this, SLOT( handleClearFormulae() ) );
    menu.exec( this->mapToGlobal( pt ) );
}


void
MSCalibSummaryWidget::handleEraseFormula()
{
    QModelIndex index = this->currentIndex();
    //model_->removeRows( index.row(), 1 );
    //emit lineDeleted( index.row() );
}

void
MSCalibSummaryWidget::handleUpdateCalibration()
{
    emit applyTriggered();
}

void
MSCalibSummaryWidget::handleUpdatePeakAssign()
{
    emit applyPeakAssign();
}

void
MSCalibSummaryWidget::handleClearFormulae()
{
    inProgress_ = true;
    bool modified = false;
    QStandardItemModel& model = *pModel_;
    for ( int row = 0; row < model.rowCount(); ++row ) {
        QString formula = model.data( model.index( row, c_formula ) ).toString();
        if ( ! formula.isEmpty()  ) {
            model.setData( model.index( row, c_formula ), "" );
            model.setData( model.index( row, c_exact_mass ), "" );
            model.setData( model.index( row, c_mass_error_mDa ), "" );
            model.setData( model.index( row, c_is_enable ), "" );
            model.setData( model.index( row, c_flags ), "" );
            modified = true;
        }
    }
    inProgress_ = false;

    if ( modified )
        emit valueChanged();
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
    emit currentChanged( index.row() );
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

    if ( pCalibrantSpectrum_ ) {
        const adcontrols::MSCalibration& calib = pCalibrantSpectrum_->calibration();
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
    
    QModelIndex last = list.last();
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

