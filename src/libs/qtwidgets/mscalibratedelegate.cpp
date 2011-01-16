/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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

#include "mscalibratedelegate.h"
#include <qtwrapper/qstring.h>
#include <QComboBox>
#include <QMetaType>
#include <QPainter>
#include <QPalette>

using namespace qtwidgets;

MSCalibrateDelegate::MSReferences::MSReferences() : curSel_(0)
{
    list_.push_back( L"Xe" );
}

QString
MSCalibrateDelegate::MSReferences::displayValue() const
{
    return qtwrapper::qstring::copy( list_[ curSel_ ] );
}

const std::wstring&
MSCalibrateDelegate::MSReferences::methodValue() const
{
    return list_[ curSel_ ];
}

///////////////////////

MSCalibrateDelegate::MSCalibrateDelegate(QObject *parent) :
    QItemDelegate(parent)
{
}

QWidget *
MSCalibrateDelegate::createEditor(QWidget *parent
                                 , const QStyleOptionViewItem &option
                                 , const QModelIndex &index) const
{
    if ( qVariantCanConvert< MSReferences >( index.data() ) ) {
        QComboBox * pCombo = new QComboBox( parent );
        QStringList list;
        list << "Xe" << "PFTBA";
        pCombo->addItems( list );
        return pCombo;
    } else {
        return QItemDelegate::createEditor( parent, option, index );
    }
}

void
MSCalibrateDelegate::paint(QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    if ( qVariantCanConvert< MSReferences >( index.data() ) ) {
        MSReferences m = qVariantValue< MSReferences >( index.data() );
        drawDisplay( painter, option, option.rect, m.displayValue() );
    } else {
        QItemDelegate::paint( painter, option, index );
    }
}

void
MSCalibrateDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if ( qVariantCanConvert< MSReferences >( index.data() ) ) {
        QComboBox * p = dynamic_cast< QComboBox * >( editor );
        MSReferences m = qVariantValue< MSReferences >( index.data() );    
        p->setCurrentIndex( 0 ); //m.methodValue() );
    } else {
        QItemDelegate::setEditorData( editor, index );
    }
}

void
MSCalibrateDelegate::setModelData( QWidget *editor
                                  , QAbstractItemModel *model
                                  , const QModelIndex &index) const
{
    if ( qVariantCanConvert< MSReferences >( index.data() ) ) {
        QComboBox * p = dynamic_cast< QComboBox * >( editor );
/*
        adcontrols::CentroidMethod::ePeakWidthMethod value = 
            static_cast<adcontrols::CentroidMethod::ePeakWidthMethod>( p->currentIndex() );
        model->setData( index, qVariantFromValue( PeakWidthMethod( value ) ) );
*/
    } else {
        QItemDelegate::setModelData( editor, model, index );
    }
}

void
MSCalibrateDelegate::updateEditorGeometry(QWidget *editor
                                       , const QStyleOptionViewItem &option
                                       , const QModelIndex &index) const
{
    Q_UNUSED( index );
    editor->setGeometry( option.rect );
}
