/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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
#include <boost/smart_ptr.hpp>

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

	class PeakMethodDelegate;

	class PeakMethodForm : public QWidget
		                 , public adplugin::LifeCycle {
		Q_OBJECT
    
	public:
		explicit PeakMethodForm(QWidget *parent = 0);
		~PeakMethodForm();

        // adplugin::LifeCycle
		virtual void OnCreate( const adportable::Configuration& );
		virtual void OnInitialUpdate();
		virtual void OnFinalClose();
        //<--
	public slots:
        void getContents( adcontrols::ProcessMethod& );
        void getLifeCycle( adplugin::LifeCycle*& );
		//--
		// void peakMethodChanged( const QModelIndex& );

	private:
		Ui::PeakMethodForm *ui;

        boost::scoped_ptr< QStandardItemModel > pModel_;
        boost::scoped_ptr< adportable::Configuration > pConfig_;
        boost::scoped_ptr< adcontrols::PeakMethod > pMethod_;
		boost::scoped_ptr< PeakMethodDelegate > pDelegate_;
	};

}

#endif // PEAKMETHODFORM_HPP
