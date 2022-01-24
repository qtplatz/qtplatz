/**************************************************************************
** Copyright (C) 2020 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include <adportable/optional.hpp>
#include <compiler/boost/workaround.hpp>
#include <boost/python.hpp>


namespace py_module {

    class IsotopeCluster {
    public:
        ~IsotopeCluster();
        IsotopeCluster( const std::string& formulae );
        IsotopeCluster( const IsotopeCluster& );

        void setResolvingPower( double );
        double resolvingPower() const;

        bool assign( const std::string& );
        bool append( const std::string& );
        std::string formula() const;
        void setCharge( int );
        int charge() const;
        std::vector< boost::python::tuple > compute() const; // mass,abundance,formula-index

    private:
        std::string formula_;
        int charge_;
        double resolvingPower_;
    };
}
