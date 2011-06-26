//
#include "centroidmethodmodel.hpp"
#include "standarditemhelper.hpp"
#include <QDebug>

CentroidMethodModel::CentroidMethodModel(QObject *parent) : QObject( parent ) // AbstractListModel( parent )
{
    peakMethod_ = "Tof";

    // mapping of role identifiers to role property names in Declarative UI, Qt4.6 or later required
    QHash< int, QByteArray > roles;
    roles[ Qt::UserRole + 1 ] = "name";
    roles[ Qt::UserRole + 2 ] = "value";
    // setRoleNames( roles );

    // items_ << MethodItem( "ScanType", qVariantFromValue( ScanType() ) );
    // items_ << MethodItem( "Area Height", qVariantFromValue( AreaHeight() ));
    // items_ << MethodItem( "Baseline width", qVariantFromValue( 500.0 ));
    // items_ << MethodItem( "Peak Centroid Fraction", qVariantFromValue(50));
}

/*
QVariant
CentroidMethodModel::data(const QModelIndex & index, int role) const
{
    qDebug() << "data(" << index << " , role=" << role;
    
    if (index.row() < 0 || index.row() > items_.count())
        return QVariant();
    
    const MethodItem item = items_[ index.row() ];
    
    if ( role == Qt::UserRole + 1 ) {
        qDebug() << "\t return " << item.name_;
        return item.name_;
    }  else {
        qDebug() << "\t return item_";
        return item.item_;
    }
 }

int
CentroidMethodModel::rowCount( const QModelIndex& ) const
{
    return items_.count();
}
*/

QVariant
CentroidMethodModel::scanType() const
{
    return qVariantFromValue( ScanType() );
}

void
CentroidMethodModel::scanType( const QVariant& )
{
}

QVariant
CentroidMethodModel::areaHeight() const
{
    return qVariantFromValue( AreaHeight() );
}

void
CentroidMethodModel::areaHeight( const QVariant& )
{
}

QList< QString >
CentroidMethodModel::getEnumPeakMethod() const
{
    QList<QString> list;
    list << "Tof" << "Proportional" << "Constant";
    return list;
}

////////////////////////////
#if 0
ScanType::ScanType( QObject * parent ) : QAbstractListModel( parent )
{
    QHash< int, QByteArray > roles;
    roles[ Qt::UserRole + 1 ] = "name";
    roles[ Qt::UserRole + 2 ] = "value";
    setRoleNames( roles );

    beginInsertRows( QModelIndex(), rowCount(), rowCount() );
    items_ << MethodItem( "Tof", qVariantFromValue( CentroidMethodModel::ScanTypeTof() ) );
    items_ << MethodItem( "Constant", qVariantFromValue( CentroidMethodModel::ScanTypeConstant() ) );
    items_ << MethodItem( "Proportional", qVariantFromValue( CentroidMethodModel::ScanTypeProportional() ) );
    endInsertRows();    
}

int
ScanType::rowCount(const QModelIndex &parent) const
{
   return items_.count();
}

QVariant
ScanType::data(const QModelIndex & index, int role) const
{
    qDebug() << "data(" << index << " , role=" << role;
    
    if (index.row() < 0 || index.row() > items_.count())
        return QVariant();
    
    const MethodItem &item = items_[ index.row() ];
    
    if ( role == Qt::UserRole + 1 ) {
        qDebug() << "\t return " << item.name_;
        return item.name_;
    }  else {
        qDebug() << "\t return item_";
        return item.item_;
    }
 }
#endif
