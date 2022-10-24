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
#include "utilities.hpp"
// #include "ui_peakmethodform.h"
#include "tableview.hpp"
#include <adcontrols/peakmethod.hpp>
#include <adcontrols/processmethod.hpp>
#include <adplot/constants.hpp>
#include <adportable/configuration.hpp>
#include <adportable/debug.hpp>
#include <adportable/is_type.hpp>
#include <boost/any.hpp>
#include <boost/format.hpp>
#include <QAbstractButton>
#include <QBoxLayout>
#include <QGridLayout>
#include <QComboBox>
#include <QDebug>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QGroupBox>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QSignalBlocker>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTextDocument>
#include <QTreeView>


using namespace adcontrols::chromatography;

namespace adwidgets {

    class teDelegate : public QStyledItemDelegate {
    public:
        explicit teDelegate( adwidgets::PeakMethodForm *, QObject *parent = 0 );

        QWidget * createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
        void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
        void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
    private:
        adwidgets::PeakMethodForm * form_;
    };
}


namespace adwidgets {

    enum { c_time, c_function, c_event_value };

    class PeakMethodForm::impl {
    public:
        impl( PeakMethodForm * ) : model_( new QStandardItemModel ) {
        }

        std::unique_ptr< QStandardItemModel > model_;
    };
}

using namespace adwidgets;

