/**************************************************************************
** Copyright (C) 2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2019 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "adwidgets_global.hpp"
#include <adwidgets/lifecycle.hpp>
#include <QWidget>
#include <memory>

class QMenu;

namespace admethods { namespace controlmethod { class ADTraceMethod; } }

namespace adwidgets {

    class MolTableView;

    class ADWIDGETSSHARED_EXPORT ADTraceWidget : public QWidget
                                                , public adplugin::LifeCycle {

        Q_OBJECT
        Q_INTERFACES( adplugin::LifeCycle );

    public:
        explicit ADTraceWidget(QWidget *parent = 0);
        ~ADTraceWidget();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& ) override;
        void OnInitialUpdate() override;
        void onUpdate( boost::any&& ) override;
        void OnFinalClose() override;
        bool getContents( boost::any& ) const override;
        bool setContents( boost::any&& ) override;
        //
        bool getContents( admethods::controlmethod::ADTraceMethod& ) const;
        bool setContents( const admethods::controlmethod::ADTraceMethod& );

        QByteArray readJson() const;
        void setJson( const QByteArray& );

    signals:
        void dataChanged( int row, int column );

    public slots:
        void handleVOffsets( const QVector< double >& );

    private slots:

    private:
        void handleContextMenu( QMenu&, const QPoint& );
        class delegate;
        class impl;
        std::unique_ptr< impl > impl_;
    };

}
