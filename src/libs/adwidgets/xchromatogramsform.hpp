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
#include <adcontrols/constants_fwd.hpp>
#include <QWidget>
#include <memory>

namespace adcontrols { class XChromatogramsMethod; }

namespace adwidgets {

    class ADWIDGETSSHARED_EXPORT XChromatogramsForm : public QWidget {

        Q_OBJECT

    public:
        explicit XChromatogramsForm( QWidget *parent = 0 );
        ~XChromatogramsForm();

        void OnInitialUpdate();
        void getContents( adcontrols::XChromatogramsMethod& ) const;
        void setContents( const adcontrols::XChromatogramsMethod& );

        void setDigitizerMode( bool );
        void setCalibrationFilename( QString&& stem, QString&& filename );

    private:
        XChromatogramsForm( const XChromatogramsForm& ) = delete;

        class impl;
        std::unique_ptr< impl > impl_;

    signals:
        void valueChanged();
        void polarityToggled( adcontrols::ion_polarity );

    public slots:

    };

}
