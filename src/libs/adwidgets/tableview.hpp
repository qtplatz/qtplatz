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

#ifndef TABLEVIEW_HPP
#define TABLEVIEW_HPP

#include "adwidgets_global.hpp"
#include <QTableView>

namespace adwidgets {

    class ADWIDGETSSHARED_EXPORT TableView : public QTableView
    {
        Q_OBJECT
    public:
        explicit TableView(QWidget *parent = 0);

    protected:
        // reimplement QTableView
        void keyPressEvent( QKeyEvent * event ) override;
        void mouseReleaseEvent( QMouseEvent * event ) override;

        bool allowDelete() { return allowDelete_; }
        void setAllowDelete( bool f ) { allowDelete_ = f; }

        virtual void handleDeleteSelection();

        virtual void addActionsToContextMenu( QMenu&, const QPoint& ) const;

        void contextMenuEvent( QContextMenuEvent * ) override;

    private:
        bool allowDelete_;
        
    signals:
        void rowsDeleted();

    public slots:
        virtual void handleCopyToClipboard();
        virtual void handlePaste();
    };

}

#endif // TABLEVIEW_HPP
