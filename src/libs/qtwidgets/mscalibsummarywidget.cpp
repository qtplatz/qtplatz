// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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
#include <adutils/processeddata.hpp>
#include <qtwrapper/qstring.hpp>
#include <QMenu>
#include <boost/any.hpp>

namespace qtwidgets {

    enum {
        c_mass
        , c_time
        , c_intensity
        , c_formula
        , c_exact_mass
        , c_mass_error_mDa
        , c_is_enable
        , c_number_of_columns
    };
}

using namespace qtwidgets;

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
    rootNode->setColumnCount( 7 );
    
    model.setHeaderData( c_mass, Qt::Horizontal, QObject::tr( "m/z(calibrated)" ) );
    model.setHeaderData( c_time, Qt::Horizontal, QObject::tr( "time(us)" ) );
    model.setHeaderData( c_intensity, Qt::Horizontal, QObject::tr( "Intensity" ) );
    model.setHeaderData( c_formula, Qt::Horizontal, QObject::tr( "formula" ) );
    model.setHeaderData( c_exact_mass, Qt::Horizontal, QObject::tr( "m/z(exact)" ) );
    model.setHeaderData( c_mass_error_mDa, Qt::Horizontal, QObject::tr( "error(mDa)" ) );
    model.setHeaderData( c_is_enable, Qt::Horizontal, QObject::tr( "enable" ) );
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
		int row = currentIndex().row();
		ptr->setColor( row, 2 );
        return true;
    }
    if ( adutils::ProcessedData::is_type< boost::shared_ptr< adcontrols::MSAssignedMasses > >( any ) ) {
        boost::shared_ptr< adcontrols::MSAssignedMasses > ptr
            = boost::any_cast< boost::shared_ptr< adcontrols::MSAssignedMasses > >( any );
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
                double time = model.index( row, c_time ).data( Qt::EditRole ).toDouble();
                double mass = model.index( row, c_mass ).data( Qt::EditRole ).toDouble();
                double exact_mass = model.index( row, c_exact_mass ).data( Qt::EditRole ).toDouble();
                adcontrols::MSAssignedMass assigned( row, indecies_[ row ], wformula, exact_mass, time, mass, true );
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

    const double * intensities = ms.getIntensityArray();
    const double * times = ms.getTimeArray();
    const double * masses = ms.getMassArray();

    indecies_.clear();
    for ( size_t i = 0; i < ms.size(); ++i ) {
        if ( intensities[i] >= res.threshold() )
            indecies_.push_back( i ); 
    }

    pCalibrantSpectrum_.reset( new adcontrols::MassSpectrum );
    pCalibrantSpectrum_->clone( ms ); // shallow copy
    pCalibrantSpectrum_->resize( indecies_.size() );
    const unsigned char * colors = ms.getColorArray();

    model.insertRows( 0, indecies_.size() );
    std::vector< unsigned char > color_table( indecies_.size() );
    
    for ( size_t row = 0; row < indecies_.size(); ++row ) {
        size_t idx = indecies_[ row ];
        model.setData( model.index( row, c_mass ), masses[ idx ] );
        model.setData( model.index( row, c_time ), times[ idx ] * 1.0e6); // s -> us
        model.setData( model.index( row, c_intensity ), intensities[ idx ] );

        pCalibrantSpectrum_->setMass( row, masses[ idx ] );
        pCalibrantSpectrum_->setTime( row, times[ idx ] );
        pCalibrantSpectrum_->setIntensity( row, intensities[ idx ] );
        color_table[ row ] = colors[ idx ];
    }
    pCalibrantSpectrum_->setColorArray( &color_table[ 0 ] );

    const adcontrols::MSAssignedMasses& assigned = res.assignedMasses();

    for ( adcontrols::MSAssignedMasses::vector_type::const_iterator it = assigned.begin(); it != assigned.end(); ++it ) {
        std::vector< size_t >::iterator index
            = std::lower_bound( indecies_.begin(), indecies_.end(), it->idMassSpectrum() );
        size_t row = std::distance( indecies_.begin(), index );

        model.setData( model.index( row, c_formula ), qtwrapper::qstring::copy( it->formula() ) );
        model.setData( model.index( row, c_exact_mass ), it->exactMass() );
        model.setData( model.index( row, c_mass_error_mDa ), ( it->mass() - it->exactMass() ) * 1000 ); // mDa
        model.setData( model.index( row, c_is_enable ), it->enable() );
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
    menu.addAction( "Clear formulae", this, SLOT( handleClearFormulae() ) );
    menu.addAction( "Update calibration", this, SLOT( handleUpdateCalibration() ) );
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
            emit valueChanged();
        }
    }
}

void
MSCalibSummaryWidget::currentChanged( const QModelIndex& index, const QModelIndex& prev )
{
    (void)prev;
    emit currentChanged( index.row() );
}
