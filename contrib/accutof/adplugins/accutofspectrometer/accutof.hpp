/**************************************************************************
** Copyright (C) 2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2019 MS-Cheminformatics LLC
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

#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrometer_factory.hpp>
#include <adplugin/plugin.hpp>

namespace adcontrols {
    class Visitor;
}

namespace accutofspectrometer {

    class accutof : public adcontrols::MassSpectrometer
                  , public adcontrols::massspectrometer_factory
                  , public adplugin::plugin {
    private:
        ~accutof();
        accutof();
    public:
        static accutof * instance();
        static void dispose();

		// adcontrols::MassSpectrometer impl.
        //void accept( adcontrols::Visitor& ) override;
        const wchar_t * name() const override;
        void initialSetup( adfs::sqlite& dbf, const boost::uuids::uuid& objuuid ) override;

		//const adcontrols::ScanLaw& getScanLaw() const override;
		//const adcontrols::DataInterpreter& getDataInterpreter() const override;

		// adcontrols::massspectrometer_factory impl.
		// adcontrols::MassSpectrometer * get( const wchar_t * ) override;

		// plugin impl.
		const char * iid() const override;
		void accept( adplugin::visitor&, const char * );
		void * query_interface_workaround( const char * typenam );

    private:
        static accutof * instance_;
        adcontrols::ScanLaw * scanLaw_;
        adcontrols::DataInterpreter * interpreter_;
    };

}
