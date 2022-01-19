/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2022 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "moltabledelegate.hpp"
#include "document.hpp"
#include <adchem/sdmol.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adwidgets/delegatehelper.hpp>
#include <QPainter>
#include <QSvgRenderer>

namespace {
    using adwidgets::ColumnState;
    using adcontrols::ChemicalFormula;

    struct paint_f_precision {
        void operator()( const ColumnState& state
                         , QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const {
            painter->drawText( option.rect
                               , option.displayAlignment
                               , QString::number( index.data( Qt::EditRole ).toDouble(), 'f', state.precision  ) );
        }
    };

    ///////////////////////
    struct paint_f_formula {
        void operator()( const ColumnState& state, QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const {
            std::string formula = ChemicalFormula::formatFormulae( index.data().toString().toStdString() );
            adwidgets::DelegateHelper::render_html2( painter, option, QString::fromStdString( formula ) );
        }
    };

    ///////////////////////
    struct paint_f_svg {
        void operator()( const ColumnState&, QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const {
            painter->save();
            QSvgRenderer renderer( index.data().toByteArray() );
            painter->translate( option.rect.x(), option.rect.y() );
            painter->scale( 1.0, 1.0 );
            QRect target( 0, 0, option.rect.width(), option.rect.height() );
            renderer.render( painter, target );
            painter->restore();
        }
    };

    struct paint_f_protocol {
        void operator()( const ColumnState&, QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const {
            painter->drawText( option.rect, option.displayAlignment, index.data().toInt() < 0 ? "*" : index.data().toString() );
        }
    };
}

using namespace sdfview;

void
MolTableDelegate::setColumnField( int column, adwidgets::ColumnState::fields f, bool editable, bool checkable )
{
    columnStates_[ column ] = adwidgets::ColumnState( f, editable, checkable );
	if ( f == adwidgets::ColumnState::f_mass )
        columnStates_[ column ].precision = 7;
}

adwidgets::ColumnState
MolTableDelegate::columnState( int column ) const
{
    auto it = columnStates_.find( column ) ;
    if ( it != columnStates_.end() ) {
        return it->second;
    }
    return {};
}

////////////////////////////

void
MolTableDelegate::paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    using adwidgets::ColumnState;

    QStyleOptionViewItem opt(option);
    initStyleOption( &opt, index );
    opt.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
    auto state = columnState( index.column() );
    auto field = state.field;

    if ( index.data().isNull() ) {
        emit onNullData( index );
    }

    switch( field ) {
    case ColumnState::f_formula:
        paint_f_formula()( state, painter, opt, index );
        break;
    case ColumnState::f_mass:
        paint_f_precision()( state, painter, opt, index );
        break;
    case ColumnState::f_svg:
        paint_f_svg()( state, painter, opt, index );
        break;
    default:
        QStyledItemDelegate::paint( painter, opt, index );
    }
}
