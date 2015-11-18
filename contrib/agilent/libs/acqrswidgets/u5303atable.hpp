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

#ifndef U5303AMETHODTABLE_HPP
#define U5303AMETHODTABLE_HPP

#include "constants.hpp"
#include <adwidgets/tableview.hpp>

class QStandardItemModel;

namespace acqrscontrols {
    namespace u5303a { class device_method; class method; }
}

namespace acqrswidgets {

    class u5303ATable : public adwidgets::TableView  {
        Q_OBJECT
    public:
        explicit u5303ATable(QWidget *parent = 0);
        ~u5303ATable();

        void onInitialUpdate();

        bool setContents( const acqrscontrols::u5303a::device_method& );

        bool getContents( acqrscontrols::u5303a::device_method& );

        void onHandleValue( idCategory, int, const QVariant& );

        void setEnabled( const QString&, bool );

    private:
        class MyDelegate;
        QStandardItemModel * model_;

    signals:
        void valueChanged( idCategory, int channel, const QVariant& );

    };

}

#endif // U5303AMETHODTABLE_HPP
