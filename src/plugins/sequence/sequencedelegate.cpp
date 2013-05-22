/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#include "sequencedelegate.hpp"
#include <adsequence/schema.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <algorithm>
#include <cstring>

namespace {
    static const char * list[] = { "UNK", "STD", "QC" };

    struct sample_type {
        static const char * name( /* adsequence::SAMPLE_TYPE */ int typ ) {
            if ( size_t( typ ) < sizeof( list ) / sizeof( list[0] ) )
                return list[ typ ];
            return "unknown";
        }
        static adsequence::SAMPLE_TYPE value( const char * name ) {
            BOOST_FOREACH( const char * a, list ) {
                if ( std::strcmp( a, name ) == 0 )
                    return static_cast< adsequence::SAMPLE_TYPE >( std::distance( list[0], a ) );
            }
            return adsequence::SAMPLE_TYPE_UNKNOWN;
        }
    };

    static struct string_format_t {
        const char * variable_name;
        const char * format;
    } string_format [] = {
        { "injvol", "%.1lf" }, { "run_length", "%.2lf" }
    };

    struct double_value {
        static std::string toString( const double& value, const std::string& variable ) {
            BOOST_FOREACH( const string_format_t& a, string_format ) {
                if ( variable == a.variable_name )
                    return ( boost::format( a.format ) % value ).str();
            }
            return ( boost::format( "%.3lf" ) % value ).str();
        }
    };
}

using namespace sequence;

SequenceDelegate::SequenceDelegate( QObject *parent ) : QItemDelegate( parent )
                                                      , schema_( new adsequence::schema )
{
}

QWidget *
SequenceDelegate::createEditor( QWidget *parent
                                 , const QStyleOptionViewItem &option
                                 , const QModelIndex &index) const
{
    return QItemDelegate::createEditor( parent, option, index );
}

void
SequenceDelegate::paint(QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    if ( schema_ && schema_->size() > size_t( index.column() ) ) {

        const adsequence::column& column = (*schema_)[ index.column() ];

        if ( column.type() == adsequence::COLUMN_SAMPLE_TYPE ) {
            drawDisplay( painter, option, option.rect, sample_type::name( index.data( Qt::EditRole ).toInt() ) );
        } else if ( column.type() == adsequence::COLUMN_INT ) {
            QItemDelegate::paint( painter, option, index );
        } else if ( column.type() == adsequence::COLUMN_DOUBLE ) {
            double value = index.data( Qt::EditRole ).toDouble();
            drawDisplay( painter, option, option.rect, double_value::toString( value, column.name() ).c_str() );
        } else if ( column.type() == adsequence::COLUMN_VARCHAR ) {
            QItemDelegate::paint( painter, option, index );
        }
        return;
    }
    QItemDelegate::paint( painter, option, index );
}

void
SequenceDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QItemDelegate::setEditorData( editor, index );
}

void
SequenceDelegate::setModelData( QWidget * editor
                                  , QAbstractItemModel * model
                                  , const QModelIndex& index ) const
{
    QItemDelegate::setModelData( editor, model, index );
    emit valueChanged( index );
}

void
SequenceDelegate::updateEditorGeometry( QWidget * editor
                                       , const QStyleOptionViewItem& option
                                       , const QModelIndex &index ) const
{
    Q_UNUSED( index );
    editor->setGeometry( option.rect );
}

void
SequenceDelegate::schema( const adsequence::schema& schema )
{
    schema_.reset( new adsequence::schema( schema ) );
}

const adsequence::schema&
SequenceDelegate::schema() const
{
    return * schema_;
}
