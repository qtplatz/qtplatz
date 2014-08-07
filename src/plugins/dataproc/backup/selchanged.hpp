// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
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

#include <boost/variant.hpp>
#include <adutils/processeddata.hpp>

#ifdef DEBUG
# include <adportable/debug.hpp>
#endif

namespace dataproc {

    template<class Wnd> class selChanged : public boost::static_visitor<bool> {
        Wnd& wnd_;
    public:
        selChanged( Wnd& wnd ) : wnd_(wnd) { }

        template<typename T> bool operator ()( T& ) const { 
#ifdef DEBUG
            adportable::debug(__FILE__, __LINE__) << "selChanged(" << typeid(T).name() << ") will do nothing.";
#endif
            return false;
        }

        bool operator () ( adutils::MassSpectrumPtr& ptr ) const {   
            wnd_.draw1( ptr );
            return true;
        }

        bool operator () ( adutils::ChromatogramPtr& ptr ) const {
            wnd_.draw( ptr );
            return true;
        }

    };

}

