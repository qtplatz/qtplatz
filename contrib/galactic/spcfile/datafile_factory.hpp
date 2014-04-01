/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@MS-Cheminformatics.com
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

#ifndef DATAFILE_FACTORY_HPP
#define DATAFILE_FACTORY_HPP

#include <adcontrols/datafile_factory.hpp>
#include <adplugin/plugin.hpp>

namespace spcfile {
	class datafile_factory : public adcontrols::datafile_factory
	                       , public adplugin::plugin {
	public:
		datafile_factory();
		~datafile_factory(void);

		const char * mimeTypes() const;
        const wchar_t * name() const;
        bool access( const wchar_t * filename, adcontrols::access_mode ) const;
        adcontrols::datafile * open( const wchar_t * filename, bool readonly ) const;
        void close( adcontrols::datafile * );
		       // adplugin::plugin
    public:
        const char * iid() const;
        void accept( adplugin::visitor& v, const char * adplugin );
    private:
        void * query_interface_workaround( const char * typnam );
	};

}

#endif // DATAFILE_FACTORY_HPP
