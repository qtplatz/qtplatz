/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "sideframe.hpp"
#include <QMessageBox>

using namespace toftune;

SideFrame::SideFrame(QWidget *parent) : QWidget(parent)
{
}

void
SideFrame::OnCreate()
{
}

void
SideFrame::OnInitialUpdate()
{
    //QDeclarativeContext * ctx = rootContext();
	//setResizeMode( QDeclarativeView::SizeRootObjectToView );
	//setSource( QUrl( "qrc:files/qml/SideFrame.qml" ));

	//QList< QDeclarativeError > errors = this->errors();
	//for ( QList< QDeclarativeError >::const_iterator it = errors.begin(); it != errors.end(); ++it )
    //    QMessageBox::warning( this, "QDeclarativeError", it->description() );
}

void
SideFrame::OnFinalClose()
{
}
