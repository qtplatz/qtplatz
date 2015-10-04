/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#ifndef U5303AMETHODTABLE_HPP
#define U5303AMETHODTABLE_HPP

#include <adwidgets/tableview.hpp>

class QStandardItemModel;

namespace u5303acontrols { class device_method; }

namespace u5303a {

    class method; // defined in <u5303a/digitizer.hpp>

    class u5303AMethodTable : public adwidgets::TableView  {
        Q_OBJECT
    public:
        explicit u5303AMethodTable(QWidget *parent = 0);
        ~u5303AMethodTable();

        void onInitialUpdate();
        bool setContents( const u5303a::method& );
        bool getContents( u5303a::method& );
        bool setContents( const u5303acontrols::device_method& );
        bool getContents( u5303acontrols::device_method& );

    private:
        QStandardItemModel * model_;
        bool in_progress_;

    signals:

    public slots:
    private slots:
        void handleDataChanged( const QModelIndex&, const QModelIndex& );

    };

}

#endif // U5303AMETHODTABLE_HPP
