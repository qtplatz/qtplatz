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

#ifndef DATAMEDIATOR_HPP
#define DATAMEDIATOR_HPP

#include <string>

namespace tof { class ControlMethod; }  // in method.hpp

namespace toftune {

    class dataMediator {
    public:
        dataMediator();

        virtual void setMethod( const tof::ControlMethod& ) = 0;
        virtual void getMethod( tof::ControlMethod& ) const = 0;

        bool isInProgress() const { return inprogress_; }
        std::string hint() const;

    protected:
        bool inprogress_;
        void hint( const std::string& );

        struct progress_lock {
            dataMediator& x_;
            progress_lock( dataMediator& x ) : x_( x ) { x_.inprogress_ = true; }
            ~progress_lock() { x_.inprogress_ = false; }
        };
    private:
        std::string hint_;
    };

}

#endif // DATAMEDIATOR_HPP
