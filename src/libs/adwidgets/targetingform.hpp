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

#ifndef TARGETINGFORM_HPP
#define TARGETINGFORM_HPP

#include "adwidgets_global.hpp"
#include <adcontrols/constants_fwd.hpp>
#include <QWidget>

#define TARGETING_FORM_LOCAL_IMPL 1

namespace Ui {
class TargetingForm;
}

namespace adcontrols { class TargetingMethod; class MetIdMethod; }

namespace adwidgets {

    // TargetingForm is re-used in Quan plugin for peak assign parameter setup

    class ADWIDGETSSHARED_EXPORT TargetingForm : public QWidget
    {
        Q_OBJECT

    public:
        explicit TargetingForm(QWidget *parent = 0);
        ~TargetingForm();

        void setTitle( const QString&, bool enableCharge = false, bool enableLimits = false  );

        void getContents( adcontrols::TargetingMethod& );
        void setContents( const adcontrols::TargetingMethod& );
        void getContents( adcontrols::MetIdMethod& );
        void setContents( const adcontrols::MetIdMethod& );

    private:
        class impl;
        std::unique_ptr< impl > impl_;
#if TARGETING_FORM_LOCAL_IMPL
        std::unique_ptr< impl >& ui;
#else
        ::Ui::TargetingForm * ui;
#endif

    signals:
        void triggerProcess();
        void polarityToggled( adcontrols::ion_polarity );
    };

}

#endif // TARGETINGFORM_HPP
