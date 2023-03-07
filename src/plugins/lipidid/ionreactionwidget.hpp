/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

namespace adcontrols { class IonReactionMethod; }

namespace lipidid {

    class IonReactionWidget : public QWidget {
        Q_OBJECT

    public:
        explicit IonReactionWidget(QWidget *parent = 0);
        ~IonReactionWidget();

        void onInitialUpdate();

        adcontrols::IonReactionMethod getContents() const;
        bool setContents( const adcontrols::IonReactionMethod& );
        void setSQLiteFilename( const QString& );

    signals:
        void triggered();
        void rejected();

    private:
        class impl;
        std::unique_ptr< impl > impl_;

    };
}
