// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#pragma once

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <boost/smart_ptr.hpp>

namespace qtwidgets {

    class TreeItem;
    typedef boost::shared_ptr<TreeItem> TreeItemPtr;
    
    class SequencesModel : public QAbstractItemModel {
        Q_OBJECT
        
    public:
        SequencesModel( QObject *parent = 0 );
        ~SequencesModel();
        
        QVariant data(const QModelIndex &index, int role) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
        
        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
        QModelIndex parent(const QModelIndex &index) const;
        
        int rowCount(const QModelIndex &parent = QModelIndex()) const;
        int columnCount(const QModelIndex &parent = QModelIndex()) const;
        
        Qt::ItemFlags flags(const QModelIndex &index) const;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
        bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);
        
        bool insertColumns(int position, int columns, const QModelIndex &parent = QModelIndex());
        bool removeColumns(int position, int columns, const QModelIndex &parent = QModelIndex());
        bool insertRows(int position, int rows, const QModelIndex &parent = QModelIndex());
        bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex());
        
        ///////////////////////////////////////////////////////////////////////////
        int findParent( const QString& addrString ) const;
        
        ///////////////////////////////////////////////////////////////////////////
    private:
        TreeItem * getItem(const QModelIndex &index) const;
        TreeItemPtr rootItem_;
    };

}
