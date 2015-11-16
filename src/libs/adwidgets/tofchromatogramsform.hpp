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

#include "adwidgets_global.hpp"
#include <QWidget>
#include <memory>

namespace adcontrols { class TofChromatogramsMethod; }

namespace adwidgets {

    class ADWIDGETSSHARED_EXPORT TofChromatogramsForm : public QWidget {

        Q_OBJECT
        
    public:
        explicit TofChromatogramsForm( QWidget *parent = 0 );
        ~TofChromatogramsForm();

        void OnInitialUpdate();
        void getContents( adcontrols::TofChromatogramsMethod& ) const;
        void setContents( const adcontrols::TofChromatogramsMethod& );

    private:
        TofChromatogramsForm( const TofChromatogramsForm& ) = delete;

        class impl;
        std::unique_ptr< impl > impl_;
        
    signals:
        void valueChanged();
        void applyTriggered();

    public slots:

    };

}

