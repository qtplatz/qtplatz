/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#include "waveform.hpp"

namespace adcontrols {

    template<> class waveform<int32_t>::impl {
    public:
        
    };

}

using namespace adcontrols;

waveform<int32_t>::waveform() : impl_( new impl() )
{
}

template<>
waveform<int32_t>::~waveform()
{
}

//static
template<> ADCONTROLSSHARED_EXPORT std::shared_ptr< waveform< int32_t > >
waveform<int32_t>::make_this()
{
    struct make_shared_enabler : public waveform< int32_t > {};
    return std::make_shared< make_shared_enabler >();
}


