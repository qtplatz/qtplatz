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

#ifndef MSCALIBRATEWIDGET_HPP
#define MSCALIBRATEWIDGET_HPP

#include "adwidgets_global.hpp"
#include <adplugin/lifecycle.hpp>
#include <QWidget>

namespace adcontrols { class MSCalibrateMethod; }

namespace adwidgets {

    class MSCalibrateForm;
    class MSReferenceTable;

    class ADWIDGETSSHARED_EXPORT MSCalibrateWidget : public QWidget
                                                   , public adplugin::LifeCycle {
        Q_OBJECT
    public:
        explicit MSCalibrateWidget(QWidget *parent = 0);

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& ) override;
        void OnInitialUpdate() override;
        void onUpdate( boost::any& ) override;
        void OnFinalClose() override;
        bool getContents( boost::any& ) const override;
        bool setContents( boost::any& ) override;   

        //
        void loadFromUI( adcontrols::MSCalibrateMethod& );
        void saveToUI( const adcontrols::MSCalibrateMethod& );

    private:
        MSCalibrateForm * form_;
        MSReferenceTable * table_;

    signals:
        void onProcess( const QString& );

    public slots:

    };

}

#endif // MSCALIBRATEWIDGET_HPP
