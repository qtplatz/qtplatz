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
class QModelIndex;

namespace dataproc {

    class Dataprocessor;
    class NavigationDelegate;

    class NavigationWidget : public QWidget {
        Q_OBJECT
    public:
        explicit NavigationWidget(QWidget *parent = 0);

        bool autoSyncronization() const;
        void setAutoSynchronization( bool sync );

    signals:

    public slots:
        void toggleAutoSynchronization();
        void handleSessionAdded( Dataprocessor * );

    private slots:
        void openItem(const QModelIndex &index);
        void setCurrentFile( const QString& filepath );
        void initView();

    private:
        bool autoSync_;
        boost::scoped_ptr< QTreeView > pTreeView_;
        boost::scoped_ptr< QStandardItemModel > pModel_;
        boost::scoped_ptr< NavigationDelegate > pDelegate_;
    };

}

#endif // NAVIGATIONWIDGET_H
