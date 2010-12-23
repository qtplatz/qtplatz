// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef NAVIGATIONDELEGATE_H
#define NAVIGATIONDELEGATE_H

#include <QItemDelegate>
#include <portfolio/folium.h>
#include <portfolio/folder.h>

namespace dataproc {

    class Dataprocessor;

    class NavigationDelegate : public QItemDelegate {
        Q_OBJECT
    public:
        explicit NavigationDelegate(QObject *parent = 0);

        QWidget * createEditor( QWidget*, const QStyleOptionViewItem&, const QModelIndex& ) const;
        void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& ) const;
        void setEditorData( QWidget* editor, const QModelIndex& ) const;
        void setModelData( QWidget* editor, QAbstractItemModel * model, QModelIndex& ) const;
        void updateEditorGeometry( QWidget* editor, const QStyleOptionViewItem&, const QModelIndex& ) const;

    signals:

    public slots:

    };

    Q_DECLARE_METATYPE( portfolio::Folium )
    Q_DECLARE_METATYPE( portfolio::Folder )
    Q_DECLARE_METATYPE( Dataprocessor * )
}

#endif // NAVIGATIONDELEGATE_H
