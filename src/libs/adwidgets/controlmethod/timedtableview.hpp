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

#pragma once

#include "tableview.hpp"
#include "columnstate.hpp"
#include <functional>
#include <memory>

class QMenu;

namespace adcontrols {
    namespace ControlMethod { class ModuleCap; class EventCap; }
}

namespace adwidgets {

    class TimedTableView : public TableView  {
        Q_OBJECT
    public:
        explicit TimedTableView( QWidget *parent = 0);
        ~TimedTableView();

        void onInitialUpdate();

        void setContextMenuHandler( std::function<void(const QPoint& )> );

    private:
        //class delegate;
        class impl;
        std::unique_ptr< impl > impl_;

        //void handleValueChanged( const QModelIndex& );
        void handleContextMenu( const QPoint& );

        void dragEnterEvent( QDragEnterEvent * ) override;
        void dragMoveEvent( QDragMoveEvent * ) override;
        void dragLeaveEvent( QDragLeaveEvent * ) override;
        void dropEvent( QDropEvent * ) override;

    signals:
        void onContextMenu( QMenu&, const QPoint& );
        void onValueChanged();
                                           
    private slots:
        void handleCopyToClipboard() override;
        void handlePaste() override;
    };

}


