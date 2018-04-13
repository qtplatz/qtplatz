/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include "u5303atable.hpp"
#include "constants.hpp"
#include <acqrscontrols/u5303a/method.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <qtwrapper/font.hpp>
#include <QBrush>
#include <QColor>
#include <QComboBox>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTextDocument>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QDebug>
#include <limits>

namespace acqrswidgets {

    enum { c_item_name, c_item_value, c_description };

    enum { r_front_end_range
           , r_front_end_offset
           , r_sampling_rate
           , r_ext_trigger_level
           , number_of_samples
           , number_of_average
           , delay_to_first_sample
           , invert_signal
           , pkd_raising_delta
           , pkd_falling_delta
           , pkd_amplitude_accumulation_enabled
           , nsa_enabled
           , nsa_threshold
           , number_of_rows
    };

    class u5303ATable::MyDelegate : public QStyledItemDelegate {

        u5303ATable * pThis_;

    public:
        MyDelegate( u5303ATable * p ) : pThis_( p ) {}

        void setDescription( QAbstractItemModel * model, const QModelIndex& index ) const {
            double range = model->index( r_front_end_range, 1 ).data().toDouble();
            int delta = index.data().toInt();
            model->setData( model->index( index.row(), 2 ), QString("PKD (%1mV)").arg( 1000 * delta * ( range / 8191 ) ) );
        }
        
        void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
            return QStyledItemDelegate::paint( painter, option, index );
        }
        
        void setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex& index ) const override {
            if ( index.row() == r_sampling_rate ) {
                if ( auto combo = qobject_cast<QComboBox *>( editor ) ) {
                    int idx = combo->currentIndex();
                    double value = ( idx == 0 ) ? 3.2e9 : 1.0e9;
                    model->setData( index, value, Qt::EditRole );
                    emit pThis_->valueChanged( idU5303ASampRate, 0, value );
                }
            } else if ( index.row() == r_front_end_range ) {
                if ( auto combo = qobject_cast<QComboBox *>( editor ) ) {
                    int idx = combo->currentIndex();
                    double value = ( idx == 0 ) ? 1.0 : 2.0;
                    model->setData( index, value, Qt::EditRole );
                    setDescription( model, model->index( pkd_raising_delta, 1 ) );
                    setDescription( model, model->index( pkd_falling_delta, 1 ) );
                }

            } else if ( index.row() == number_of_average ) {
                if ( auto spin = qobject_cast<QSpinBox *>( editor ) ) {
                    uint32_t value = spin->value();
                    if ( value >= 8 && value % 8 )
                        value &= ~07;
                    model->setData( index, value & ~07 );
                    emit pThis_->valueChanged( idNbrAverages, 0, value );
                }
            } else if ( index.row() == number_of_samples ) {
                QStyledItemDelegate::setModelData( editor, model, index );
                emit pThis_->valueChanged( idU5303ANbrSamples, 0, index.data( Qt::EditRole ).toInt() );
            } else if ( index.row() == pkd_raising_delta || index.row() == pkd_falling_delta ) {
                QStyledItemDelegate::setModelData( editor, model, index );
                setDescription( model, model->index( pkd_raising_delta, 1 ) );
                setDescription( model, model->index( pkd_falling_delta, 1 ) );
            } else {
                return QStyledItemDelegate::setModelData( editor, model, index );
            }
            
        }

        QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
            if ( index.row() == r_sampling_rate ) {
                auto combo = new QComboBox( parent );
                combo->addItems( QStringList() << "3.2GS/s" << "1.0GS/s" );
                if ( index.data().toDouble() <= ( 1.0e9 + std::numeric_limits< double >::epsilon() ) )
                    combo->setCurrentIndex( 1 );
                return combo;
            } else if ( index.row() == r_front_end_range ) {
                auto combo = new QComboBox( parent );
                combo->addItems( QStringList() << "1V" << "2V" );
                if ( index.data().toDouble() >= ( 2.0 - std::numeric_limits< double >::epsilon() ) )
                    combo->setCurrentIndex( 1 );                
                return combo;                
            } else {
                return QStyledItemDelegate::createEditor( parent, option, index );
            }
        }
        
    };

}

