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

#include "ap240methodtable.hpp"
#include <ap240/digitizer.hpp>
#include <qtwrapper/font.hpp>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QHeaderView>
#include <QTextDocument>
#include <QComboBox>

namespace ap240 {

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
    
    class ap240MethodDelegate : public QStyledItemDelegate {
        // Q_OBJECT
    public:
        void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
            return QStyledItemDelegate::paint( painter, option, index );
        }
        void setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex& index ) const override {
            if ( index.row() == r_sampling_rate ) {
                if ( auto combo = qobject_cast< QComboBox * >( editor ) ) {
                    int idx = combo->currentIndex();
                    double value = ( idx == 0 ) ? 3.2e9 : 1.0e9;
                    model->setData( index, value, Qt::EditRole );
                }
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

using namespace ap240;

ap240MethodTable::~ap240MethodTable()
{
    delete model_;
}

ap240MethodTable::ap240MethodTable(QWidget *parent) : adwidgets::TableView(parent)
                                                      , model_( new QStandardItemModel )
                                                      , in_progress_( false )
{
    setModel( model_ );
	setItemDelegate( new ap240MethodDelegate );
    QFont font;
    setFont( qtwrapper::font::setFamily( font, qtwrapper::fontTableBody ) );
}

void
ap240MethodTable::handleDataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight )
{
    if ( in_progress_ )
        return;

    QStandardItemModel& model = *model_;

    for ( int row = topLeft.row(); row <= bottomRight.row(); ++row )
		model.itemFromIndex( model.index( row, c_item_value ) )->setBackground( QColor( Qt::yellow ) );
}
    
void
ap240MethodTable::onInitialUpdate()
{
    QStandardItemModel& model = *model_;

    model.setColumnCount( 3 );
    model.setRowCount( 8 );
    
    model.setHeaderData( 0, Qt::Horizontal, QObject::tr( "parameter" ) );
    model.setHeaderData( 1, Qt::Horizontal, QObject::tr( "value" ) );
    model.setHeaderData( 2, Qt::Horizontal, QObject::tr( "description" ) );
    // this->setColumnHidden( 3, true );

    ap240::method m; // constructor default method for initial display
    int row = 0;
    model.setData( model.index( row, 0 ), "front end range" );
    model.setData( model.index( row, 1 ), m.front_end_range );
    model.setData( model.index( row, 2 ), "1V/2V range" );
    ++row;
    model.setData( model.index( row, 0 ), "front end offset" );
    model.setData( model.index( row, 1 ), m.front_end_offset );
    model.setData( model.index( row, 2 ), "[-0.5V,0.5V], [-1V,1V] offset" );
    ++row;
    model.setData( model.index( row, 0 ), "sampling rate" );
    model.setData( model.index( row, 1 ), m.samp_rate );
    model.setData( model.index( row, 2 ), "sampling rate (1.0GS/s or 3.2GS/s)" );
    ++row;
    model.setData( model.index( row, 0 ), "ext. trigger level" );
    model.setData( model.index( row, 1 ), m.ext_trigger_level );
    model.setData( model.index( row, 2 ), "external trigger threshold" );
    ++row;
    model.setData( model.index( row, 0 ), "number of samples" );
    model.setData( model.index( row, 1 ), uint32_t( m.nbr_of_s_to_acquire_ ) );
    model.setData( model.index( row, 2 ), "number of sample points in a spectrum" );
    ++row;
    model.setData( model.index( row, 0 ), "number of average" );
    model.setData( model.index( row, 1 ), m.nbr_of_averages );
    model.setData( model.index( row, 2 ), "number of averages ninus one." );
    ++row;
    model.setData( model.index( row, 0 ), "delay to first sample" );
    model.setData( model.index( row, 1 ), m.delay_to_first_sample_ );
    model.setData( model.index( row, 2 ), "delay to first sample" );
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

	connect( model_, SIGNAL( dataChanged(const QModelIndex&, const QModelIndex& ) )
             , this, SLOT( handleDataChanged( const QModelIndex&, const QModelIndex& ) ) );
}

bool
ap240MethodTable::setContents( const ap240::method& m )
{
    QStandardItemModel& model = *model_;
    
    in_progress_ = true;

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
    model.setData( model.index( row, 1 ), m.delay_to_first_sample_ * 1.0e6 ); // s -> us
    ++row;
    model.setData( model.index( row, 1 ), m.invert_signal ? true : false );
    ++row;
    model.setData( model.index( row, 1 ), m.nsa );

    in_progress_ = false;
	
	return true;
}

bool
ap240MethodTable::getContents( ap240::method& m )
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
    m.delay_to_first_sample_ = model.index( row, 1 ).data().toDouble() * 1.0e-6; // us -> s
    ++row;
    m.invert_signal = model.index( row, 1 ).data().toBool() ? 1 : 0;
    ++row;
    m.nsa = model.index( row, 1 ).data().toInt();

    in_progress_ = true;
    for ( int row = 0; row < model.rowCount(); ++row )
		model.itemFromIndex( model.index( row, c_item_value ) )->setBackground( QColor( Qt::white ) );
    in_progress_ = false;
	return true;
}
