// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: info@ms-cheminfo.com
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

#include "factory.hpp"
#include <adplugin/adplugin.hpp>
#include <adplugin/constants.hpp>
#include <adportable/debug.hpp>
#include <qdebug.h>
#include <QtCore/qplugin.h>
#include "mscalibrationform.hpp"
#include "mscalibsummarywidget.hpp"
#include <adplugin/lifecycle.hpp>
#include <QMessageBox>

using namespace qtwidgets2;

QWidget *
factory::create_widget( const wchar_t * iid, QWidget * parent )
{
    QWidget * pWidget = 0;
    if ( std::wstring( iid ) == L"qtwidgets2::MSCalibrationForm" ) {
        pWidget = new qtwidgets::MSCalibrationForm( parent );
    } else if ( std::wstring( iid ) == L"qtwidgets2::MSCalibSummaryWidget" ) {
        pWidget = new qtwidgets2::MSCalibSummaryWidget( parent );
    }
    if ( pWidget )
        return pWidget;
    adportable::debug dbg(__FILE__, __LINE__);
    dbg << "create_widget(" << std::wstring(iid) << ") -- no such class.";
    QMessageBox::warning( 0, QLatin1String("qtwidgets::factory"), dbg.str().c_str() );
    return 0;
}

QObject *
factory::create_object( const wchar_t * iid, QObject * parent )
{
    Q_UNUSED( parent );
    Q_UNUSED( iid );
    return 0;
}

void
factory::release()
{
}

EXPORT_FACTORY( qtwidgets2::factory )
