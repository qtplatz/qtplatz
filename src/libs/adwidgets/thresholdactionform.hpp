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

#ifndef TRIGGERACTIONFORM_HPP
#define TRIGGERACTIONFORM_HPP

#include "adwidgets_global.hpp"
#include <adplugin_manager/lifecycle.hpp>
#include <QWidget>
#include <memory>

class QStandardItemModel;

namespace adcontrols { class threshold_action; class MassSpectrometer; }

namespace adwidgets {

    namespace Ui {
        class ThresholdActionForm;
    }

    class ADWIDGETSSHARED_EXPORT ThresholdActionForm : public QWidget
                                                     , adplugin::LifeCycle {

        Q_OBJECT
        Q_INTERFACES( adplugin::LifeCycle )

    public:
        explicit ThresholdActionForm( QWidget *parent = 0 );
        ~ThresholdActionForm();

        void OnCreate( const adportable::Configuration& ) override;
        void OnInitialUpdate() override;
        void OnFinalClose() override;
        void onUpdate( boost::any&& ) override;
        bool getContents( boost::any& ) const override;
        bool setContents( boost::any&& ) override;

        bool get( adcontrols::threshold_action& ) const;
        bool set( const adcontrols::threshold_action& );
        
        void setMassSpectrometer( std::shared_ptr< const adcontrols::MassSpectrometer > );
        std::shared_ptr< const adcontrols::MassSpectrometer > massSpectrometer() const;
            
    signals:
        void valueChanged();

    private:
        Ui::ThresholdActionForm *ui;
        std::weak_ptr< const adcontrols::MassSpectrometer > spectrometer_;
        void formulaChanged( const QString& );
        void modeChanged( int );
        void massChanged( double );
    };
}

#endif // TRIGGERACTIONFORM_HPP
