//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "factory.h"
#include <adplugin/adplugin.h>
#include <QtCore/qplugin.h>
#include "logwidget.h"
#include "sequencewidget.h"
#include "sequencesform.h"
#include "centroidform.h"
#include "elementalcompositionform.h"
#include "mscalibrationform.h"
#include "peakidtableform.h"
#include "targetingform.h"
#include "reportform.h"
#include "mslockform.h"
#include "chromatographicpeakform.h"
#include "isotopeform.h"

using namespace qtwidgets;

QWidget *
factory::create_widget( const wchar_t * iid, QWidget * parent )
{
    if ( std::wstring(iid) == adplugin::iid_iLog ) {
        return new LogWidget( parent );
    } else if ( std::wstring( iid ) == adplugin::iid_iSequence ) {
        return new qtwidgets::SequenceWidget;
    } else if ( std::wstring( iid ) == adplugin::iid_iSequencesForm ) {
        return new qtwidgets::SequencesForm;
    } else if ( std::wstring( iid ) == L"qtwidgets::CentroidForm" ) {
        return new qtwidgets::CentroidForm;
    } else if ( std::wstring( iid ) == L"qtwidgets::ElementalCompositionForm" ) {
        return new qtwidgets::ElementalCompositionForm;
    } else if ( std::wstring( iid ) == L"qtwidgets::IsotopeForm" ) {
        return new qtwidgets::IsotopeForm;
    } else if ( std::wstring( iid ) == L"qtwidgets::MSCalibrationForm" ) {
        return new qtwidgets::MSCalibrationForm;
    } else if ( std::wstring( iid ) == L"qtwidgets::TargetingForm" ) {
        return new qtwidgets::TargetingForm;
    } else if ( std::wstring( iid ) == L"qtwidgets::MSLockForm" ) {
        return new qtwidgets::MSLockForm;
    } else if ( std::wstring( iid ) == L"qtwidgets::PeakMethodForm" ) {
        return new qtwidgets::ChromatographicPeakForm;
    } else if ( std::wstring( iid ) == L"qtwidgets::PeakIDTableForm" ) {
        return new qtwidgets::PeakIDTableForm;
    } else if ( std::wstring( iid ) == L"qtwidgets::ReportForm" ) {
        return new qtwidgets::ReportForm;
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

EXPORT_FACTORY( factory )
