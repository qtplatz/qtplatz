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

#include "mscalibsummarydelegate.hpp"

using namespace qtwidgets2;

MSCalibSummaryDelegate::MSCalibSummaryDelegate(QObject *parent) : QItemDelegate(parent)
{
}

QWidget *
MSCalibSummaryDelegate::createEditor(QWidget *parent
                                     , const QStyleOptionViewItem &option
                                     , const QModelIndex &index) const
{
    return QItemDelegate::createEditor( parent, option, index );
}

void
MSCalibSummaryDelegate::paint(QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    QItemDelegate::paint( painter, option, index );
}

void
MSCalibSummaryDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QItemDelegate::setEditorData( editor, index );
}

void
MSCalibSummaryDelegate::setModelData( QWidget *editor
                                  , QAbstractItemModel *model
                                  , const QModelIndex &index) const
{
    QItemDelegate::setModelData( editor, model, index );
    emit valueChanged( index );
}

void
MSCalibSummaryDelegate::updateEditorGeometry(QWidget *editor
                                       , const QStyleOptionViewItem &option
                                       , const QModelIndex &index) const
{
    Q_UNUSED( index );
    editor->setGeometry( option.rect );
}

