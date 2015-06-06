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

#ifndef QUERYRESULTTABLE_HPP
#define QUERYRESULTTABLE_HPP

#include <adwidgets/tableview.hpp>
#include <memory>
#include <set>

class QStandardItemModel;

namespace query {

    class QueryQuery;

    class QueryResultTable : public adwidgets::TableView  {
        Q_OBJECT
    public:
        ~QueryResultTable();
        explicit QueryResultTable(QWidget *parent = 0);

        void prepare( const QueryQuery& );
        void addRecord( const QueryQuery& );

        void setColumnHide( const std::string& );
        void clear();
        int findColumn( const QString& );

        inline QStandardItemModel& model() { return *model_; }

    private:
        std::unique_ptr< QStandardItemModel > model_;
        std::set< std::string > hideColumns_;

        void currentChanged( const QModelIndex&, const QModelIndex& ) override;

    signals:
        void onCurrentChanged( const QModelIndex& );

    public slots:

    };

}

#endif // QUERYRESULTTABLE_HPP