using namespace acqrswidgets;

u5303ATable::~u5303ATable()
{
    delete model_;
}

u5303ATable::u5303ATable(QWidget *parent) : adwidgets::TableView(parent)
    , model_( new QStandardItemModel )
    , pkd_enabled_( false )
{
    setModel( model_ );
	setItemDelegate( new MyDelegate( this ) );
    model_->setColumnCount( 3 );
    model_->setRowCount( number_of_rows );
}

void
u5303ATable::onInitialUpdate()
{
    QStandardItemModel& model = *model_;

    model.setColumnCount( 3 );
    model.setRowCount( number_of_rows );
    
    model.setHeaderData( 0, Qt::Horizontal, QObject::tr( "parameter" ) );
    model.setHeaderData( 1, Qt::Horizontal, QObject::tr( "value" ) );
    model.setHeaderData( 2, Qt::Horizontal, QObject::tr( "description" ) );
    // this->setColumnHidden( 3, true );

    acqrscontrols::u5303a::device_method m; // constructor default method for initial display
    int row = 0;
    model.setData( model.index( row, 0 ), "front end range" );
    model.setData( model.index( row, 1 ), m.front_end_range );
    model.setData( model.index( row, 2 ), "1V/2V range" );
    ++row;
    model.setData( model.index( row, 0 ), "front end offset" );
    model.setData( model.index( row, 1 ), m.front_end_offset );
    model.setData( model.index( row, 2 ), "[-0.5V,0.5V], [-1V,1V] offset" );
    ++row;
    model.setData( model.index( row, 0 ), "sampling rate (Hz)" );
    model.setData( model.index( row, 1 ), m.samp_rate );
    model.setData( model.index( row, 2 ), "sampling rate (1.0GS/s or 3.2GS/s)" );
    ++row;
    model.setData( model.index( row, 0 ), "ext. trigger level (V)" );
    model.setData( model.index( row, 1 ), m.ext_trigger_level );
    model.setData( model.index( row, 2 ), "external trigger threshold (+/- 5V FS)" );
    ++row;
    model.setData( model.index( row, 0 ), "number of samples" );
    model.setData( model.index( row, 1 ), uint32_t( m.nbr_of_s_to_acquire_ ) );
    model.setData( model.index( row, 2 ), "number of sample points in a spectrum" );
    ++row;
    model.setData( model.index( row, 0 ), "number of average" );
    model.setData( model.index( row, 1 ), m.nbr_of_averages );
    model.setData( model.index( row, 2 ), "number of averages [1,2,3,4,5,6,7,8,16,24,32..520000]." );
    ++row;
    model.setData( model.index( row, 0 ), "delay to first sample" );
    model.setData( model.index( row, 1 ), m.delay_to_first_sample_ );
    model.setData( model.index( row, 2 ), "delay to first sample (seconds)" );
    ++row;
    model.setData( model.index( row, 0 ), "invert signal" );
    model.setData( model.index( row, 1 ), m.invert_signal ? true : false );
    model.setData( model.index( row, 2 ), "inversion" );
    ++row;
    model.setData( model.index( row, 0 ), "Rising Delta" );
    model.setData( model.index( row, 1 ), m.pkd_raising_delta );
    model.setData( model.index( row, 2 ), "PKD" );
    ++row;
    model.setData( model.index( row, 0 ), "Falling Delta" );
    model.setData( model.index( row, 1 ), m.pkd_falling_delta );
    model.setData( model.index( row, 2 ), "PKD" );
    ++row;
    model.setData( model.index( row, 0 ), "Amplitude accumulation" );
    model.setData( model.index( row, 1 ), m.pkd_amplitude_accumulation_enabled ? true : false );
    model.setData( model.index( row, 2 ), "PKD" );
    ++row;
    model.setData( model.index( row, 0 ), "nsa" );
    model.setData( model.index( row, 1 ), m.nsa_enabled ? true : false );
    model.setData( model.index( row, 2 ), "bit[31]->enable, bit[11:0]->threshold" );
    ++row;
    model.setData( model.index( row, 0 ), "nsa threshold" );
    model.setData( model.index( row, 1 ), m.nsa_threshold );
    model.setData( model.index( row, 2 ), "bit[31]->enable, bit[11:0]->threshold" );
    ++row;

    resizeColumnsToContents();
	resizeRowsToContents();

    for ( int row = 0; row < model.rowCount(); ++row ) {
        model.item( row, c_item_name )->setEditable( false );
        model.item( row, c_item_value )->setEditable( true );
        model.item( row, c_description )->setEditable( false );
    }

    connect( model_, &QStandardItemModel::dataChanged, [this] ( const QModelIndex& topLeft, const QModelIndex& bottomRight ) {
            if ( !this->signalsBlocked() ) {
                for ( int row = topLeft.row(); row <= bottomRight.row(); ++row )
                    model_->itemFromIndex( model_->index( row, c_item_value ) )->setBackground( QColor( Qt::yellow ) );
            }
        });
}

