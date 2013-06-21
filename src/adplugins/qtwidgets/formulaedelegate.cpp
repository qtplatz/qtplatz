/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "formulaedelegate.hpp"

#include <QComboBox>
#include <QMetaType>
#include <QPainter>
#include <QPalette>

using namespace qtwidgets;

FormulaeDelegate::FormulaeDelegate(QObject *parent) : QItemDelegate(parent)
{
}

QWidget *
FormulaeDelegate::createEditor(QWidget *parent
                                 , const QStyleOptionViewItem &option
                                 , const QModelIndex &index) const
{
    return QItemDelegate::createEditor( parent, option, index );
}

void
FormulaeDelegate::paint(QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    QItemDelegate::paint( painter, option, index );
}

void
FormulaeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QItemDelegate::setEditorData( editor, index );
}

void
FormulaeDelegate::setModelData( QWidget *editor
                                  , QAbstractItemModel *model
                                  , const QModelIndex &index) const
{
    // if ( index.data().canConvert< MSReferences >() ) {
    //     QComboBox * p = dynamic_cast< QComboBox * >( editor );
    //     model->setData( index, qVariantFromValue( MSReferences( qtwrapper::wstring( p->currentText() ) ) ) );
    //     emit signalMSReferencesChanged( index );
    // } else {
    QItemDelegate::setModelData( editor, model, index );
}

void
FormulaeDelegate::updateEditorGeometry(QWidget *editor
                                       , const QStyleOptionViewItem &option
                                       , const QModelIndex &index) const
{
    Q_UNUSED( index );
    editor->setGeometry( option.rect );
}

