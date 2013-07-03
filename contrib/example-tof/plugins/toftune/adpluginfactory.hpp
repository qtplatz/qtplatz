/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#ifndef PLUGINFACTORY_HPP
#define PLUGINFACTORY_HPP

#include <adplugin/widget_factory.hpp>
#include <adplugin/plugin.hpp>

namespace toftune {

	class adpluginfactory : public adplugin::widget_factory
	                      , public adplugin::plugin {
	public:
		adpluginfactory();
        static adpluginfactory * instance();

        // adplugin::widget_factory
		virtual QWidget * create_widget( const wchar_t * iid, QWidget * parent );
		virtual QObject * create_object( const wchar_t * iid, QObject * parent );
        virtual void release();

        // adplugin::plugin
        virtual void accept( adplugin::visitor&, const char * adpluginspec );
        virtual const char * iid() const;

    private:
        virtual void * query_interface_workaround( const char * _typenam );
        static adpluginfactory * instance_;
	};

}

#endif // PLUGINFACTORY_HPP
