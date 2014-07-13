/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "elementalcompmodel.hpp"
#include <QDebug>

using namespace qtwidgets;

ElementalCompModel::ElementalCompModel(QObject *parent) :
    QAbstractListModel(parent)
{
    QHash< int, QByteArray > roles;
    roles[ AtomRole ] = "atom";
    roles[ MinimumRole ] = "Minimum";
    roles[ MaximumRole ] = "Maximum";
    // setRoleNames( roles );
    // reset();
}

ElementalCompModel::ElectronMode
ElementalCompModel::electronMode() const
{
    return static_cast< ElectronMode >( method_.electronMode() );
}

void
ElementalCompModel::electronMode( ElectronMode v )
{
    method_.electronMode( static_cast< adcontrols::ElementalCompositionMethod::ElectronMode >(v) );
}

bool
ElementalCompModel::toleranceInPpm() const
{
    return method_.toleranceInPpm();
}

void
ElementalCompModel::toleranceInPpm( bool v )
{
    method_.toleranceInPpm( v );
}

double
ElementalCompModel::tolerance_ppm() const
{
    return method_.tolerance( true );
}

void
ElementalCompModel::tolerance_ppm( double v )
{
    method_.tolerance( true, v );
}

double
ElementalCompModel::tolerance_mDa() const
{
    return method_.tolerance( false );
}

void
ElementalCompModel::tolerance_mDa( double v )
{
    method_.tolerance( false, v );
}

double
ElementalCompModel::dbeMinimum() const
{
    return method_.dbeMinimum();
}

void
ElementalCompModel::dbeMinimum( double v )
{
    method_.dbeMinimum( v );
}

double
ElementalCompModel::dbeMaximum() const
{
    return method_.dbeMaximum();
}

void
ElementalCompModel::dbeMaximum( double v )
{
    method_.dbeMaximum( v );
}

size_t
ElementalCompModel::numResults() const
{
    return method_.numResults();
}

void
ElementalCompModel::numResults( size_t v )
{
    method_.numResults( v );
}

// QAbstractListModel
int
ElementalCompModel::rowCount( const QModelIndex& parent ) const
{
    Q_UNUSED( parent );
    return static_cast<int>( method_.size() );
}

QVariant
ElementalCompModel::data( const QModelIndex& index, int role ) const
{
    if ( index.row() < 0 || index.row() >= int( method_.size() ) )
        return QVariant();

    adcontrols::ElementalCompositionMethod::vector_type::const_iterator it = method_.begin() + index.row();

    switch ( role ) {
    case AtomRole:
        return it->atom.c_str();
    case MinimumRole:
        return int( it->numMinimum );
    case MaximumRole:
        return int( it->numMaximum );
    default:
        break;
    }
    return QVariant();

}

bool
ElementalCompModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    Q_UNUSED( index );
    Q_UNUSED( value );
    Q_UNUSED( role );
    return true;
}

Qt::ItemFlags
ElementalCompModel::flags( const QModelIndex& index ) const
{
    Q_UNUSED( index );
    return Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

//---------------------

void
ElementalCompModel::appendRow( int currentRow )
{
    using adcontrols::ElementalCompositionMethod;
    Q_UNUSED( currentRow );
    beginInsertRows( QModelIndex(), int(method_.size() + 1), int(method_.size() + 1) );
    method_.addCompositionConstraint( ElementalCompositionMethod::CompositionConstraint( "H", 0, 1 ) );
    endInsertRows();
}

void
ElementalCompModel::removeRow( int rowIndex )
{
    if ( rowIndex >= 0 && unsigned( rowIndex ) < method_.size() - 1 ) {
        method_.erase( method_.begin() + rowIndex, method_.end() + rowIndex + 1 );
        // reset();
    }
}

void
ElementalCompModel::setProperty( int rowIndex, const QString& role, const QVariant& value )
{
    qDebug() << "setProperty(" << rowIndex << ", " << role << ", " << value << ")";
}

