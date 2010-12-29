/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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

#pragma once

#include <boost/variant.hpp>
#include <adutils/processeddata.h>

namespace dataproc {
    namespace internal {

        template<class Wnd> class selChanged : public boost::static_visitor<void> {
            Wnd& wnd_;
        public:
            selChanged( Wnd& wnd ) : wnd_(wnd) { }

            template<typename T> void operator ()( T& ) const {
            }

            template<> void operator () ( adutils::MassSpectrumPtr& ptr ) const {   
                wnd_.draw1( ptr );
            }

            template<> void operator () ( adutils::ChromatogramPtr& ptr ) const {
                wnd_.draw( ptr );
            }

        };

    }
}

