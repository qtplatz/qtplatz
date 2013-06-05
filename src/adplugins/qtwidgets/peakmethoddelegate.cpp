/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
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

#include "peakmethoddelegate.hpp"
#include <QComboBox>

using namespace qtwidgets;

PeakMethodDelegate::PeakMethodDelegate(QObject *parent) :
    QItemDelegate(parent)
{
}

QWidget *
PeakMethodDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  if ( index.data().canConvert< PharmacopoeiaEnum >() ) {
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
  if ( index.data().canConvert< PharmacopoeiaEnum >() ) {
    PharmacopoeiaEnum e = index.data().value< PharmacopoeiaEnum >();
    drawDisplay( painter, option, option.rect, e.displayValue() );
  } else {
    QItemDelegate::paint( painter, option, index );
  }
}

void
PeakMethodDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  if ( index.data().canConvert< PharmacopoeiaEnum >() ) {
    using adcontrols::chromatography::ePharmacopoeia;
    
    QComboBox * p = dynamic_cast< QComboBox * >( editor );
    PharmacopoeiaEnum e = index.data().value< PharmacopoeiaEnum >();
    e.setCurrentValue( static_cast< ePharmacopoeia > ( p->currentIndex() ) );
  } else {
    QItemDelegate::setEditorData( editor, index );
  }
}

void
PeakMethodDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
  if ( index.data().canConvert< PharmacopoeiaEnum >() ) {
    using adcontrols::chromatography::ePharmacopoeia;
    QComboBox * p = dynamic_cast< QComboBox * >( editor );
    model->setData( index, qVariantFromValue( PharmacopoeiaEnum( static_cast< ePharmacopoeia >( p->currentIndex() ) ) ) );
    // emit signalMSReferencesChanged( index );
  } else {
    QItemDelegate::setModelData( editor, model, index );
  }
}

void
PeakMethodDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED( index );
    editor->setGeometry( option.rect );
}

////

PeakMethodDelegate::PharmacopoeiaEnum::PharmacopoeiaEnum( adcontrols::chromatography::ePharmacopoeia t ) : value_( t )
{
}

adcontrols::chromatography::ePharmacopoeia
PeakMethodDelegate::PharmacopoeiaEnum::methodValue() const
{
	return value_;
}

QString
PeakMethodDelegate::PharmacopoeiaEnum::displayValue() const
{
	switch ( value_ ) {
		case adcontrols::chromatography::ePHARMACOPOEIA_NotSpcified: return "Not specified";
		case adcontrols::chromatography::ePHARMACOPOEIA_EP: return "EP";
		case adcontrols::chromatography::ePHARMACOPOEIA_JP: return "JP";
		case adcontrols::chromatography::ePHARMACOPOEIA_USP: return "USP";
		default:
			break;
	}
	return "Not specified";
}

void
PeakMethodDelegate::PharmacopoeiaEnum::setCurrentValue( adcontrols::chromatography::ePharmacopoeia v )
{
	value_ = v;
}

