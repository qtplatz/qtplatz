/**************************************************************************
** Copyright (C) 2014-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2014-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <adextension/icontroller.hpp>
#include <mutex>
#include <condition_variable>

namespace ap240 {

    class iControllerImpl : public adextension::iController {
        Q_OBJECT
    public:
        iControllerImpl();
        ~iControllerImpl();
        bool connect() override;
        bool wait_for_connection_ready() override;
        bool preparing_for_run( adcontrols::ControlMethod& ) override;
        QString module_name() const override { return "ap240"; }
        int module_number() const override { return 1; }            
        
        // local methods
        void setInitialized( bool );
        
    signals:
            
    private slots:
        
    private:
        bool isInitialized_;
        std::mutex mutex_;
        std::condition_variable cv_;
        class impl;
        impl * impl_;
    };
}

