/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#ifndef MASSSPECTROMETERFACTORY_HPP
#define MASSSPECTROMETERFACTORY_HPP

#include <adcontrols/massspectrometer_factory.hpp>
#include <memory>

namespace adcontrols { class datafile; }

namespace batchproc {

    class MassSpectrometerFactory : public adcontrols::massspectrometer_factory {
    public:
        MassSpectrometerFactory();

        const wchar_t * name() const override;
        adcontrols::MassSpectrometer * get( const wchar_t * modelname ) override;
        std::shared_ptr< adcontrols::MassSpectrometer > create( const wchar_t * modelname, adcontrols::datafile * ) const override;

    private:
        std::unique_ptr< adcontrols::MassSpectrometer > spectrometer_;
    };
}

#endif // MASSSPECTROMETERFACTORY_HPP
