/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <QWidget>
#include <memory>

namespace adcontrols { class MassSpectrum; class MetIdMethod; }
namespace portfolio  { class Foliium; }

namespace lipidid {

    class MetIdWidget : public QWidget {
        Q_OBJECT
    public:
        ~MetIdWidget();
        explicit MetIdWidget( QWidget *parent = 0 );
        typedef adcontrols::MetIdMethod value_type;

        void onInitialUpdate();

        value_type getContents() const;
        bool setContents( const value_type& );

    public slots:

    signals:
        void triggered();

    private:
        class impl;
        std::unique_ptr< impl > impl_;
    };

}
