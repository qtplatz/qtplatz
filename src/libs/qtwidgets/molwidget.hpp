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

#ifndef MOLWIDGET_HPP
#define MOLWIDGET_HPP

#include <QWidget>
#include <adcontrols/ctable.hpp>
#include <vector>

namespace qtwidgets {

	class MolWidget : public QWidget {
		Q_OBJECT
	public:
		explicit MolWidget(QWidget *parent = 0);

		bool getCTable( const QString&, adcontrols::CTable& );
		bool draw( const adcontrols::CTable& );

	protected:
        void dragEnterEvent( QDragEnterEvent * );
        void dropEvent( QDropEvent * );
		void paintEvent( QPaintEvent * );

	private:
		typedef std::pair< QString, adcontrols::CTable > key_ctable_pair_t;
		std::vector< key_ctable_pair_t > ctabs_;

	public:
    
    signals:
		void molChanged( QString );
    
	public slots:
    
    };

}

#endif // MOLWIDGET_HPP
