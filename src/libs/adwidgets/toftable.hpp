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

#pragma once

#include "tableview.hpp"
#include <memory>
#include <QItemDelegate>

class QStandardItemModel;

namespace adcontrols { class MSPeaks; class MSPeak; }

namespace adwidgets {

    class TOFTable : public TableView {
        Q_OBJECT
    public:
        explicit TOFTable(QWidget *parent = 0);
        void onInitialUpdate();
        QStandardItemModel& model() { return *model_; }
        void setPeaks( const adcontrols::MSPeaks&, double t0 = 0 );
        void clear();

    signals:

    public slots:
        void handleCopyToClipboard();

    private:
        std::shared_ptr< QStandardItemModel > model_;

        void addPeak( const adcontrols::MSPeak&, double t0 );
        friend class MSPeakWidget;
        class ItemDelegate;
    };

}

