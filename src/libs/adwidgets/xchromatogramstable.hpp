/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "moltable.hpp"
#include <adcontrols/constants_fwd.hpp>

class QStandardItemModel;
class QMenu;

namespace adcontrols {
    class XChromatogramsMethod;
    namespace xic { class xic_method; }
    class MassSpectrometer;
}

namespace adwidgets {

    class XChromatogramsTable : public TableView  {
        Q_OBJECT
    public:
        explicit XChromatogramsTable(QWidget *parent = 0);
        ~XChromatogramsTable();

        void onInitialUpdate();
        void setValue( const adcontrols::XChromatogramsMethod& );
        void getContents( adcontrols::XChromatogramsMethod& );

        void setMassSpectrometer( std::shared_ptr< const adcontrols::MassSpectrometer > sp );
    private:
        void setValue( int row, const adcontrols::xic::xic_method&, adcontrols::ion_polarity );

    signals:
        void valueChanged();
        void editorValueChanged( const QModelIndex&, double );

    public slots:
        void handlePolarity( adcontrols::ion_polarity );
        void handleDataChanged( const QModelIndex&, const QModelIndex& );

    private slots:
        void handleContextMenu( const QPoint& pt );
        void handleSetAdducts();

    private:
        class impl;
        std::unique_ptr< impl > impl_;
    };

}
