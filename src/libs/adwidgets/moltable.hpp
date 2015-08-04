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

#ifndef MOLTABLE_HPP
#define MOLTABLE_HPP

#include "tableview.hpp"

class QStandardItemModel;
class QMenu;

namespace adcontrols { class moltable; }

namespace adwidgets {

    class MolTable : public TableView  {
        Q_OBJECT
    public:
        explicit MolTable(QWidget *parent = 0);
        ~MolTable();

        void onInitialUpdate();
        //void setEditable( fields, bool );

        void setContents( const adcontrols::moltable& );
        void getContents( adcontrols::moltable& );

        QStandardItemModel& model();

    private:
        QStandardItemModel * model_;
        bool mass_editable_;

        void handleValueChanged( const QModelIndex& );
        void handleContextMenu( const QPoint& );
        void enable_all( bool );

        void dragEnterEvent( QDragEnterEvent * ) override;
        void dragMoveEvent( QDragMoveEvent * ) override;
        void dragLeaveEvent( QDragLeaveEvent * ) override;
        void dropEvent( QDropEvent * ) override;

    signals:
        void onContextMenu( QMenu&, const QPoint& );
        void onValueChanged();
                                           
    public slots:
    
    };

}

#endif // MOLTABLE_HPP
