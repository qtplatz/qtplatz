/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include "peakmethodform.hpp"
#include "ui_peakmethodform.h"
#include "tableview.hpp"

#include <adcontrols/peakmethod.hpp>
#include <adcontrols/processmethod.hpp>
#include <adportable/configuration.hpp>
#include <adlog/logger.hpp>
#include <adportable/is_type.hpp>
#include <boost/any.hpp>
#include <boost/format.hpp>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QSignalBlocker>
#include <QTextDocument>
#include <QTreeView>
#include <QPainter>
#include <QKeyEvent>
#include <QEvent>
#include <QDebug>

using namespace adcontrols::chromatography;

namespace adwidgets {

    enum { c_time, c_function, c_event_value };

    namespace peakmethodform {
    
        class teDelegate : public QStyledItemDelegate {
        public:
            explicit teDelegate( PeakMethodForm *, QObject *parent = 0 );
            
            QWidget * createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
            void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
            void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
        private:
            PeakMethodForm * form_;
        };

    }

    class PeakMethodForm::impl {
        PeakMethodForm * this_;
    public:
        impl( PeakMethodForm * p ) : this_( p )
                                   , model_( new QStandardItemModel ) {
        }
                 
        std::unique_ptr< QStandardItemModel > model_;
    };
}

using namespace adwidgets;
using namespace adwidgets::peakmethodform;

PeakMethodForm::PeakMethodForm( QWidget *parent ) : QWidget( parent )
                                                  , ui( new Ui::PeakMethodForm )
                                                  , impl_( new impl( this ) )
{
    ui->setupUi(this);

    ui->tableView->setModel( impl_->model_.get() );
    ui->tableView->setItemDelegate( new teDelegate( this ) );
    ui->tableView->verticalHeader()->setDefaultSectionSize( 18 );
}

PeakMethodForm::~PeakMethodForm()
{
    delete ui;
}

void
PeakMethodForm::OnCreate( const adportable::Configuration& config )
{
    (void)config;
}

void
PeakMethodForm::OnInitialUpdate()
{
    // initialize with ctor default value
    setContents( adcontrols::PeakMethod() );

    if ( auto table = findChild< TableView * >() ) {
        
        QStandardItemModel& model = *impl_->model_;
        model.setColumnCount( 3 );
        model.setHeaderData( c_time, Qt::Horizontal, "Time(min)" );
        model.setHeaderData( c_function, Qt::Horizontal, "Func" );
        model.setHeaderData( c_event_value, Qt::Horizontal, "Value" );
        table->setSortingEnabled( true );

    }
}

void
PeakMethodForm::OnFinalClose()
{
}

bool
PeakMethodForm::getContents( boost::any& any ) const
{
    if ( adportable::a_type< adcontrols::ProcessMethod >::is_pointer( any ) ) {

        adcontrols::ProcessMethod* pm = boost::any_cast< adcontrols::ProcessMethod* >( any );
        getContents( *pm );

        return true;
    }

    return false;
}

bool
PeakMethodForm::setContents( boost::any& any )
{
    if ( adportable::a_type< adcontrols::ProcessMethod >::is_a( any ) ) {

        adcontrols::ProcessMethod& pm = boost::any_cast< adcontrols::ProcessMethod& >( any );
        if ( const adcontrols::PeakMethod *p = pm.find< adcontrols::PeakMethod >() ) {
            setContents( *p );
            return true;
        }
    }
    return false;
}

void
PeakMethodForm::getContents( adcontrols::ProcessMethod& pm ) const
{
    adcontrols::PeakMethod pkm;
    getContents( pkm );
    pm.appendMethod< adcontrols::PeakMethod >( pkm );
}

void
PeakMethodForm::setContents( const adcontrols::PeakMethod& method )
{
    ui->doubleSpinBox->setValue( method.slope() );
    ui->doubleSpinBox_2->setValue( method.minimumWidth() );
    ui->doubleSpinBox_3->setValue( method.minimumHeight() );
    ui->doubleSpinBox_4->setValue( method.drift() );
    ui->doubleSpinBox_5->setValue( method.minimumArea() );
    ui->doubleSpinBox_6->setValue( method.t0() );
    // method.doubleWidthTime( 0.0 );
    ui->comboBox->setCurrentIndex( int( method.pharmacopoeia() ) );

    do {
        QStandardItemModel& model = *impl_->model_;
		
		model.setRowCount( static_cast< int >( method.size() + 1 ) ); // add one blank line at end
		int row = 0;
        for ( const auto& item : method ) {

            if ( item.peakEvent() != ePeakEvent_Nothing ) {

                model.setData( model.index( row, c_time ), item.time() );
                model.setData( model.index( row, c_function ), item.peakEvent() );
                if ( item.isBool() )
                    model.setData( model.index( row, c_event_value ), item.boolValue() );
                else
                    model.setData( model.index( row, c_event_value ), item.doubleValue() );

                ++row;
            }
		}
        
    } while(0);
}

