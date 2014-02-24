/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
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

#ifndef PEAKMETHODFORM_HPP
#define PEAKMETHODFORM_HPP

#include <QWidget>
#include <adplugin/lifecycle.hpp>
#include <memory>

class QStandardItemModel;
class QStandardItem;
class QModelIndex;

namespace Ui {
	class PeakMethodForm;
}

namespace adcontrols {
	class PeakMethod;
    class ProcessMethod;
}

namespace qtwidgets {

	class TimeEventsDelegate;
    class PeakMethodDelegate;

	class PeakMethodForm : public QWidget
		                 , public adplugin::LifeCycle {
		Q_OBJECT
    
	public:
		explicit PeakMethodForm(QWidget *parent = 0);
		~PeakMethodForm();

        // adplugin::LifeCycle
		virtual void OnCreate( const adportable::Configuration& ) override;
		virtual void OnInitialUpdate() override;
		virtual void OnFinalClose() override;
        bool getContents( boost::any& ) const override;
        bool setContents( boost::any& ) override;

        //<--
	public slots:
        void getContents( adcontrols::ProcessMethod& ) const;
        void getLifeCycle( adplugin::LifeCycle*& );

    signals:
		void apply( adcontrols::ProcessMethod& );

    private slots:

    private:
		Ui::PeakMethodForm *ui;
        std::unique_ptr< adcontrols::PeakMethod > pMethod_;
        std::unique_ptr< QStandardItemModel > pTimeEventsModel_; // time events
		std::unique_ptr< TimeEventsDelegate > pTimeEventsDelegate_;
        std::unique_ptr< QStandardItemModel > pGlobalModel_; // time events
		std::unique_ptr< PeakMethodDelegate > pGlobalDelegate_;
        void setContents( const adcontrols::PeakMethod& );
        void getContents( adcontrols::PeakMethod& ) const;
	};

}

#endif // PEAKMETHODFORM_HPP