bool
u5303ATable::setContents( const acqrscontrols::u5303a::device_method& m )
{
    QStandardItemModel& model = *model_;

    QSignalBlocker block( this );

    pkd_enabled_ = m.pkd_enabled;

    int row = 0;
    model.setData( model.index( row, 1 ), m.front_end_range );
    ++row;
    model.setData( model.index( row, 1 ), m.front_end_offset );
    ++row;
    model.setData( model.index( row, 1 ), m.samp_rate );
    ++row;
    model.setData( model.index( row, 1 ), m.ext_trigger_level );
    ++row;
    model.setData( model.index( row, 1 ), m.nbr_of_s_to_acquire_ );
    ++row;
    model.setData( model.index( row, 1 ), m.nbr_of_averages );
    ++row;
    model.setData( model.index( row, 1 ), m.delay_to_first_sample_ ); // seconds
    ++row;
    model.setData( model.index( row, 1 ), m.invert_signal ? true : false );
    ++row;
    model.setData( model.index( row, 1 ), m.pkd_raising_delta );
    ++row;
    model.setData( model.index( row, 1 ), m.pkd_falling_delta );
    ++row;
    model.setData( model.index( row, 1 ), m.pkd_amplitude_accumulation_enabled );        
    ++row;
    model.setData( model.index( row, 1 ), m.nsa_enabled );
    ++row;
    model.setData( model.index( row, 1 ), m.nsa_threshold );

    for ( int row = pkd_raising_delta; row <= pkd_amplitude_accumulation_enabled; ++row ) {
        if ( auto item = model.item( row, 0 ) )
            item->setForeground( m.pkd_enabled ? QColor( Qt::black ) : QColor( Qt::gray ) );
    }

	return true;
}

