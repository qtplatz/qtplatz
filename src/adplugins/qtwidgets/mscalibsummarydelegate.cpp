// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

using namespace qtwidgets;

MSCalibSummaryDelegate::MSCalibSummaryDelegate(QObject *parent) :
    QItemDelegate(parent)
{
}

QWidget *
MSCalibSummaryDelegate::createEditor(QWidget *parent
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
MSCalibSummaryDelegate::paint(QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
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
MSCalibSummaryDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
/*
    if ( qVariantCanConvert< MSReferences >( index.data() ) ) {
        QComboBox * p = dynamic_cast< QComboBox * >( editor );
        MSReferences m = qVariantValue< MSReferences >( index.data() );
        std::wstring refname = qtwrapper::wstring( p->currentText() );
        // const adcontrols::MSReferenceDefns& defns = this->refDefns_[ refname ];
        m.setCurrentValue( refname );
    } else {
        QItemDelegate::setEditorData( editor, index );
    }
*/
    QItemDelegate::setEditorData( editor, index );
}

void
MSCalibSummaryDelegate::setModelData( QWidget *editor
                                  , QAbstractItemModel *model
                                  , const QModelIndex &index) const
{
/*
    if ( qVariantCanConvert< MSReferences >( index.data() ) ) {
        QComboBox * p = dynamic_cast< QComboBox * >( editor );
        model->setData( index, qVariantFromValue( MSReferences( qtwrapper::wstring( p->currentText() ) ) ) );
        emit signalMSReferencesChanged( index );
    } else {
        QItemDelegate::setModelData( editor, model, index );
    }
*/
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

