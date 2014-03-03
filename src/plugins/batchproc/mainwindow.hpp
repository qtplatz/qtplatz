/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#pragma once

#include <utils/fancymainwindow.h>
#include <memory>
#include <deque>

namespace Core { class IMode; }
namespace Utils { class StyledBar; }

class QStandardItemModel;
class QTableView;

namespace batchproc {

    class BatchprocDelegate;

    class MainWindow : public Utils::FancyMainWindow {
        Q_OBJECT
    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

        void createActions();

        QWidget * createContents( Core::IMode * );
        void onInitialUpdate();
        
    signals:
        void emitProgress( int, int, int );
            
    public slots:
        void handleDropped( const QList<QString>& );

    private:
        std::unique_ptr< QTableView > tableView_;
        std::unique_ptr< QStandardItemModel > model_;
        std::unique_ptr< BatchprocDelegate > delegate_;
        std::deque< std::wstring > files_;
        std::wstring destDir_;

        void createDockWidgets();
        QDockWidget * createDockWidget( QWidget *, const QString& title = QString() );
        void setSimpleDockWidgetArrangement();


    private slots:
        void showContextMenu( const QPoint& pt );
        void handleStateChanged( const QModelIndex& );
        void handleProgress( int row, int current, int total );
        void handleDestDirChanged( const QString& );
    };

}

#endif // MAINWINDOW_HPP
