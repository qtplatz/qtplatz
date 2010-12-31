/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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

#include "peak.h"
#include "import_sagraphics.h"

using namespace adwidgets::ui;
using namespace adwidgets::ui::internal;

Peak::~Peak()
{
  if ( pi_ )
    pi_->Release();
}

Peak::Peak( SAGRAPHICSLib::ISADPPeak * pi ) : pi_(pi)
{
  pi_->AddRef();
}

Peak::Peak( const Peak& t )
{
   if ( t.pi_ )
     t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
   if ( pi_ )
     pi_->Release();
   pi_ = t.pi_;
}

bool
Peak::visible() const
{
    return variant_bool::to_native( pi_->Visible );
}

void
Peak::visible( bool value )
{
    pi_->Visible = variant_bool::to_variant( value );
}

double
Peak::startX() const
{
    return pi_->StartX;
}

void
Peak::startX( double value )
{
    pi_->StartX = value;
}

double
Peak::startY() const
{
    return pi_->StartY;
}

void
Peak::startY( double value )
{
    pi_->StartY = value;
}


double
Peak::endX() const
{
    return pi_->EndX;
}

void
Peak::endX( double value )
{
    pi_->EndX = value;
}


double
Peak::endY() const
{
    return pi_->EndY;
}

void
Peak::endY( double value )
{
    pi_->EndY = value;
}


double
Peak::centreX() const
{
    return pi_->CenterX;
}

void
Peak::centreX( double value )
{
    pi_->CenterX = value;
}


double
Peak::centreY() const
{
    return pi_->CenterY;
}

void
Peak::centreY( double value )
{
    pi_->CenterY = value;
}


double
Peak::baselineStartY() const
{
    return pi_->BaselineStartY;
}

void
Peak::baselineStartY( double value )
{
    pi_->BaselineStartY = value;
}


double
Peak::baselineEndY() const
{
    return pi_->BaselineEndY;
}

void
Peak::baselineEndY( double value )
{
    pi_->BaselineEndY = value;
}

double
Peak::baselineCentreY() const
{
    return pi_->BaselineCenterY;
}

void
Peak::baselineCentreY( double value )
{
    pi_->BaselineCenterY = value;
}

bool
Peak::drawBaseline() const
{
    return variant_bool::to_native( pi_->DrawBaseline );
}

void
Peak::drawBaseline( bool value )
{
    pi_->DrawBaseline = variant_bool::to_variant( value );
}

bool
Peak::drawBaselineCentre() const
{
    return variant_bool::to_native( pi_->DrawBaselineCenter );
}

void
Peak::drawBaselineCentre( bool value )
{
    pi_->DrawBaselineCenter = variant_bool::to_variant( value );
}

bool
Peak::peakFill() const
{
    return variant_bool::to_native( pi_->PeakFill );
}

void
Peak::peakFill( bool value )
{
    pi_->PeakFill = variant_bool::to_variant( value );
}


short
Peak::colorIndex() const
{
    return pi_->ColorIndex;
}

void
Peak::colorIndex( short value )
{
    pi_->ColorIndex = value;
}


bool
Peak::marked() const
{
    return variant_bool::to_native( pi_->Marked );
}

void
Peak::marked( bool value )
{
    pi_->Marked = variant_bool::to_variant( value );
}

short
Peak::fillStyle()
{
    return pi_->FillStyle;
}

void
Peak::fillStyle( short value )
{
    pi_->FillStyle = value;
}


short
Peak::fillColorIndex()
{
    return pi_->FillColorIndex;
}

void
Peak::fillColorIndex( short value )
{
    pi_->FillColorIndex = value;
}


PeakMarkerStyle
Peak::startMarkerStyle() const
{
    return static_cast< PeakMarkerStyle >( pi_->StartMarkerStyle );
}

void
Peak::startMarkerStyel( PeakMarkerStyle value )
{
    pi_->StartMarkerStyle = static_cast< SAGRAPHICSLib::PeakMarkerStyle >( value );
}


PeakMarkerStyle
Peak::endMarkerStyle() const
{
    return static_cast< PeakMarkerStyle >( pi_->EndMarkerStyle );
}

void
Peak::endMarkerStyle( PeakMarkerStyle value )
{
    pi_->EndMarkerStyle = static_cast<SAGRAPHICSLib::PeakMarkerStyle>(value);
}


PeakMarkerStyle
Peak::centreMarkerStyle() const
{
    return static_cast< PeakMarkerStyle >( pi_->CenterMarkerStyle );
}

void
Peak::centreMarkerStyle( PeakMarkerStyle value )
{
    pi_->CenterMarkerStyle = static_cast< SAGRAPHICSLib::PeakMarkerStyle >( value );
}

short
Peak::startMarkerColorIndex() const
{
    return pi_->StartMarkerColorIndex;
}

void
Peak::startMarkerColorIndex( short value )
{
    pi_->StartMarkerColorIndex = value;
}


short
Peak::endMarkerColorIndex() const
{
    return pi_->EndMarkerColorIndex;
}

void
Peak::endMarkerColorIndex( short value )
{
    pi_->EndMarkerColorIndex = value;
}

short
Peak::centreMarkerColorIndex() const
{
    return pi_->CenterMarkerColorIndex;
}

void
Peak::centreMarkerColroIndex( short value )
{
    pi_->CenterMarkerColorIndex = value;
}

