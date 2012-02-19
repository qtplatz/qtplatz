// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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
#include "processmethodview.hpp"
#include "logwidget.hpp"
#include "sequencewidget.hpp"
#include "sequencesform.hpp"
#include "centroidform.hpp"
#include "elementalcompositionform.hpp"
#include "mscalibrationform.hpp"
#include "mscalibsummarywidget.hpp"
#include "peakresultwidget.hpp"
#include "molwidget.hpp"
#include "isotopeform.hpp"
#include <adplugin/lifecycle.hpp>
#include <QMessageBox>

using namespace qtwidgets;

QWidget *
factory::create_widget( const wchar_t * iid, QWidget * parent )
{
    QWidget * pWidget = 0;
    if ( std::wstring(iid) == iid_iLog ) {
        pWidget = new LogWidget( parent );
    } else if ( std::wstring( iid ) == iid_iSequence ) {
        pWidget = new qtwidgets::SequenceWidget( parent );
    } else if ( std::wstring( iid ) == iid_iSequencesForm ) {
        pWidget = new qtwidgets::SequencesForm( parent );
    } else if ( std::wstring( iid ) == L"qtwidgets::CentroidForm" ) {
        pWidget = new qtwidgets::CentroidForm( parent );
    } else if ( std::wstring( iid ) == L"qtwidgets::ElementalCompositionForm" ) {
        pWidget = new qtwidgets::ElementalCompositionForm( parent );
    } else if ( std::wstring( iid ) == L"qtwidgets::MSCalibrationForm" ) {
        pWidget = new qtwidgets::MSCalibrationForm( parent );
    } else if ( std::wstring( iid ) == L"qtwidgets::MSCalibSummaryWidget" ) {
        pWidget = new qtwidgets::MSCalibSummaryWidget( parent );
    } else if ( std::wstring( iid ) == L"qtwidgets::PeakResultWidget" ) {
        pWidget = new qtwidgets::PeakResultWidget( parent );
    } else if ( std::wstring( iid ) == L"qtwidgets::ProcessMethodView" ) {
        pWidget = new qtwidgets::ProcessMethodView( parent );
    } else if ( std::wstring( iid ) == L"qtwidgets::MolWidget" ) {
        pWidget = new qtwidgets::MolWidget( parent );
    } else if ( std::wstring( iid ) == L"qtwidgets::IsotopeForm" ) {
        pWidget = new qtwidgets::IsotopeForm( parent );
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

EXPORT_FACTORY( qtwidgets::factory )
