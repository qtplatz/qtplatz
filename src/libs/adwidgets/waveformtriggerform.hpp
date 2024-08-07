/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
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

#ifndef WAVEFORMTRIGGERFORM_HPP
#define WAVEFORMTRIGGERFORM_HPP

#include "adwidgets_global.hpp"
#include <QWidget>
#include <adwidgets/lifecycle.hpp>

namespace adwidgets {

    namespace Ui {
        class WaveformTriggerForm;
    }

    class ADWIDGETSSHARED_EXPORT WaveformTriggerForm : public QWidget
                                                     , public adplugin::LifeCycle {

        Q_OBJECT
        Q_INTERFACES( adplugin::LifeCycle )

    public:
        explicit WaveformTriggerForm( QWidget *parent = 0 );
        ~WaveformTriggerForm();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& ) override;
        void OnInitialUpdate() override;
        void OnFinalClose() override;
        bool getContents( boost::any& ) const override;
        bool setContents( boost::any&& ) override;
        void * query_interface_workaround( const char * typenam ) override;

    private:
        Ui::WaveformTriggerForm *ui;
    };

}

#endif // WAVEFORMTRIGGERFORM_HPP
