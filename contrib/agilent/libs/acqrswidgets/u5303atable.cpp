/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <qtwrapper/font.hpp>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QHeaderView>
#include <QTextDocument>
#include <QComboBox>
#include <QSignalBlocker>
#include <QSpinBox>

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
           , nsa
    };
    
    class u5303ATable::MyDelegate : public QStyledItemDelegate {

        u5303ATable * pThis_;

    public:
        MyDelegate( u5303ATable * p ) : pThis_( p ) {}
        
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
            } else {
                return QStyledItemDelegate::setModelData( editor, model, index );
            }
            
        }

        QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
            if ( index.row() == r_sampling_rate ) {
                auto combo = new QComboBox( parent );
                combo->addItems( QStringList() << "3.2GS/s" << "1.0GS/s" );
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
{
    setModel( model_ );
	setItemDelegate( new MyDelegate( this ) );
    QFont font;
    setFont( qtwrapper::font::setFamily( font, qtwrapper::fontTableBody ) );
}

void
u5303ATable::onInitialUpdate()
{
    QStandardItemModel& model = *model_;

    model.setColumnCount( 3 );
    model.setRowCount( 8 );
    
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
    model.setData( model.index( row, 0 ), "nsa" );
    model.setData( model.index( row, 1 ), m.nsa );
    model.setData( model.index( row, 2 ), "bit[31]->enable, bit[11:0]->threshold" );

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
    model.setData( model.index( row, 1 ), m.nsa );

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
    ++row;
    m.nsa = model.index( row, 1 ).data().toInt();

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
    }
}

