// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
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

#pragma once

#include <QWidget>

class QTreeView;
class QStandardItemModel;
class QStandardItem;
class QModelIndex;

namespace portfolio { class Folium; }

namespace dataproc {

    class Dataprocessor;
    class NavigationDelegate;

    class NavigationWidget : public QWidget {
        Q_OBJECT
    public:
        explicit NavigationWidget(QWidget *parent = 0);
        ~NavigationWidget();

        // bool autoSyncronization() const;
        // void setAutoSynchronization( bool sync );
        void invalidateSession( Dataprocessor * );

    signals:

    public slots:
        void handleAddSession( Dataprocessor * );
		void handleSessionUpdated( Dataprocessor *, portfolio::Folium& );
		void handleSessionUpdated( Dataprocessor *, const QString& foliumId );
		void handleFolderChanged( Dataprocessor *, const QString& folder );

        // add child node when process applied (such as Centroid)
        void handleFoliumChanged( Dataprocessor *, const portfolio::Folium& );
        void handleRemoveSession( Dataprocessor * );

        void handleCheckAllSpectra();
        void handleUncheckAllSpectra();

        void handleCheckAllXICs();
        void handleUncheckAllXICs();

        void handleInvalidateFolium( Dataprocessor *, portfolio::Folium );

    private slots:
        void initView();
        // connecting to QAbstractItemView
        void handle_activated( const QModelIndex& );
        void handle_clicked( const QModelIndex& );
        void handle_doubleClicked( const QModelIndex& );
        void handle_entered( const QModelIndex& );
        void handle_pressed( const QModelIndex& );
        void handleItemChanged( QStandardItem * );

        void handleContextMenuRequested( const QPoint& );

    private:
        bool autoSync_;
        QTreeView * pTreeView_;
        QStandardItemModel * pModel_;
        NavigationDelegate * pDelegate_;
        void handleAllCheckState( bool, const QString& );
        void handleAllCheckState( bool, const QString&, const QString& re );
        bool eventFilter( QObject *, QEvent * );
    };

}
