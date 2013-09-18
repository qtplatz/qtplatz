// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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
#include "mscalibsummarywidget.hpp"
#include <boost/format.hpp>
#include <QEvent>

using namespace qtwidgets2;

MSCalibSummaryDelegate::MSCalibSummaryDelegate(QObject *parent) : QItemDelegate(parent)
{
}

void
MSCalibSummaryDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    switch( index.column() ) {
    case MSCalibSummaryWidget::c_time:
        drawDisplay( painter, option, option.rect, ( boost::format("%.5lf") % index.data( Qt::EditRole ).toDouble() ).str().c_str() );
        break;
    case MSCalibSummaryWidget::c_exact_mass:
		if ( ! index.model()->data( index.model()->index( index.row(), MSCalibSummaryWidget::c_formula ), Qt::EditRole ).toString().isEmpty() )
			drawDisplay( painter, option, option.rect, ( boost::format("%.7lf") % index.data( Qt::EditRole ).toDouble() ).str().c_str() );
		break;
    case MSCalibSummaryWidget::c_mass:
	case MSCalibSummaryWidget::c_mass_calibrated:
        drawDisplay( painter, option, option.rect, ( boost::format("%.7lf") % index.data( Qt::EditRole ).toDouble() ).str().c_str() );
        break;
    case MSCalibSummaryWidget::c_intensity:
        drawDisplay( painter, option, option.rect, ( boost::format("%.1lf") % index.data( Qt::EditRole ).toDouble() ).str().c_str() );
        break;
    case MSCalibSummaryWidget::c_mass_error_mDa:
    case MSCalibSummaryWidget::c_mass_error_calibrated_mDa:
		if ( ! index.model()->data( index.model()->index( index.row(), MSCalibSummaryWidget::c_formula ), Qt::EditRole ).toString().isEmpty() ) {
			drawDisplay( painter, option, option.rect, ( boost::format("%.3lf") % index.data( Qt::EditRole ).toDouble() ).str().c_str() );
		}
		break;
    case MSCalibSummaryWidget::c_delta_mass:
    case MSCalibSummaryWidget::c_formula:
    case MSCalibSummaryWidget::c_is_enable:
    case MSCalibSummaryWidget::c_flags_:
    case MSCalibSummaryWidget::c_mode:
    case MSCalibSummaryWidget::c_fcn:
    default:
        QItemDelegate::paint( painter, option, index );
    }
}

void
MSCalibSummaryDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    
    QItemDelegate::setEditorData( editor, index );
}

bool
MSCalibSummaryDelegate::editorEvent( QEvent * event
                            , QAbstractItemModel * model
                            , const QStyleOptionViewItem& option
                            , const QModelIndex& index )
{
    bool res = QItemDelegate::editorEvent( event, model, option, index );
    if ( event->type() == QEvent::MouseButtonRelease && model->flags(index) & Qt::ItemIsUserCheckable ) {
        QVariant st = index.data( Qt::CheckStateRole );
        if ( index.data( Qt::EditRole ).type() == QVariant::Bool ) {
            model->setData( index, ( st == Qt::Checked ) ? true : false );
            emit valueChanged( index );
        }
    }
    return res;
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
MSCalibSummaryDelegate::to_print_text( std::string& text, const QModelIndex &index )
{
    text.clear();
    switch( index.column() ) {
    case MSCalibSummaryWidget::c_time:
        text = ( boost::format("%.5lf") % index.data( Qt::EditRole ).toDouble() ).str();
        break;
    case MSCalibSummaryWidget::c_exact_mass:
		if ( ! index.model()->data( index.model()->index( index.row(), MSCalibSummaryWidget::c_formula ), Qt::EditRole ).toString().isEmpty() )
			text = ( boost::format("%.7lf") % index.data( Qt::EditRole ).toDouble() ).str();
        break;
    case MSCalibSummaryWidget::c_mass:
	case MSCalibSummaryWidget::c_mass_calibrated:
        text = ( boost::format("%.7lf") % index.data( Qt::EditRole ).toDouble() ).str();
        break;
    case MSCalibSummaryWidget::c_intensity:
        text = ( boost::format("%.1lf") % index.data( Qt::EditRole ).toDouble() ).str();
        break;
    case MSCalibSummaryWidget::c_mass_error_mDa:
    case MSCalibSummaryWidget::c_mass_error_calibrated_mDa:
		if ( ! index.model()->data( index.model()->index( index.row(), MSCalibSummaryWidget::c_formula ), Qt::EditRole ).toString().isEmpty() )
			text = ( boost::format("%.3lf") % index.data( Qt::EditRole ).toDouble() ).str();
		break;
    case MSCalibSummaryWidget::c_formula:
    case MSCalibSummaryWidget::c_is_enable:
    case MSCalibSummaryWidget::c_flags_:
    case MSCalibSummaryWidget::c_mode:
    case MSCalibSummaryWidget::c_fcn:
    default:
        text = index.data( Qt::DisplayRole ).toString().toStdString();
    }
}
