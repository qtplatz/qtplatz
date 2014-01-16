/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "mschromatogramwidget.hpp"
#include <adcontrols/mschromatogrammethod.hpp>
#include <adcontrols/processmethod.hpp>
#include <adportable/is_type.hpp>
#include <boost/any.hpp>
#include <QStandardItemModel>
#include <QItemDelegate>
#include <QTextDocument>
#include <QComboBox>

namespace qtwidgets2 {
    enum {
        r_mschromatogram_datasource
        , r_mschromatogram_width
        , r_mschromatogram_limits
    };
}

using namespace qtwidgets2;

MSChromatogramWidget::MSChromatogramWidget(QWidget *parent) : QTreeView(parent)
                                                            , model_( new QStandardItemModel )
                                                            , delegate_( new MSChromatogramDelegate )
                                                            , method_( new adcontrols::MSChromatogramMethod )
{
    setModel( model_.get() );
    setItemDelegate( delegate_.get() );
    connect( delegate_.get(), SIGNAL( valueChanged( const QModelIndex& ) ), this, SLOT( handleValueChanged( const QModelIndex& ) ) );
}

MSChromatogramWidget::~MSChromatogramWidget()
{
}

void
MSChromatogramWidget::OnCreate( const adportable::Configuration& )
{
}

void
MSChromatogramWidget::OnInitialUpdate()
{
    QStandardItemModel& model = *model_;

    model.setColumnCount( 4 );
    model.setRowCount( 5 );

    model.setData( model.index( r_mschromatogram_datasource, 0 ), "data source" );

    model.setData( model.index( r_mschromatogram_width,      0 ), "m/z width" );
    do { // width
        QStandardItem * parent = model.itemFromIndex( model.index( r_mschromatogram_width, 0 ) );
        parent->setRowCount( 2 );
        parent->setColumnCount( 2 );
		const int rows [] = { 0, 1 };
		for ( auto row: rows ) {
			QStandardItem * chk = model.itemFromIndex( model.index( row, 0 , parent->index() ) );
            //chk->setFlags( chk->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
			chk->setCheckable( true );
			chk->setEnabled( true );
            chk->setEditable( true );
			model.setData( model.index( row, 0, parent->index() ), ( row == 0 ) ? "Dalton" : "R.P." );
            model.setData( model.index( row, 0, parent->index() ), ( row == method_->widthMethod())? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
        }
    } while(0);

    model.setData( model.index( r_mschromatogram_limits,     0 ), "mass range" );
    do { // width
        QStandardItem * parent = model.itemFromIndex( model.index( r_mschromatogram_limits, 0 ) );
        parent->setRowCount( 2 );
        parent->setColumnCount( 2 );
        model.setData( model.index( 0, 0, parent->index() ), "lower" );
        model.setData( model.index( 1, 0, parent->index() ), "upper" );
    } while(0);

    setContents( *method_ );    

    // for ( int c = 0; c < model.columnCount(); ++c )
    //     resizeColumnToContents( c );

    expandAll();
}

void
MSChromatogramWidget::OnFinalClose()
{
}

bool
MSChromatogramWidget::getContents( boost::any& any ) const
{
    if ( adportable::a_type< adcontrols::ProcessMethod >::is_pointer( any ) ) {

        getContents( *method_ );

        if ( adcontrols::ProcessMethod* pm = boost::any_cast< adcontrols::ProcessMethod* >( any ) )
            pm->appendMethod< adcontrols::MSChromatogramMethod >( *method_ );
        
        return true;
    }

    return false;
}

bool
MSChromatogramWidget::setContents( boost::any& any )
{
    if ( adportable::a_type< adcontrols::ProcessMethod >::is_a( any ) ) {

        adcontrols::ProcessMethod& pm = boost::any_cast< adcontrols::ProcessMethod& >( any );
        if ( const adcontrols::MSChromatogramMethod *p = pm.find< adcontrols::MSChromatogramMethod >() ) {
            *method_ = *p;
            setContents( *method_ );
            return true;
        }
    }
    return false;
}

void
MSChromatogramWidget::getLifeCycle( adplugin::LifeCycle *& p )
{
    p = this;
}

void
MSChromatogramWidget::getContents( adcontrols::ProcessMethod& pm )
{
    pm.appendMethod< adcontrols::MSChromatogramMethod >( *method_ );
}

void
MSChromatogramWidget::setContents( const adcontrols::MSChromatogramMethod& m )
{
    QStandardItemModel& model = *model_;

    model.setData( model.index( r_mschromatogram_datasource, 1 ), m.dataSource() );
    model.setData( model.index( r_mschromatogram_width,      1 ), m.widthMethod() );
    model.setData( model.index( r_mschromatogram_width,      2 ), m.width( m.widthMethod() ) );

    static int rows [] = { 0, 1 };
    QStandardItem * parent = model.itemFromIndex( model.index( r_mschromatogram_width, 0 ) );
    for ( auto row: rows )
        model.setData( model.index( row, 1, parent->index() ), m.width( adcontrols::MSChromatogramMethod::WidthMethod( row ) ) );

    // mass range
    // mass_range summary
    model.setData( model.index( r_mschromatogram_limits,     1 ), m.lower_limit() );
    model.setData( model.index( r_mschromatogram_limits,     2 ), m.upper_limit() );
    // mass_range details
    parent = model.itemFromIndex( model.index( r_mschromatogram_limits, 0 ) );
    model.setData( model.index( 0, 1, parent->index() ), m.lower_limit() );
    model.setData( model.index( 1, 1, parent->index() ), m.upper_limit() );
}

void
MSChromatogramWidget::getContents( adcontrols::MSChromatogramMethod& m ) const
{
    m = *method_;
}

void
MSChromatogramWidget::handleValueChanged( const QModelIndex& index ) 
{
	QStandardItemModel& model = *model_;

    // width method
    if ( index.parent() == QModelIndex() && index.row() == r_mschromatogram_width ) {
        // summary (root) level
        if ( index.column() == 1 ) {
            auto value = adcontrols::MSChromatogramMethod::WidthMethod( index.data().toInt() );
            method_->widthMethod( value );
        } else if ( index.column() == 2 ) {
            method_->width( index.data().toDouble(), method_->widthMethod() );            
        }
    }
    
	if ( index.parent() == model.index( r_mschromatogram_width, 0 ) ) {
        // width in detail (sub) level
        if ( index.row() == 0 ) 
            method_->width( index.data().toDouble(), adcontrols::MSChromatogramMethod::widthInDa );
        else
            method_->width( index.data().toDouble(), adcontrols::MSChromatogramMethod::widthInRP );
    }

    // limits in detail level
    if ( index.parent() == model.index( r_mschromatogram_limits, 0 ) ) {
        if ( index.row() == 0 )
			method_->lower_limit( index.data().toDouble() );
        else
			method_->upper_limit( index.data().toDouble() );
    }

    // limits in summary level
    if ( index.parent() == QModelIndex() && index.row() == r_mschromatogram_limits ) {
        if ( index.column() == 1 )
			method_->lower_limit( index.data().toDouble() );
        else
			method_->upper_limit( index.data().toDouble() );
    }

    setContents( *method_ );
}

//////////////////// delegate ////////////////////
MSChromatogramDelegate::MSChromatogramDelegate( QObject *parent ) : QItemDelegate(parent)
{
}

QWidget *
MSChromatogramDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if ( index.parent() == QModelIndex() ) {
        if ( index.row() == r_mschromatogram_datasource && index.column() == 1 ) {
            QComboBox * pCombo = new QComboBox( parent );
            QStringList list;
            list << "Profile" << "Centroid";
            pCombo->addItems( list );
            return pCombo;
        }
		if ( index.row() == r_mschromatogram_width && index.column() == 1 ) {
            QComboBox * pCombo = new QComboBox( parent );
            QStringList list;
            list << "Daltons" << "R.P.";
            pCombo->addItems( list );
            return pCombo;
        }
    }
    return QItemDelegate::createEditor( parent, option, index );
}

