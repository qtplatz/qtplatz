/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#pragma once

#include <QWidget>
#include "adwidgets_global.hpp"
#include <adplugin/lifecycle.hpp>

class QBoxLayout;

namespace adcontrols { namespace controlmethod { class MethodItem; } }
namespace adportable { class Configuration; }

namespace adwidgets {

    class  ADWIDGETSSHARED_EXPORT ControlMethodContainer : public QWidget
                                                          , public adplugin::LifeCycle {

        Q_OBJECT

    public:
        explicit ControlMethodContainer( QWidget *parent = 0 );
        virtual ~ControlMethodContainer();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& );
        void OnInitialUpdate();
        void OnFinalClose();
        bool getContents( boost::any& ) const;
        bool setContents( boost::any& );
        // LifeCycle

        void addWidget( QWidget *, const QString& label );
        QWidget * widget();

    private:
        QBoxLayout * layout_;

    signals:
            
    public slots:
        
    };

}