bool
u5303ATable::getContents( acqrscontrols::u5303a::device_method& m )
{
    QStandardItemModel& model = *model_;

    int row = 0;
	m.front_end_range = model.index( row, 1 ).data().toDouble();
    ++row;
	m.front_end_offset = model.index( row, 1 ).data().toDouble();
    ++row;
	m.samp_rate = model.index( row, 1 ).data().toDouble();
	++row;
	m.ext_trigger_level = model.index( row, 1 ).data().toDouble();
    ++row;
	m.nbr_of_s_to_acquire_ = model.index( row, 1 ).data().toInt();
    ++row;
	m.nbr_of_averages = model.index( row, 1 ).data().toInt();
    ++row;
    m.delay_to_first_sample_ = model.index( row, 1 ).data().toDouble(); // seconds
    ++row;
    m.invert_signal = model.index( row, 1 ).data().toBool() ? 1 : 0;

    // PKD
    ++row;
    m.pkd_raising_delta = model.index( row, 1 ).data().toInt();
    ++row;
    m.pkd_falling_delta = model.index( row, 1 ).data().toInt();
    ++row;
    m.pkd_amplitude_accumulation_enabled = model.index( row, 1 ).data().toBool();
    ++row;

    // NSA
    m.nsa_enabled = model.index( row, 1 ).data().toBool();
    ++row;
    m.nsa_threshold = model.index( row, 1 ).data().toInt();

    // digitizer_<...> will be override -- due to u5303a require self calibration for following two parameter change.
    // It takes one to two seconds so that is not acceptable for multi-turn protocol acquisition.
    // Application layer will take a look overall 'protocol' parameer and chosse widest range put on digitizer_<...>
    // parameters;  Later, it will be trimed into desiered range.
    m.digitizer_delay_to_first_sample = m.delay_to_first_sample_;  
    m.digitizer_nbr_of_s_to_acquire = m.nbr_of_s_to_acquire_;

    QSignalBlocker blocks[] = { QSignalBlocker( model_ ), QSignalBlocker( this ) };

    for ( int row = 0; row < model.rowCount(); ++row )
		model.itemFromIndex( model.index( row, c_item_value ) )->setBackground( QColor( Qt::white ) );

	return true;
}

void
u5303ATable::onHandleValue( idCategory id, int channel, const QVariant& value )
{
    QSignalBlocker block( this );

    switch ( id ) {
    case idU5303AStartDelay:
        if ( model_ ) {
            model_->setData( model_->index( delay_to_first_sample, c_item_value ), value );
        }
        break;
    case idU5303AWidth:
        if ( model_ ) {
            double rate = model_->index( r_sampling_rate, c_item_value ).data( Qt::EditRole ).toDouble();
            int nSamples = value.toDouble() * rate + 0.5;
            model_->setData( model_->index( number_of_samples, c_item_value ), nSamples );
        }
        break;
    case idNbrAverages:
        if ( model_ ) {
            model_->setData( model_->index( number_of_average, c_item_value ), value );
        }
        break;
    case idPKDEnable:
        if ( model_ ) {
            pkd_enabled_ = value.toBool();
            auto rate = model_->index( r_sampling_rate, c_item_value ).data( Qt::EditRole ).toDouble();
            double next_rate(0);

            if ( pkd_enabled_ && adportable::compare< double >::essentiallyEqual( 3.2e+9, rate ) )
                next_rate = 1.6e+9;
            else if ( !pkd_enabled_ && adportable::compare< double >::essentiallyEqual( 1.6e+9, rate ) )
                next_rate = 3.2e+9;

            if ( adportable::compare< double >::definitelyGreaterThan( next_rate, 0 ) ) {
                model_->setData( model_->index( r_sampling_rate, c_item_value ), next_rate );
                double width = model_->index( number_of_samples, c_item_value ).data( Qt::EditRole).toInt() / rate;
                model_->setData( model_->index( number_of_samples, c_item_value ), int( width * next_rate + 0.5 ) );
            }

            for ( int row = pkd_raising_delta; row <= pkd_amplitude_accumulation_enabled; ++row )
                model_->item( row, 0 )->setForeground( value.toBool() ? QColor( Qt::black ) : QColor( Qt::gray ) );
        }
        break;
    }
}

void
u5303ATable::setEnabled( const QString& name, bool enable )
{
    if ( name == "StartDelay" ) {
        if ( auto item = model_->item( delay_to_first_sample, c_item_value ) )
            item->setEditable( enable );
    } else if ( name == "Width" ) {
        if ( auto item = model_->item( number_of_samples, c_item_value ) )
            item->setEditable( enable );
    }
}
