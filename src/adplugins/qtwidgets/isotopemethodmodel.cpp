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

#include "isotopemethodmodel.hpp"
#include <qtwrapper/qstring.hpp>
#include <QDebug>

using namespace qtwidgets;

IsotopeMethodModel::IsotopeMethodModel(QObject *parent) : QAbstractListModel( parent )
{
    QHash< int, QByteArray > roles;
    roles[ FormulaRole ] = "formula";    // std::wstring formula
    roles[ AdductRole ]  = "adduct";      // std::wstring adduct;
    roles[ LoseRole ] = "lose";
    roles[ ChargeRole ]  = "chargeState"; // size_t chargeState;
    roles[ AmountsRole ] = "amounts";    // double relativeAmounts;
    roles[ MassRole ]    = "mass";       // doubl mass (read only)
    // setRoleNames( roles );
}

int
IsotopeMethodModel::rowCount( const QModelIndex& ) const
{
    return static_cast<int>( method_.size() );
}

Qt::ItemFlags
IsotopeMethodModel::flags( const QModelIndex& ) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QVariant
IsotopeMethodModel::data( const QModelIndex& index, int role ) const
{
    if ( index.row() < 0 || index.row() >= int( method_.size() ) )
        return QVariant();

    adcontrols::IsotopeMethod::vector_type::const_iterator formula = method_.begin() + index.row();

    switch ( role ) {
    case FormulaRole:
        return qtwrapper::qstring::copy( formula->formula );
    case AdductRole:
        return qtwrapper::qstring::copy( formula->adduct );
    case ChargeRole:
        return int( formula->chargeState );
    case AmountsRole:
        return formula->relativeAmounts;
    case MassRole:
        return 999.999;
    default:
        break;
    }
    return QVariant();
}

//Q_INVOKABLE
bool
IsotopeMethodModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    Q_UNUSED( index );
    Q_UNUSED( value );
    Q_UNUSED( role );
    return true;
}

void
IsotopeMethodModel::appendFormula( const adcontrols::IsotopeMethod::Formula& formula, int rowIndex )
{
    Q_UNUSED( rowIndex );
    beginInsertRows( QModelIndex(), int( method_.size() + 1), int(method_.size() + 1) );
    method_.addFormula( formula );
    endInsertRows();
}

void
IsotopeMethodModel::appendRow( int currentRow )
{
    appendFormula( adcontrols::IsotopeMethod::Formula( L"", L"---", L"H", 1, 1.0 ), currentRow );
}

void
IsotopeMethodModel::removeRow( int rowIndex )
{
    if ( rowIndex >= 0 && unsigned( rowIndex ) < method_.size() - 1 ) {
        method_.erase( method_.begin() + rowIndex, method_.begin() + rowIndex + 1 );
        // reset();
    }
}

void
IsotopeMethodModel::setProperty( int rowIndex, const QString& role, const QVariant& value )
{
    qDebug() << "setProperty(" << rowIndex << ", " << role << ", " << value << ")";
}

bool
IsotopeMethodModel::polarityPositive() const
{
    return method_.polarityPositive();
}

void
IsotopeMethodModel::polarityPositive( bool t )
{
    method_.polarityPositive( t );
    emit valueChanged();
}

bool
IsotopeMethodModel::useElectronMass() const
{
    return method_.useElectronMass();
}

void
IsotopeMethodModel::useElectronMass( bool t )
{
    method_.useElectronMass( t );
    emit valueChanged();
}

double
IsotopeMethodModel::threshold() const
{
    return method_.threshold();
}

void
IsotopeMethodModel::threshold( double t )
{
    method_.threshold( t ) ;
    emit valueChanged();
}

double
IsotopeMethodModel::resolution() const
{
    return method_.resolution();
}

void
IsotopeMethodModel::resolution( double t )
{
    method_.resolution( t );
    emit valueChanged();
}
