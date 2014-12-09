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

#ifndef CONTROLMETHODTABLE_HPP
#define CONTROLMETHODTABLE_HPP

#pragma once

#include "tableview.hpp"
#include "adwidgets_global.hpp"

#include <memory>

class QStandardItemModel;

namespace adcontrols { class ControlMethod; namespace controlmethod { class MethodItem; } }

namespace adwidgets {

    class ControlMethodWidget;

    class ControlMethodTable : public TableView {
        Q_OBJECT
    public:
        explicit ControlMethodTable( ControlMethodWidget * parent );

        void onInitialUpdate();

        void setSharedPointer( std::shared_ptr< adcontrols::ControlMethod > );
        bool getContents( adcontrols::ControlMethod& );
        const adcontrols::controlmethod::MethodItem& operator []( int row ) const;
        QStandardItemModel& model();

        void addItem( const QString& );
        bool append( const adcontrols::controlmethod::MethodItem& );
        void commit();

        // TableView
        void showContextMenu( const QPoint& );

    private:
        bool setContents( const adcontrols::ControlMethod& );
        void currentChanged( const QModelIndex&, const QModelIndex& ) override;
        void insertMethod( const QString& model, const QModelIndex& );
        void sort();
        void delLine( int row );
        void setData( const adcontrols::controlmethod::MethodItem&, int row );
        adcontrols::controlmethod::MethodItem data( int row ) const;

        ControlMethodWidget * parent_;
        QStandardItemModel * model_;
        std::shared_ptr< adcontrols::ControlMethod > method_;
        QList< QString > items_;

    signals:
        void onAddMethod( const QString& item );

    public slots:

    };

}

#endif // CONTROLMETHODTABLE_HPP
