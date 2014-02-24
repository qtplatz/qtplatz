/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#ifndef EDITORFACTORY_HPP
#define EDITORFACTORY_HPP

#include <adextension/ieditorfactory.hpp>
#include <adportable/configuration.hpp>
#include <string>

namespace dataproc {

	class EditorFactory : public adextension::iEditorFactory {
		std::wstring path_;
		adportable::Configuration config_;
    public:
        EditorFactory( const adportable::Configuration& config, const std::wstring& path );
        virtual QWidget * createEditor( QWidget * parent = 0 );
        virtual QString title() const;
	};

}

#endif // EDITORFACTORY_HPP