PeakMethodForm::PeakMethodForm( QWidget *parent ) : QWidget( parent )
                                                    //, ui( new Ui::PeakMethodForm )
                                                  , impl_( new impl( this ) )
{
    // ui->setupUi(this);
    using namespace spin_initializer;

    if ( auto hTopLayout = new QHBoxLayout( this ) ) { // ui->horizontalLayout_2 ) {
        hTopLayout->setContentsMargins( 0, 0, 0, 0 );
        hTopLayout->setSpacing( 2 );
        if ( auto vLeft = add_layout( hTopLayout, create_widget< QVBoxLayout >("vLayout") ) ) {
            vLeft->setContentsMargins( 0, 0, 0, 0 );
            if ( auto gbx = add_widget( vLeft, create_widget< QGroupBox >( "globalGroup", "Global" ) ) ) {
                if ( auto grid = create_widget< QGridLayout >( "grid", gbx ) ) {
                    grid->setSpacing( 2 );
                    grid->setContentsMargins( 2, 0, 2, 0 );
                    std::tuple< size_t, size_t > xy{0,0};
                    if ( auto label = add_widget( grid, create_widget< QLabel >( "labelSlope", "Slope (&mu;V/min)"), std::get<0>(xy), std::get<1>(xy)++ ) ) {
                        label->setTextFormat(Qt::RichText);
                        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
                        if ( auto spin = add_widget( grid, create_widget< QDoubleSpinBox >( "doubleSpinBoxSlope"), std::get<0>(xy), std::get<1>(xy)++ ) ) {
#if __cplusplus >= 201703L // same all folloing call of spin_init
                            spin_init( spin, std::tuple{
                                    Decimals{2}, Minimum{0.0}, Maximum{99.999}, SingleStep{0.01}, Value{0.05}, Alignment{Qt::AlignRight} } );
#else // for gcc 6.3 support
                            spin_init( spin, std::make_tuple(
                                           Decimals{2}, Minimum<>{0.0}, Maximum<>{99.999}, SingleStep<>{0.01}, Value<>{0.05},Alignment{Qt::AlignRight} ) );
#endif
                        }
                    }
                    ++xy;
                    if ( auto label = add_widget( grid, create_widget< QLabel >( "labelMW", "Minimum width (s)"), std::get<0>(xy), std::get<1>(xy)++ ) ) {
                        label->setTextFormat(Qt::RichText);
                        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
                        if ( auto spin = add_widget( grid, create_widget< QDoubleSpinBox >( "doubleSpinBoxMW"), std::get<0>(xy), std::get<1>(xy)++ ) ) {
                            spin_init( spin, std::make_tuple(
                                           Decimals{2}, Minimum<>{0.0}, Maximum<>{99.999}, SingleStep<>{0.10}, Value<>{0.1}, Alignment{Qt::AlignRight} ) );
                        }
                    }
                    ++xy;
                    if ( auto label = add_widget( grid, create_widget< QLabel >( "labelMH", "Minimum height"), std::get<0>(xy), std::get<1>(xy)++ ) ) {
                        label->setTextFormat(Qt::RichText);
                        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
                        if ( auto spin = add_widget( grid, create_widget< QDoubleSpinBox >( "doubleSpinBoxMH"), std::get<0>(xy), std::get<1>(xy)++ ) ) {
                            spin_init( spin, std::make_tuple(
                                           Decimals{2}, Minimum<>{0.0}, Maximum<>{999999999999.9}, SingleStep<>{1.0}, Value<>{1.0},Alignment{Qt::AlignRight} ) );
                        }
                    }
                    ++xy;
                    if ( auto label = add_widget( grid, create_widget< QLabel >( "labelDrift", "Drift (&mu;V/s)"), std::get<0>(xy), std::get<1>(xy)++ ) ) {
                        label->setTextFormat(Qt::RichText);
                        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
                        if ( auto spin = add_widget( grid, create_widget< QDoubleSpinBox >( "doubleSpinBoxDrift"), std::get<0>(xy), std::get<1>(xy)++ ) ) {
                            spin_init( spin, std::make_tuple(
                                           Decimals{2}, Minimum<>{0.0}, Maximum<>{9999.99}, SingleStep<>{0.1}, Value<>{0.1}, Alignment{Qt::AlignRight} ) );
                        }
                    }
                    ++xy;
                    if ( auto label = add_widget( grid, create_widget< QLabel >( "labelMA", "Minimum area (&mu;V&times;s)"), std::get<0>(xy), std::get<1>(xy)++ ) ) {
                        label->setTextFormat(Qt::RichText);
                        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
                        if ( auto spin = add_widget( grid, create_widget< QDoubleSpinBox >( "doubleSpinBoxMA"), std::get<0>(xy), std::get<1>(xy)++ ) ) {
                            spin_init( spin, std::make_tuple(
                                           Decimals{2}, Minimum<>{0.0}, Maximum<>{999999999999.9}, SingleStep<>{1.0}, Value<>{1.0}, Alignment{Qt::AlignRight} ) );
                        }
                    }
                    ++xy;
                    if ( auto label = add_widget( grid, create_widget< QLabel >( "labelT0", "<i>T<sub>0</sub>"), std::get<0>(xy), std::get<1>(xy)++ ) ) {
                        label->setTextFormat(Qt::RichText);
                        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
                        if ( auto spin = add_widget( grid, create_widget< QDoubleSpinBox >( "doubleSpinBoxT0"), std::get<0>(xy), std::get<1>(xy)++ ) ) {
                            spin_init( spin, std::make_tuple(
                                           Decimals{3}, Minimum<>{0.0}, Maximum<>{3600.0}, SingleStep<>{4.0}, Value<>{0.1}, Alignment{Qt::AlignRight} ) );
                        }
                    }
                    ++xy;
                    if ( auto label = add_widget( grid, create_widget< QLabel >( "labelPC", "Pharmacopoeia"), std::get<0>(xy), std::get<1>(xy)++ ) ) {
                        label->setTextFormat(Qt::RichText);
                        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
                        if ( auto combo = add_widget( grid, create_widget< QComboBox >( "comboBoxPharmacopoeia"), std::get<0>(xy), std::get<1>(xy)++ ) ) {
                            combo->addItems( {"Not specified", "EP", "JP", "USP"} );
                        }
                    }
                    vLeft->addSpacerItem( new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding) );
                    if ( auto buttonBox = add_widget( vLeft, create_widget< QDialogButtonBox >( "buttonBox" ) ) ) {
                        buttonBox->setStandardButtons(QDialogButtonBox::Apply);
                    }
                }
            }
        }
        if ( auto vRight = add_layout( hTopLayout, create_widget< QVBoxLayout >("vLayout") ) ) {
            vRight->setContentsMargins( 0, 0, 0, 0 );
            if ( auto gbx = add_widget( vRight, create_widget< QGroupBox >( "timedEventsGroup", "Timed event" ) ) ) {
                auto layout = new QVBoxLayout( gbx );
                layout->setContentsMargins( 0, 0, 0, 0 );
                if ( auto table = add_widget( layout, create_widget< adwidgets::TableView >( "timedEventTable", gbx ) ) ) {
                    table->setModel( impl_->model_.get() );
                    table->setItemDelegate( new teDelegate( this ) );
                    table->verticalHeader()->setDefaultSectionSize( 18 );
                }
            }
        }

        hTopLayout->setStretch( 0, 0 );
        hTopLayout->setStretch( 1, 1 );
    }

    if ( auto buttonBox = findChild< QDialogButtonBox * >( "buttonBox" ) ) {
        connect( buttonBox, &QDialogButtonBox::clicked, [this] () { emit triggerProcess( "PeakFind" ); } );
    }
}

