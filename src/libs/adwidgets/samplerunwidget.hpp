/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef SAMPLERUNWIDGET_HPP
#define SAMPLERUNWIDGET_HPP

#pragma once

#include <QWidget>
#include <adwidgets/lifecycle.hpp>
#include "adwidgets_global.hpp"

namespace adcontrols { class SampleRun; }

namespace adwidgets {

    class ADWIDGETSSHARED_EXPORT SampleRunWidget : public QWidget
                                                 , public adplugin::LifeCycle {
        Q_OBJECT
        Q_INTERFACES( adplugin::LifeCycle )

    public:
        explicit SampleRunWidget(QWidget *parent = 0);

        // LifeCycle
        void OnCreate( const adportable::Configuration& ) override;
        void OnInitialUpdate() override;
        void OnFinalClose() override;

        bool getContents( boost::any& ) const override;
        bool setContents( boost::any&& ) override;

        void setSampleRun( const adcontrols::SampleRun& );
        void getSampleRun( adcontrols::SampleRun& ) const;

    signals:
        void apply();

    public slots:

    };

}

#endif // SAMPLERUNWIDGET_HPP
