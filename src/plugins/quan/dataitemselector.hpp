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

#ifndef DATAITEMSELECTOR_HPP
#define DATAITEMSELECTOR_HPP

#include <QTreeView>
#include <memory>

namespace adcontrols { class datafile; class dataSubscriber; }

class QStandardItemModel;
class QStandardItem;

namespace quan {

    class DataItemSelector : public QTreeView
    {
        Q_OBJECT
    public:
        explicit DataItemSelector(QWidget *parent = 0);
        ~DataItemSelector();

        void setData( std::shared_ptr< adcontrols::datafile >& );
        void clearData();

    private:
        std::shared_ptr< QStandardItemModel > model_;
        std::shared_ptr< adcontrols::dataSubscriber > subscriber_;

        void setRaw( QStandardItem * parent );
        void setProcessed( QStandardItem * parent );
        void handleValueChanged( const QModelIndex& );

    signals:

    public slots:

    };

}

#endif // DATAITEMSELECTOR_HPP
