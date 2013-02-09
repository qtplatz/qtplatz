/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "isotopedelegate.hpp"

using namespace qtwidgets;

IsotopeDelegate::IsotopeDelegate(QObject *parent) :
    QItemDelegate(parent)
{
}

QWidget *
IsotopeDelegate::createEditor(QWidget *parent
                                 , const QStyleOptionViewItem &option
                                 , const QModelIndex &index) const
{
/*
    if ( qVariantCanConvert< MSReferences >( index.data() ) ) {
        QComboBox * pCombo = new QComboBox( parent );
        QStringList list;
        for ( refs_type::const_iterator it = refs_.begin(); it != refs_.end(); ++it )
            list << qtwrapper::qstring( it->first );
        pCombo->addItems( list );
        return pCombo;
    } else {
        return QItemDelegate::createEditor( parent, option, index );
    }
*/
    return QItemDelegate::createEditor( parent, option, index );
}

void
IsotopeDelegate::paint(QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
/*
    if ( qVariantCanConvert< MSReferences >( index.data() ) ) {
        MSReferences m = qVariantValue< MSReferences >( index.data() );
        drawDisplay( painter, option, option.rect, m.displayValue() );
    } else {
        QItemDelegate::paint( painter, option, index );
    }
*/
    QItemDelegate::paint( painter, option, index );
}

void
IsotopeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QItemDelegate::setEditorData( editor, index );
}

void
IsotopeDelegate::setModelData( QWidget *editor
                                  , QAbstractItemModel *model
                                  , const QModelIndex &index) const
{
    QItemDelegate::setModelData( editor, model, index );
}

void
IsotopeDelegate::updateEditorGeometry(QWidget *editor
                                       , const QStyleOptionViewItem &option
                                       , const QModelIndex &index) const
{
    Q_UNUSED( index );
    editor->setGeometry( option.rect );
}