void
MSChromatogramDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    using adcontrols::MSChromatogramMethod;

    if ( index.parent() == QModelIndex() ) {
        if ( index.row() == r_mschromatogram_datasource && index.column() == 1 ) {
            const char * text = ( index.data().toInt() == MSChromatogramMethod::Profile ) ? "Profile" : "Centroid";
            drawDisplay( painter, option, option.rect, text );
            return;
        } else if ( index.row() == r_mschromatogram_width && index.column() == 1 ) {
			int value = index.data().toInt();
            const char * text = ( value == MSChromatogramMethod::widthInDa ) ? "Daltons" : "R.P.";
            drawDisplay( painter, option, option.rect, text );
            return;            
        }
    }
    // } else if ( index.column() == c_header ) {
    //     QStyleOptionViewItem op = option;
    //     painter->save();

    //     QTextDocument doc;
    //     doc.setHtml( index.data().toString() );
    //     op.widget->style()->drawControl( QStyle::CE_ItemViewItem, &op, painter );
    //     painter->translate( op.rect.left(), op.rect.top() );
    //     QRect clip( 0, 0, op.rect.width(), op.rect.height() );
    //     doc.drawContents( painter, clip );

    //     painter->restore();
    // } else {
    QItemDelegate::paint( painter, option, index );
}

QSize
MSChromatogramDelegate::sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
	if ( index.column() == 0 ) {
		QTextDocument doc;
		doc.setHtml( index.data().toString() );
		return QSize( doc.size().width(), doc.size().height() );
	} else 
		return QItemDelegate::sizeHint( option, index );
}

void
MSChromatogramDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QItemDelegate::setEditorData( editor, index );
}

void
MSChromatogramDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    using adcontrols::MSChromatogramMethod;
	bool handled( false );

	if ( index.parent() == QModelIndex() ) {
        if ( index.row() == r_mschromatogram_datasource && index.column() == 1 ) {
            QComboBox * p = dynamic_cast< QComboBox * >( editor );
            model->setData( index, p->currentIndex() );
			handled = true;
        }
		if ( index.row() == r_mschromatogram_width && index.column() == 1 ) {
            QComboBox * p = dynamic_cast< QComboBox * >( editor );
            MSChromatogramMethod::WidthMethod value = static_cast< MSChromatogramMethod::WidthMethod >( p->currentIndex() );
            model->setData( index, value );
			handled = true;
		}
    }
	if ( ! handled )
        QItemDelegate::setModelData( editor, model, index );
    emit valueChanged( index );
}

bool
MSChromatogramDelegate::editorEvent( QEvent * 
                                     , QAbstractItemModel * 
                                     , const QStyleOptionViewItem&
                                     , const QModelIndex& )
{
	return false;
}
