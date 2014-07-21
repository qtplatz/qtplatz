/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#ifndef COMPOUNDSTABLE_HPP
#define COMPOUNDSTABLE_HPP

#include <adwidgets/tableview.hpp>
#include <cstdint>
class QModelIndex;
class QStandardItemModel;
class QPoint;

namespace adcontrols { class QuanCompounds; class QuanMethod; }

namespace quan {

    class CompoundsTable : public adwidgets::TableView {
        Q_OBJECT
    public:
        ~CompoundsTable();
        explicit CompoundsTable(QWidget *parent = 0);
        void onInitialUpdate();

        bool setContents( const adcontrols::QuanCompounds& );
        bool getContents( adcontrols::QuanCompounds& );
        void handleQuanMethod( const adcontrols::QuanMethod& );

    private:
        QStandardItemModel * model_;
        uint32_t levels_;

        void handleValueChanged( const QModelIndex& );
        void handleContextMenu( const QPoint& );

    signals:

    public slots:

    };

}

#endif // COMPOUNDSTABLE_HPP
