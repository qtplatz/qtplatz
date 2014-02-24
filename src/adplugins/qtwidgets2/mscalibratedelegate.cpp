/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
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

#include "mscalibratedelegate.hpp"
#include "mscalibrationform.hpp"
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msreference.hpp>
#include <qtwrapper/qstring.hpp>
#include <QComboBox>
#include <QMetaType>
#include <QPainter>
#include <QPalette>
#include <QEvent>

using namespace qtwidgets2;

MSCalibrateDelegate::MSCalibrateDelegate(QObject *parent) : QItemDelegate(parent)
{
}

bool
MSCalibrateDelegate::editorEvent( QEvent * event
                                  , QAbstractItemModel * model
                                  , const QStyleOptionViewItem& option
                                  , const QModelIndex& index )
{
    if ( event->type() == QEvent::MouseButtonRelease && model->flags(index) & Qt::ItemIsUserCheckable ) {
        // here also can handle checkbox as same code in treeview.cpp
    }
    return QItemDelegate::editorEvent( event, model, option, index );
}

void
MSCalibrateDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	QItemDelegate::setModelData( editor, model, index );
	emit valueChanged( index );
}
