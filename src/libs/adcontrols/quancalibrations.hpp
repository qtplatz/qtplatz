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

#ifndef QUANCALIBRATIONS_HPP
#define QUANCALIBRATIONS_HPP

#include "adcontrols_global.h"
#include "idaudit.hpp"
#include "quancalibration.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <memory>
#include <string>

namespace adcontrols {

#if defined _MSC_VER
    template class ADCONTROLSSHARED_EXPORT std::vector < QuanCalibration > ;
#endif

    class ADCONTROLSSHARED_EXPORT QuanCalibrations  {
    public:
        ~QuanCalibrations();
        QuanCalibrations();
        QuanCalibrations( const QuanCalibrations& );

        QuanCalibrations& operator << ( const QuanCalibration& );

        typedef std::vector< QuanCalibration >::iterator iterator;
        typedef std::vector< QuanCalibration >::const_iterator const_iterator;

        iterator begin() { return values_.begin(); }
        iterator end() { return values_.end(); }
        const_iterator begin() const { return values_.begin(); }
        const_iterator end() const { return values_.end(); }
        size_t size() const { return values_.size(); }
        void clear() { values_.clear(); }

        const idAudit& ident() const { return ident_; }
        
    private:
        std::vector< QuanCalibration > values_;
        idAudit ident_;
    };

}

#endif // QUANCALIBRATIONS_HPP
