//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "elementalcompositiondelegate.hpp"
#include <QComboBox>

using namespace qtwidgets;

ElementalCompositionDelegate::ElementalCompositionDelegate(QObject *parent) :
    QItemDelegate(parent)
{
}

QWidget *
ElementalCompositionDelegate::createEditor(QWidget *parent
                                , const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
  if ( index.data().canConvert< ElectronMode >() ) {
        QComboBox * pCombo = new QComboBox( parent );
        QStringList list;
        list << "Even" << "Odd" << "Odd/Even";
        pCombo->addItems( list );
        return pCombo;
    } else {
        return QItemDelegate::createEditor( parent, option, index );
    }
}

void
ElementalCompositionDelegate::paint(QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
  if ( index.data().canConvert< ElectronMode >() ) {
    ElectronMode m = index.data().value< ElectronMode >();
    drawDisplay( painter, option, option.rect, m.displayValue() );
  } else {
    QItemDelegate::paint( painter, option, index );
  }
}

void
ElementalCompositionDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  if ( index.data().canConvert< ElectronMode >() ) {
    QComboBox * p = dynamic_cast< QComboBox * >( editor );
    ElectronMode m = index.data().value< ElectronMode >();    
    p->setCurrentIndex( m.methodValue() );
  } else {
    QItemDelegate::setEditorData( editor, index );
  }
}

void
ElementalCompositionDelegate::setModelData( QWidget *editor
                                , QAbstractItemModel *model,
                                const QModelIndex &index) const
{
  if ( index.data().canConvert< ElectronMode >() ) {
        QComboBox * p = dynamic_cast< QComboBox * >( editor );
        unsigned int value = p->currentIndex();
        model->setData( index, qVariantFromValue( ElectronMode( value ) ) );

    } else {
        QItemDelegate::setModelData( editor, model, index );
    }
}

void
ElementalCompositionDelegate::updateEditorGeometry(QWidget *editor
                                       , const QStyleOptionViewItem &option
                                       , const QModelIndex &index) const
{
    Q_UNUSED( index );
    editor->setGeometry( option.rect );
}

//////////////////////////////
ElementalCompositionDelegate::ElectronMode::ElectronMode( unsigned int value ) : value_(value)
{
}

QString
ElementalCompositionDelegate::ElectronMode::displayValue() const
{
    switch( value_ ) {
    case 0:
       return "Even";
    case 1:
       return "Odd";
    case 2:
       return "Odd/Even";
    }
    return "unknown";
}

unsigned int
ElementalCompositionDelegate::ElectronMode::methodValue() const
{
    return value_;
}
