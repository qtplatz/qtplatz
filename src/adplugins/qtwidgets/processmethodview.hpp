/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
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

#ifndef PROCESSMETHODVIEW_HPP
#define PROCESSMETHODVIEW_HPP

// #include <QDeclarativeView>
#include <adplugin/lifecycle.hpp>
#include <adcontrols/processmethod.hpp>
#include "centroiddelegate.hpp"
#include <boost/scoped_ptr.hpp>
#include "centroidmethodmodel.hpp"
#include "isotopemethodmodel.hpp"
#include "elementalcompmodel.hpp"

namespace adportable { class Configuration; }

namespace qtwidgets {

    class ProcessMethodView : public QWidget
                            , public adplugin::LifeCycle {
        
        Q_OBJECT
    public:
        explicit ProcessMethodView(QWidget *parent = 0);
        ~ProcessMethodView();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& );
        void OnInitialUpdate();
        void OnFinalClose();
        bool getContents( boost::any& ) const;
        bool setContents( boost::any& );
        //<--

        // QWidget
        virtual QSize sizeHint() const;
        
    signals:
            
    public slots:
        void getContents( adcontrols::ProcessMethod& );
        void getLifeCycle( adplugin::LifeCycle *& p );
        
    private:
        adportable::Configuration * pConfig_;
        boost::scoped_ptr< CentroidMethodModel > pCentroidModel_;
        boost::scoped_ptr< IsotopeMethodModel > pIsotopeModel_;
        boost::scoped_ptr< ElementalCompModel > pElementalCompModel_;
    };
    
}

#endif // PROCESSMETHODVIEW_HPP
