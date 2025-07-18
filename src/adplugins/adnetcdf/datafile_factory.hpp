// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include <adcontrols/datafile_factory.hpp>
#include <adplugin/plugin.hpp>

namespace adnetcdf {

    class datafile_factory : public adcontrols::datafile_factory
                           , public adplugin::plugin {
        static std::shared_ptr< datafile_factory > instance_;
    public:
        ~datafile_factory(void);
        datafile_factory(void);

        //static datafile_factory * instance();
        static adplugin::plugin * instance();

        // adcontrols::datafile_factory
		const char * mimeTypes() const override;
        const wchar_t * name() const override;
        bool access( const wchar_t * filename, adcontrols::access_mode ) const override;
        bool access( const std::filesystem::path& filename, adcontrols::access_mode ) const override;

        adcontrols::datafile * open( const wchar_t * filename, bool readonly ) const override;
        adcontrols::datafile * open( const std::filesystem::path& filename, bool readonly ) const override;

        void close( adcontrols::datafile * ) override;

        // adplugin::plugin
        const char * iid() const override;
        void accept( adplugin::visitor& v, const char * adplugin ) override;
    private:
        void * query_interface_workaround( const char * typnam ) override;
    };

}