void
PeakMethodForm::getContents( adcontrols::PeakMethod& method ) const
{
    method.slope( ui->doubleSpinBox->value() );
    method.minimumWidth( ui->doubleSpinBox_2->value() );
    method.minimumHeight( ui->doubleSpinBox_3->value() );
    method.drift( ui->doubleSpinBox_4->value() );
    method.minimumArea( ui->doubleSpinBox_5->value() );
    method.doubleWidthTime( 0.0 );
    method.t0( ui->doubleSpinBox_6->value() );

    int value = ui->comboBox->currentIndex();
    method.pharmacopoeia( static_cast< adcontrols::chromatography::ePharmacopoeia >( value ) );

    
    switch( method.pharmacopoeia() ) {
    case ePHARMACOPOEIA_USP:
        method.theoreticalPlateMethod( ePeakWidth_Tangent );
        break;
    case ePHARMACOPOEIA_EP:
    case ePHARMACOPOEIA_JP:
        method.theoreticalPlateMethod( ePeakWidth_HalfHeight );
        method.peakWidthMethod( ePeakWidth_HalfHeight );        
        break;
    }

    QStandardItemModel& model = *impl_->model_;
    
    method.erase( method.begin(), method.end() );
    
    for ( int row = 0; row < model.rowCount(); ++row ) {

        double minutes = model.data( model.index( row, c_time ) ).toDouble();
        adcontrols::chromatography::ePeakEvent func 
            = static_cast< adcontrols::chromatography::ePeakEvent >( model.data( model.index( row, c_function ) ).toInt() );
        if ( func != ePeakEvent_Nothing ) {
            const QVariant value = model.data( model.index( row, c_event_value ) );
            adcontrols::PeakMethod::TimedEvent e( adcontrols::timeutil::toSeconds( minutes ), func );
            if ( e.isBool() )
                e.setValue( value.toBool() );
            else
                e.setValue( value.toDouble() );
            method << e;
        }
    }
}

namespace adwidgets { namespace internal {

        static const char * functions [] = {
            "Off"
            // , "Forced base"
            // , "Shift base"
            // , "V-to-V"
            // , "Tailing"
            // , "Leading"
            // , "Shoulder"
            // , "Negative peak"
            // , "Negative lock"
            // , "Horizontal base"
            // , "Post horizontal base"
            // , "Forced peak"
            // , "Slope"
            // , "Minimum width"
            // , "Minimum height"
            // , "Minimum area"
            // , "Drift"
            // , "Elmination"
            // , "Manual"
        };
    }
}

#define countof(x) ( sizeof(x)/sizeof(x[0]) )

teDelegate::teDelegate(PeakMethodForm * form, QObject *parent) : QStyledItemDelegate(parent), form_( form )
{
}

QWidget *
teDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if ( index.column() == c_function ) {
        QComboBox * pCombo = new QComboBox( parent );
        QStringList list;
        for ( size_t i = 0; i < countof( internal::functions ); ++i )
            list << internal::functions[ i ];
        pCombo->addItems( list );
        return pCombo;
    } else {
        return QStyledItemDelegate::createEditor( parent, option, index );
  }
}

void
teDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if ( index.column() == c_function ) {
        int idx = index.data().toInt() - 1;
        if ( size_t( idx ) < countof( internal::functions ) )
            painter->drawText( option.rect, Qt::AlignLeft | Qt::AlignVCenter, internal::functions[ idx ] );
        else
            painter->drawText( option.rect, Qt::AlignLeft | Qt::AlignVCenter, index.data().toString() );

    } else if ( index.column() == c_time ) {

        double value = index.data().toDouble();
        painter->drawText( option.rect, Qt::AlignLeft | Qt::AlignVCenter, QString::number( value, 'f', 4 ) );

    } else if ( index.column() == c_event_value ) {

        painter->drawText( option.rect, Qt::AlignLeft | Qt::AlignVCenter, index.data().toString() );
        // double value = index.data().toDouble();
        // painter->drawText( option.rect, Qt::AlignLeft | Qt::AlignVCenter, QString::number( value, 'f', 3 ) );

    } else {
        QStyledItemDelegate::paint( painter, option, index );
    }
}

void
teDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if ( index.column() == c_function ) {

        QComboBox * p = dynamic_cast< QComboBox * >( editor );
        adcontrols::chromatography::ePeakEvent func = static_cast< adcontrols::chromatography::ePeakEvent >( p->currentIndex() + 1 );
        
        model->setData( index, func );

        if ( adcontrols::PeakMethod::TimedEvent::isBool( func ) ) {
            if ( model->index( index.row(), c_event_value ).data( Qt::EditRole ).type() != QVariant::Bool )
                model->setData( model->index( index.row(), c_event_value ), QVariant(false), Qt::EditRole );
        } else {
            if ( model->index( index.row(), c_event_value ).data( Qt::EditRole ).type() != QVariant::Double )
                model->setData( model->index( index.row(), c_event_value ), QVariant(0.0), Qt::EditRole );
        }

    } else {

        QStyledItemDelegate::setModelData( editor, model, index );

    }

    if ( index.row() == model->rowCount() - 1 )
        model->insertRow( index.row() + 1 ); // add last blank line

    emit form_->valueChanged();
}

////////////////////////////////////

