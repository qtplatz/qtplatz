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

#ifndef IEDITORFACTORY_HPP
#define IEDITORFACTORY_HPP

#include "adextension_global.hpp"

class QWidget;
class QString;

namespace adextension {

    class ADEXTENSIONSHARED_EXPORT iEditorFactory {
    public:
        enum METHOD_TYPE { PROCESS_METHOD, CONTROL_METHOD };
		iEditorFactory();
		virtual ~iEditorFactory();
        virtual QWidget * createEditor( QWidget * pearent = 0 ) = 0;
        virtual QString title() const = 0;
        virtual METHOD_TYPE method_type() const { return PROCESS_METHOD; }
    };

}

#endif // IEDITORFACTORY_HPP
