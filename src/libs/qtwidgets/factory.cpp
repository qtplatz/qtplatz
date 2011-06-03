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
#include "logwidget.hpp"
#include "sequencewidget.hpp"
#include "sequencesform.hpp"
#include "centroidform.hpp"
#include "elementalcompositionform.hpp"
#include "mscalibrationform.hpp"
#include "peakidtableform.hpp"
#include "targetingform.hpp"
#include "reportform.hpp"
#include "mslockform.hpp"
#include "chromatographicpeakform.hpp"
#include "isotopeform.hpp"
#include "mscalibsummarywidget.hpp"
#include "peakresultwidget.hpp"
#include <adplugin/lifecycle.hpp>

using namespace qtwidgets;

QWidget *
factory::create_widget( const wchar_t * iid, QWidget * parent )
{
    adportable::debug dbg(__FILE__, __LINE__);
    dbg << L"qtwidgets::factory::create_widget(" << iid << ")";
    qDebug() << dbg.str().c_str();

    if ( std::wstring(iid) == iid_iLog ) {
        return new LogWidget( parent );
    } else if ( std::wstring( iid ) == iid_iSequence ) {
        return new qtwidgets::SequenceWidget( parent );
    } else if ( std::wstring( iid ) == iid_iSequencesForm ) {
        return new qtwidgets::SequencesForm( parent );
    } else if ( std::wstring( iid ) == L"qtwidgets::CentroidForm" ) {
        return new qtwidgets::CentroidForm( parent );
    } else if ( std::wstring( iid ) == L"qtwidgets::ElementalCompositionForm" ) {
        return new qtwidgets::ElementalCompositionForm( parent );
    } else if ( std::wstring( iid ) == L"qtwidgets::IsotopeForm" ) {
        return new qtwidgets::IsotopeForm( parent );
    } else if ( std::wstring( iid ) == L"qtwidgets::MSCalibrationForm" ) {
        return new qtwidgets::MSCalibrationForm( parent );
    } else if ( std::wstring( iid ) == L"qtwidgets::TargetingForm" ) {
        return new qtwidgets::TargetingForm( parent );
    } else if ( std::wstring( iid ) == L"qtwidgets::MSLockForm" ) {
        return new qtwidgets::MSLockForm ( parent );
    } else if ( std::wstring( iid ) == L"qtwidgets::PeakMethodForm" ) {
        return new qtwidgets::ChromatographicPeakForm( parent );
    } else if ( std::wstring( iid ) == L"qtwidgets::PeakIDTableForm" ) {
        return new qtwidgets::PeakIDTableForm( parent );
    } else if ( std::wstring( iid ) == L"qtwidgets::ReportForm" ) {
        return new qtwidgets::ReportForm( parent );
    } else if ( std::wstring( iid ) == L"qtwidgets::MSCalibSummaryWidget" ) {
        return new qtwidgets::MSCalibSummaryWidget( parent );
    } else if ( std::wstring( iid ) == L"qtwidgets::PeakResultWidget" ) {
        return new qtwidgets::PeakResultWidget( parent );
    }
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
