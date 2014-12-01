/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef U5303AMETHODWIDGET_HPP
#define U5303AMETHODWIDGET_HPP

#include <QWidget>
#include <adplugin/lifecycle.hpp>

namespace u5303a {

    class u5303AMethodWidget : public QWidget
                             , public adplugin::LifeCycle {
        Q_OBJECT
    public:
        explicit u5303AMethodWidget(QWidget *parent = 0);

        // LifeCycle
        void OnCreate( const adportable::Configuration& ) override;
        void OnInitialUpdate() override;
        void OnFinalClose() override;
        //virtual void onUpdate( boost::any& ) {}
        virtual bool getContents( boost::any& ) const;
        virtual bool setContents( boost::any& );

        void onInitialUpdate();
        void onStatus( int );

    private:

    signals:

    public slots:
        void handle_trigger_apply();
        void getLifeCycle( adplugin::LifeCycle *& p );
    };

}

#endif // U5303AMETHODWIDGET_HPP
