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

#pragma once

#include "constants.hpp"
#include <adwidgets/tableview.hpp>

class QStandardItemModel;

namespace aqmd3controls {
    class device_method;
}

namespace aqmd3widgets {

    class aqmd3Table : public adwidgets::TableView  {
        Q_OBJECT
    public:
        explicit aqmd3Table(QWidget *parent = 0);
        ~aqmd3Table();

        void onInitialUpdate();

        bool setContents( const aqmd3controls::device_method& );

        bool getContents( aqmd3controls::device_method& );

        void onHandleValue( idCategory, int, const QVariant& );

        void setEnabled( const QString&, bool );

    private:
        class MyDelegate;
        QStandardItemModel * model_;
        bool pkd_enabled_;
    signals:
        void valueChanged( idCategory, int channel, const QVariant& );

    };

}
