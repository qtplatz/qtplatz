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

#ifndef DATASEQUENCETREE_HPP
#define DATASEQUENCETREE_HPP

#include <QTreeView>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>

class QStandardItemModel;
class QStandardItem;
class QModelIndex;

namespace adcontrols { class datafile; class QuanSequence; }

namespace quan {

    namespace datasequencetree { class dataSubscriber; }

    class DataSequenceTree : public QTreeView {
        Q_OBJECT
    public:
        explicit DataSequenceTree(QWidget *parent = 0);

        void setData( std::shared_ptr< adcontrols::datafile >& );
        void setData( const QStringList& );
        bool setContents( const adcontrols::QuanSequence& );
        bool getContents( adcontrols::QuanSequence& );

    protected:
        void dragEnterEvent( QDragEnterEvent * ) override;
        void dragMoveEvent( QDragMoveEvent * ) override;
        void dragLeaveEvent( QDragLeaveEvent * ) override;
        void dropEvent( QDropEvent * ) override;

    private:
        std::vector< std::shared_ptr< datasequencetree::dataSubscriber > > dataSubscribers_;
        std::shared_ptr< QStandardItemModel > model_;
        std::vector< std::thread > threads_;
        std::mutex mutex_;

        void handleValueChanged( const QModelIndex& );
        void dropIt( const std::wstring& );
        void handleIt( datasequencetree::dataSubscriber * );
        std::atomic< size_t > dropCount_;

        void handleData( int row );
        size_t setRaw( datasequencetree::dataSubscriber *, QStandardItem * );
        size_t setProcessed( datasequencetree::dataSubscriber *, QStandardItem * );

    signals:
        void onJoin( int row );

    private slots:
        void handleJoin( int row );

    };

}

#endif // DATASEQUENCETREE_HPP
