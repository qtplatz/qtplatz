/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef MSCHROMATOGRAMFORM_HPP
#define MSCHROMATOGRAMFORM_HPP

#include <adcontrols/constants_fwd.hpp>
#include <adwidgets/lifecycle.hpp>
#include <QWidget>
#include "adwidgets_global.hpp"

namespace adcontrols { class ProcessMethod; class MSChromatogramMethod; }

namespace adwidgets {

    namespace Ui {
        class MSChromatogramForm;
    }


    class ADWIDGETSSHARED_EXPORT MSChromatogramForm : public QWidget
                                                    , public adplugin::LifeCycle {

        Q_OBJECT
        Q_INTERFACES( adplugin::LifeCycle )

    public:
        explicit MSChromatogramForm(QWidget *parent = 0);
        ~MSChromatogramForm();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& );
        void OnInitialUpdate();
        void OnFinalClose();
        bool getContents( boost::any& ) const;
        bool setContents( boost::any&& );
        //
        void setContents( const adcontrols::MSChromatogramMethod& );
        void getContents( adcontrols::MSChromatogramMethod& ) const;

    signals:
        void polarityToggled( adcontrols::ion_polarity );
        void triggerProcess();
        void onEnableLockMass( bool );
        void onAutoTargetingEnabled( bool );

    private:
        Ui::MSChromatogramForm *ui;
    };
}

#endif // MSCHROMATOGRAMFORM_HPP
