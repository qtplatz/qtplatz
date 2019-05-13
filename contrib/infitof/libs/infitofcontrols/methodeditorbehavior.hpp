/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "infitofcontrols_global.hpp"
#include <tuple>

namespace infitofcontrols {

    template<typename T> class method_archive;

    class INFITOFCONTROLSSHARED_EXPORT MethodEditorBehavior {
    public:
        MethodEditorBehavior();
        MethodEditorBehavior( const MethodEditorBehavior& );

        bool gateEditable( int /*0|1*/ ) const;
        bool injectEditable() const;
        bool gateEnable( int ) const;
        std::pair< double, double > gate( int ) const;
        std::pair< double, double > inject() const;

        void setGateEditable( int, bool );
        void setInjectEditable( bool );
        void setGateEnable( int, bool );
        void setGate( int, double, double );
        void setInject( double, double );

    private:
        std::tuple< bool, double, double, bool > gate1_;
        std::tuple< bool, double, double, bool > gate2_;
        std::tuple< bool, double, double > inject_;
    };

};
