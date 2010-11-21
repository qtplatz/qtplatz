// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef NAVIGATIONWIDGET_H
#define NAVIGATIONWIDGET_H

#include <QWidget>
#include <boost/smart_ptr.hpp>

class QTreeView;
class QStandardItemModel;
class QLabel;

namespace dataproc {

    class NavigationWidget : public QWidget {
        Q_OBJECT
    public:
        explicit NavigationWidget(QWidget *parent = 0);

    signals:

    public slots:

    private:
        boost::scoped_ptr< QLabel > pTitle_;
        boost::scoped_ptr< QTreeView > pTreeView_;
        boost::scoped_ptr< QStandardItemModel > pModel_;
    };

}

#endif // NAVIGATIONWIDGET_H
