// This is a -*- C++ -*- header.
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

#ifndef SEQUENCEWIDGET_H
#define SEQUENCEWIDGET_H

#include <QWidget>
#include <adplugin/lifecycle.hpp>
#if ! defined Q_MOC_RUN
#include <boost/smart_ptr.hpp>
#endif

class QStandardItemModel;
namespace adportable {
    class Configuration;
}

namespace Ui {
    class SequenceWidget;
}

namespace qtwidgets {

    class SequenceModel;

    class SequenceWidget : public QWidget
                         , public adplugin::LifeCycle {
        Q_OBJECT

    public:
        explicit SequenceWidget(QWidget *parent = 0);
        ~SequenceWidget();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& );
        void OnInitialUpdate();
        void OnFinalClose();
        //<--
    public slots:
        void getLifeCycle( adplugin::LifeCycle*& );


    private:
        Ui::SequenceWidget *ui;
        boost::scoped_ptr< QStandardItemModel > pModel_;
        boost::scoped_ptr< adportable::Configuration > pConfig_;
    };

}

#endif // SEQUENCEWIDGET_H