PeakMethodForm::~PeakMethodForm()
{
    // delete ui;
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
    if ( auto table = findChild< TableView * >( "tableView" ) ) {
        table->setHidden( true );
    }

    if ( auto table = findChild< TableView * >( "timedEventTable" ) ) {
        QStandardItemModel& model = *impl_->model_;
        model.setColumnCount( 3 );
        model.setHeaderData( c_time, Qt::Horizontal, "Time (s)" );
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
PeakMethodForm::setContents( boost::any&& any )
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
    if ( auto spin = findChild< QDoubleSpinBox * >("doubleSpinBoxSlope") ) {
        spin->setValue( method.slope() );
    }
    if ( auto spin = findChild< QDoubleSpinBox * >("doubleSpinBoxMW") ) {
        spin->setValue( method.minimumWidth() );
    }
    if ( auto spin = findChild< QDoubleSpinBox * >("doubleSpinBoxMH") ) {
        spin->setValue( method.minimumHeight() );
    }
    if ( auto spin = findChild< QDoubleSpinBox * >("doubleSpinBoxDrift") ) {
        spin->setValue( method.drift() );
    }
    if ( auto spin = findChild< QDoubleSpinBox * >("doubleSpinBoxMA") ) {
        spin->setValue( method.minimumArea() );
    }
    if ( auto spin = findChild< QDoubleSpinBox * >("doubleSpinBoxT0") ) {
        spin->setValue( method.t0() );
    }
    if ( auto combo = findChild< QComboBox * >("comboBoxPharmacopoeia") ) {
        combo->setCurrentIndex( int( method.pharmacopoeia() ) );
    }

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
    // method.slope( ui->doubleSpinBox->value() );
    // method.minimumWidth( ui->doubleSpinBox_2->value() );
    // method.minimumHeight( ui->doubleSpinBox_3->value() );
    // method.drift( ui->doubleSpinBox_4->value() );
    // method.minimumArea( ui->doubleSpinBox_5->value() );
    // method.doubleWidthTime( 0.0 );
    // method.t0( ui->doubleSpinBox_6->value() );
    // int value = ui->comboBox->currentIndex();
    // method.pharmacopoeia( static_cast< adcontrols::chromatography::ePharmacopoeia >( value ) );

    if ( auto spin = findChild< QDoubleSpinBox * >("doubleSpinBoxSlope") ) {
        method.slope( spin->value() );
    }
    if ( auto spin = findChild< QDoubleSpinBox * >("doubleSpinBoxMW") ) {
        method.minimumWidth( spin->value() );
    }
    if ( auto spin = findChild< QDoubleSpinBox * >("doubleSpinBoxMH") ) {
        method.minimumHeight( spin->value() );
    }
    if ( auto spin = findChild< QDoubleSpinBox * >("doubleSpinBoxDrift") ) {
        method.drift( spin->value() );
    }
    if ( auto spin = findChild< QDoubleSpinBox * >("doubleSpinBoxMA") ) {
        method.minimumArea( spin->value() );
    }
    if ( auto spin = findChild< QDoubleSpinBox * >("doubleSpinBoxT0") ) {
        method.t0( spin->value() );
    }
    if ( auto combo = findChild< QComboBox * >("comboBoxPharmacopoeia") ) {
        method.pharmacopoeia( static_cast< adcontrols::chromatography::ePharmacopoeia >( combo->currentIndex() ) );
    }

    switch( method.pharmacopoeia() ) {
    case ePHARMACOPOEIA_USP:
        method.theoreticalPlateMethod( ePeakWidth_Tangent );
        break;
    case ePHARMACOPOEIA_EP:
    case ePHARMACOPOEIA_JP:
        method.theoreticalPlateMethod( ePeakWidth_HalfHeight );
        method.peakWidthMethod( ePeakWidth_HalfHeight );
        break;
    case ePHARMACOPOEIA_NotSpcified:
        break;
    }

    QStandardItemModel& model = *impl_->model_;

    method.erase( method.begin(), method.end() );

    for ( int row = 0; row < model.rowCount(); ++row ) {

        double seconds = model.data( model.index( row, c_time ) ).toDouble();
        adcontrols::chromatography::ePeakEvent func
            = static_cast< adcontrols::chromatography::ePeakEvent >( model.data( model.index( row, c_function ) ).toInt() );
        if ( func != ePeakEvent_Nothing ) {
            const QVariant value = model.data( model.index( row, c_event_value ) );
            adcontrols::PeakMethod::TimedEvent e( seconds, func );
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
