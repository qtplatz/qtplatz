// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#ifndef SEQUENCES_H
#define SEQUENCES_H

#include <QWidget>
#include <adplugin/lifecycle.hpp>
#include <adportable/configuration.hpp>
#include <boost/smart_ptr.hpp>

class QStandardItemModel;

namespace Ui {
    class SequencesForm;
}

namespace qtwidgets {

    class SequencesModel;

    class SequencesForm : public QWidget
                        , public adplugin::LifeCycle {
        Q_OBJECT
        
    public:
        explicit SequencesForm(QWidget *parent = 0);
        ~SequencesForm();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& );
        void OnInitialUpdate();
        void OnFinalClose();
        bool getContents( boost::any& ) const;
        bool setContents( boost::any& );


    public slots:
        void getLifeCycle( adplugin::LifeCycle*& );
        
    private:
        Ui::SequencesForm *ui;
        // qtwidgets::SequencesModel * pModel_;
        boost::scoped_ptr< QStandardItemModel > pModel_;
        adportable::Configuration config_;
        
    };
}

#endif // SEQUENCES_H
