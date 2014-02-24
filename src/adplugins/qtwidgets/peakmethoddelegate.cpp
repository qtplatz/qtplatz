/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "peakmethoddelegate.hpp"
#include <adcontrols/peakmethod.hpp>
#include <boost/format.hpp>
#include <QComboBox>
#include <QTextDocument>
#include <QPainter>
#include <QKeyEvent>
#include <QEvent>

using namespace qtwidgets;

PeakMethodDelegate::PeakMethodDelegate(QObject *parent) :  QItemDelegate(parent)
{
}

QWidget *
PeakMethodDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if ( index.row() == r_pharmacopoeia ) {
        QComboBox * pCombo = new QComboBox( parent );
        QStringList list;
        list << "Not specified" << "EP" << "JP" << "USP";
        pCombo->addItems( list );
        return pCombo;
    } else {
        return QItemDelegate::createEditor( parent, option, index );
  }
}

void
PeakMethodDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if ( index.column() == c_value ) {
        if ( index.row() == r_pharmacopoeia ) {
            int value = index.data().toInt();
            QString text;
            switch ( value ) {
            case adcontrols::chromatography::ePHARMACOPOEIA_NotSpcified: text = "Not specified"; break;
            case adcontrols::chromatography::ePHARMACOPOEIA_EP:          text = "EP"; break;
            case adcontrols::chromatography::ePHARMACOPOEIA_JP:          text = "JP"; break;
            case adcontrols::chromatography::ePHARMACOPOEIA_USP:         text = "USP"; break;
            }
            drawDisplay( painter, option, option.rect, text );
        } else {
            drawDisplay( painter, option, option.rect, ( boost::format( "%.3lf" ) % index.data().toDouble() ).str().c_str() );
        }
    } else if ( index.column() == c_header ) {
        QStyleOptionViewItem op = option;
        painter->save();

        QTextDocument doc;
        doc.setHtml( index.data().toString() );
        op.widget->style()->drawControl( QStyle::CE_ItemViewItem, &op, painter );
        painter->translate( op.rect.left(), op.rect.top() );
        QRect clip( 0, 0, op.rect.width(), op.rect.height() );
        doc.drawContents( painter, clip );

        painter->restore();
    } else {
        QItemDelegate::paint( painter, option, index );
    }
}

QSize
PeakMethodDelegate::sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
	if ( index.column() == c_header ) {
		QTextDocument doc;
		doc.setHtml( index.data().toString() );
		return QSize( doc.size().width(), doc.size().height() );
	} else 
		return QItemDelegate::sizeHint( option, index );
}

void
PeakMethodDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QItemDelegate::setEditorData( editor, index );
}

void
PeakMethodDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if ( index.row() == r_pharmacopoeia ) {
        QComboBox * p = dynamic_cast< QComboBox * >( editor );
        int value = p->currentIndex();
        model->setData( index, value );
    } else
        QItemDelegate::setModelData( editor, model, index );
}

bool
PeakMethodDelegate::editorEvent( QEvent * 
                                 , QAbstractItemModel * 
                                 , const QStyleOptionViewItem&
                                 , const QModelIndex& )
{
	return false;
}
