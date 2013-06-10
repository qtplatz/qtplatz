/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#ifndef LIFECYCLEACCESSOR_HPP
#define LIFECYCLEACCESSOR_HPP

#include "adplugin_global.h"
#include <QObject>

namespace adplugin {

    class LifeCycle;

    class ADPLUGINSHARED_EXPORT LifeCycleAccessor : public QObject {
        Q_OBJECT
    private:
        QObject * pObject_;
        adplugin::LifeCycle * p_;
        bool conn_;
    public:
        explicit LifeCycleAccessor(QObject *target);
        ~LifeCycleAccessor();

        LifeCycle * get();

    signals:
        void trigger( adplugin::LifeCycle *& );

    public slots:


    };

}

#endif // LIFECYCLEACCESSOR_HPP
