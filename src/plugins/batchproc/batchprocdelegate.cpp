/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "batchprocdelegate.hpp"
#include <QPainter>
#include <QApplication>
#include <QMenu>
#include <QComboBox>

using namespace batchproc;

BatchprocDelegate::BatchprocDelegate(QObject *parent) :  QItemDelegate(parent)
{
}

void
BatchprocDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const 
{
    if ( index.column() == Constants::c_batchproc_progress ) {
        int progress = index.data().toInt();
        
        QStyleOptionProgressBar progressBarOption;
        progressBarOption.rect = QRect( option.rect.x(), option.rect.y() + (option.rect.height() - 12) / 2, option.rect.width(), 12 );
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.progress = progress;
        progressBarOption.text = QString::number(progress) + "%";
        progressBarOption.textVisible = true;
        progressBarOption.textAlignment = Qt::AlignCenter;
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);

    } else if ( index.column() == Constants::c_batchproc_process ) {
        if ( index.data().canConvert< process >() )
            drawDisplay( painter, option, option.rect, index.data().value<process>().display_name() );
    } else {
        QItemDelegate::paint( painter, option, index );
    }
}

QWidget *
BatchprocDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    if ( index.column() == Constants::c_batchproc_process ) {
        QComboBox * cbx = new QComboBox( parent );
        cbx->addItem( "None" );
        cbx->addItem( "Import" );
        cbx->setCurrentIndex( index.data().value< process >().kind() );
        return cbx;
    }
    return QItemDelegate::createEditor( parent, option, index );
}

void
BatchprocDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if ( index.data().canConvert< process >() ) {
        if ( QComboBox * cbx = dynamic_cast< QComboBox * >( editor ) )
            cbx->setCurrentIndex( process_kind( index.data().value< process >().kind() ) );
    } else {
        QItemDelegate::setEditorData( editor, index );
    }
}

void
BatchprocDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if ( index.row() == Constants::c_batchproc_process ) {
        if ( QComboBox * cbx = dynamic_cast< QComboBox * >( editor ) ) {
            auto proc = index.data( Qt::UserRole ).value< process >();
            proc.kind( process_kind( cbx->currentIndex() ) );
            model->setData( index, proc.display_name() );
        }
    } else {
        QItemDelegate::setModelData( editor, model, index );
    }
}

bool
BatchprocDelegate::editorEvent( QEvent * event
                                , QAbstractItemModel * model
                                , const QStyleOptionViewItem& option
                                , const QModelIndex& index )
{
    bool res = QItemDelegate::editorEvent( event, model, option, index );
    if ( event->type() == QEvent::MouseButtonRelease && model->flags(index) & Qt::ItemIsUserCheckable ) {
        emit stateChanged( index );
    }
    return res;
}
