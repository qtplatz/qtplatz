/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#ifndef MONITORUI_HPP
#define MONITORUI_HPP

#include <adplugin/imonitor.hpp>
#include <adportable/configuration.hpp>
#include <memory>
#include <QWidget>

namespace Ui {
class MonitorUI;
}

namespace toftune {

    class MonitorUI : public adplugin::ui::IMonitor {
        Q_OBJECT
    
	public:
        explicit MonitorUI(QWidget *parent = 0);
        ~MonitorUI();

        // IMonitor impl
        virtual void OnCreate( const adportable::Configuration& );
        virtual void OnInitialUpdate();
        virtual void OnUpdate( boost::any& );
        virtual void OnFinalClose();
        

    signals:
        void signal_log( QString, QString );
        void signal_messagee( unsigned long, unsigned long );
    protected slots:
        void handle_message( unsigned long, unsigned long );    
        void handleSampInterval( int );
        void handlePeakWidth( int );

    private:
        friend class Receiver_i;
        adportable::Configuration config_;
        class Receiver_i * receiver_i_;

        Ui::MonitorUI *ui;
    };

}

#endif // MONITORUI_HPP
