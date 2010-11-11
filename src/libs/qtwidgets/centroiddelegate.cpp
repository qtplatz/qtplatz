//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "centroiddelegate.h"
#include <adcontrols/centroidmethod.h>
#include <QComboBox>
#include <QDebug>
#include <QMetaType>
#include <QPainter>
#include <QPalette>

using namespace qtwidgets;

CentroidDelegate::CentroidDelegate(QObject *parent) :  QItemDelegate(parent)
{
}

QWidget *
CentroidDelegate::createEditor(QWidget *parent
                                , const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    if ( qVariantCanConvert< PeakWidthMethod >( index.data() ) ) {
        QComboBox * pCombo = new QComboBox( parent );
        QStringList list;
        list << "TOF" << "Proportional" << "Constant";
        pCombo->addItems( list );
        return pCombo;
    } else if ( qVariantCanConvert< AreaHeight > ( index.data() ) ) {
        QComboBox * pCombo = new QComboBox( parent );
        QStringList list;
        list << "Area" << "Height";
        pCombo->addItems( list );
        return pCombo;
    } else {
        return QItemDelegate::createEditor( parent, option, index );
    }
}

void
CentroidDelegate::paint(QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    if ( qVariantCanConvert< PeakWidthMethod >( index.data() ) ) {
        PeakWidthMethod m = qVariantValue< PeakWidthMethod >( index.data() );
        drawDisplay( painter, option, option.rect, m.displayValue() );
    } else if ( qVariantCanConvert< AreaHeight > ( index.data() ) ) {
        AreaHeight m = qVariantValue< AreaHeight >( index.data() );
        drawDisplay( painter, option, option.rect, m.displayValue() );
    } else {
        QItemDelegate::paint( painter, option, index );
    }
}

void
CentroidDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if ( qVariantCanConvert< PeakWidthMethod >( index.data() ) ) {
        QComboBox * p = dynamic_cast< QComboBox * >( editor );
        PeakWidthMethod m = qVariantValue< PeakWidthMethod >( index.data() );    
        p->setCurrentIndex( m.methodValue() );
    } else if ( qVariantCanConvert< AreaHeight > ( index.data() ) ) {
        QComboBox * p = dynamic_cast< QComboBox * >( editor );
        AreaHeight m = qVariantValue< AreaHeight >( index.data() );    
        p->setCurrentIndex( m.methodValue() ? 0 : 1 );
    } else {
        QItemDelegate::setEditorData( editor, index );
    }
}

void
CentroidDelegate::setModelData( QWidget *editor
                                , QAbstractItemModel *model,
                                const QModelIndex &index) const
{
    if ( qVariantCanConvert< PeakWidthMethod >( index.data() ) ) {
        QComboBox * p = dynamic_cast< QComboBox * >( editor );
        adcontrols::CentroidMethod::ePeakWidthMethod value = 
            static_cast<adcontrols::CentroidMethod::ePeakWidthMethod>( p->currentIndex() );
        model->setData( index, qVariantFromValue( PeakWidthMethod( value ) ) );

    } else if ( qVariantCanConvert< AreaHeight > ( index.data() ) ) {
        QComboBox * p = dynamic_cast< QComboBox * >( editor );
        bool value = p->currentIndex() == 0 ? true : false;
        model->setData( index, qVariantFromValue( AreaHeight( value ) ) );
    } else {
        QItemDelegate::setModelData( editor, model, index );
    }
}

void
CentroidDelegate::updateEditorGeometry(QWidget *editor
                                       , const QStyleOptionViewItem &option
                                       , const QModelIndex &index) const
{
    Q_UNUSED( index );
    editor->setGeometry( option.rect );
}


//////////////////////////////

CentroidDelegate::PeakWidthMethod::PeakWidthMethod( adcontrols::CentroidMethod::ePeakWidthMethod value ) : value_( value )
{
}

adcontrols::CentroidMethod::ePeakWidthMethod
CentroidDelegate::PeakWidthMethod::methodValue() const
{
    return value_;
}

QString
CentroidDelegate::PeakWidthMethod::displayValue() const
{
    switch( value_ ) {
        case adcontrols::CentroidMethod::ePeakWidthConstant:
            return "Constant";
        case adcontrols::CentroidMethod::ePeakWidthProportional:
            return "Proportional";
        case adcontrols::CentroidMethod::ePeakWidthTOF:
        default:
            return "TOF";
    }
    return "unknown";
}

//////////////////////
CentroidDelegate::AreaHeight::AreaHeight( bool value ) : value_(value)
{
}

QString
CentroidDelegate::AreaHeight::displayValue() const
{
    return value_ ? "Area" : "Height";
}

bool
CentroidDelegate::AreaHeight::methodValue() const
{
    return value_;
}