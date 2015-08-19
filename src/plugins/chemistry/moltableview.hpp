/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef MOLTABLEVIEW_HPP
#define MOLTABLEVIEW_HPP

#include <adwidgets/tableview.hpp>
#include <QTableView>
#include <QUrl>
#include <memory>
#include <set>

class QStandardItemModel;
class QProgressBar;

namespace adchem { class SDFile; }

namespace chemistry {

    class MolTableDelegate;
    class ChemQuery;

    class MolTableView : public adwidgets::TableView {
        Q_OBJECT
    public:
        explicit MolTableView(QWidget *parent = 0);
        ~MolTableView();
        
        void prepare( const ChemQuery& );
        void addRecord( const ChemQuery& );

        void setMol( adchem::SDFile&, QProgressBar& );
        
        QStandardItemModel& model() { return *model_; }

    signals:
        void dropped( const QList< QUrl >& );
        void onCurrentChanged( const QModelIndex& );

    public slots:

    private:
        void handleContextMenu( const QPoint& );
        
        void dragEnterEvent( QDragEnterEvent * ) override;
        void dragMoveEvent( QDragMoveEvent * ) override;
        void dragLeaveEvent( QDragLeaveEvent * ) override;
        void dropEvent( QDropEvent * ) override;
        void currentChanged( const QModelIndex&, const QModelIndex& ) override;

        MolTableDelegate * delegate_;
        QStandardItemModel * model_;
        std::set< std::string > hideColumns_;

    private slots:
        void handleCopyToClipboard();
        void handlePaste();
    };

}

#endif // MOLTABLEVIEW_HPP
