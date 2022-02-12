/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2014-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <adwidgets/tableview.hpp>
#include <QString>
#include <QUrl>
#include <memory>
#include <set>

class QSqlQueryModel;
class QProgressBar;
class QStyledItemDelegate;

namespace adchem { class SDFile; }
namespace adwidgets { class MolTableView; }

namespace lipidid {

    class MolTableDelegate;
    class ChemQuery;

    class MolTableWnd : public QWidget {
        Q_OBJECT
    public:
        explicit MolTableWnd(QWidget *parent = 0);
        ~MolTableWnd();

        void setQuery( const QString& sqlstmt );
        void setModel( QAbstractItemModel * );
        void setModel( std::unique_ptr< QAbstractItemModel >&& );

        QAbstractItemModel * model();
        QVariant data( int row, const QString& column );

    signals:
        void dropped( const QList< QUrl >& );
        void activated( const QModelIndex& );
        void onProgressInitiated( int );
        void onProgressFinished();
        void onProgress( int );

    public slots:
        void handleSDFileChanged();

    private:
        void handleContextMenu( const QPoint& );

        void dragEnterEvent( QDragEnterEvent * ); // override;
        void dragMoveEvent( QDragMoveEvent * );   // override;
        void dragLeaveEvent( QDragLeaveEvent * ); // override;
        void dropEvent( QDropEvent * );           // override;

        std::unique_ptr< QAbstractItemModel > model_;
        adwidgets::MolTableView * table_;
        QAbstractItemDelegate * backup_;

    private slots:
        // void handleCopyToClipboard(); // override;
        void handlePaste(); // override;
        void handleDataChaged( const QModelIndex&, const QModelIndex&, const QVector<int>& );
        void handleNullData( const QModelIndex& );
    };

}
