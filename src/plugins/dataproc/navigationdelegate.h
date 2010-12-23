// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef NAVIGATIONDELEGATE_H
#define NAVIGATIONDELEGATE_H

#include <QItemDelegate>
#include <portfolio/folium.h>

namespace dataproc {

    class NavigationDelegate : public QItemDelegate {
        Q_OBJECT
    public:
        explicit NavigationDelegate(QObject *parent = 0);

    signals:

    public slots:

    };

    Q_DECLARE_METATYPE( portfolio::Folium )

}

#endif // NAVIGATIONDELEGATE_H
