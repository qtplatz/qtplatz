/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
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

#ifndef ISOTOPEFORM_HPP
#define ISOTOPEFORM_HPP

#include <QWidget>
#include <adplugin/lifecycle.hpp>
#include <adcontrols/ctable.hpp>
#include <vector>
#include <memory>

class QStandardItemModel;
class QModelIndex;

namespace adcontrols {
	class ProcessMethod;
	class IsotopeMethod;
}

namespace Ui {
	class IsotopeForm;
}

namespace qtwidgets {

    class IsotopeDelegate;

	class IsotopeForm : public QWidget
		              , adplugin::LifeCycle {
		Q_OBJECT
    
	public:
		explicit IsotopeForm(QWidget *parent = 0);
		~IsotopeForm();

		// adplugin::LifeCycle
        virtual void OnCreate( const adportable::Configuration& );
        virtual void OnInitialUpdate();
		//virtual void OnUpdate( boost::any& );
		//virtual void OnUpdate( unsigned long );
        virtual void OnFinalClose();
        bool getContents( boost::any& ) const;
        bool setContents( boost::any& );

		// <--

    
	private:
		Ui::IsotopeForm *ui;
		std::unique_ptr< adcontrols::IsotopeMethod > pMethod_;
		std::unique_ptr< QStandardItemModel > pModel_;
		std::unique_ptr< IsotopeDelegate > pDelegate_;
		std::vector< std::pair< QString, adcontrols::CTable > > ctabs_;

    public slots:
        void getLifeCycle( adplugin::LifeCycle *& p );
        void getContents( adcontrols::ProcessMethod& );

    private slots:
	   void onMolChanged( QString );
	   void onCurrentChanged( const QModelIndex& curr, const QModelIndex& prev );
	   void onActivated( const QModelIndex& curr );

    signals:
	   void apply( adcontrols::ProcessMethod & );

	};

}

#endif // ISOTOPEFORM_HPP
