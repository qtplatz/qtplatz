/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#ifndef PEPTIDEWIDGET_HPP
#define PEPTIDEWIDGET_HPP

#include "adwidgets_global.hpp"
#include <adplugin_manager/lifecycle.hpp>
#include <QWidget>

namespace adwidgets {

    class TargetingForm;
	class TargetingTable;
    class PeptideTable;

    class ADWIDGETSSHARED_EXPORT PeptideWidget : public QWidget
                                               , public adplugin::LifeCycle {
        Q_OBJECT
        Q_INTERFACES( adplugin::LifeCycle )

    public:
        explicit PeptideWidget(QWidget *parent = 0);
        ~PeptideWidget();

        static QWidget * create( QWidget * parent );

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& ) override;
        void OnInitialUpdate() override;
        void onUpdate( boost::any& ) override;
        void OnFinalClose() override;
        bool getContents( boost::any& ) const override;
        bool setContents( boost::any& ) override;   

    private:
        TargetingForm * form_;
        TargetingTable * table_;
        PeptideTable * peptideTable_;

    signals:
        void triggerFind( const QVector< QPair< QString, QString > >& ) const; // (sequence, formula)

    public slots:

    private slots:
        void showContextMenu( const QPoint& );

    };

}

#endif // PEPTIDEWIDGET_HPP
