// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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
        void handleSessionUpdated( Dataprocessor * );

    private slots:
        void initView();
        // connecting to QAbstractItemView
        void handle_activated( const QModelIndex& );
        void handle_clicked( const QModelIndex& );
        void handle_doubleClicked( const QModelIndex& );
        void handle_entered( const QModelIndex& );
        void handle_pressed( const QModelIndex& );

        void handleContextMenuRequested( const QPoint& );

    private:
        bool autoSync_;
        boost::scoped_ptr< QTreeView > pTreeView_;
        boost::scoped_ptr< QStandardItemModel > pModel_;
        boost::scoped_ptr< NavigationDelegate > pDelegate_;
    };

}

#endif // NAVIGATIONWIDGET_H
