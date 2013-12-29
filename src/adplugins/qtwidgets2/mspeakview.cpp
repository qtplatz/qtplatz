/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "mspeakview.hpp"
#include "mspeaksummary.hpp"
#include "mspeaktable.hpp"
#include <QSplitter>
#include <QBoxLayout>

using namespace qtwidgets2;

MSPeakView::MSPeakView(QWidget *parent) : QWidget(parent)
                                        , peakSummary_( new MSPeakSummary )
                                        , peakTable_( new MSPeakTable )
{
    if ( QSplitter * splitter = new QSplitter ) {

        splitter->addWidget( peakSummary_.get() );
        splitter->addWidget( peakTable_.get() );
        splitter->setOrientation( Qt::Horizontal );

        QVBoxLayout * layout = new QVBoxLayout( this );
        layout->setMargin( 0 );
        layout->setSpacing( 2 );
        layout->addWidget( splitter );
    }
}

void *
MSPeakView::query_interface_workaround( const char * typenam )
{
    if ( typenam == typeid( MSPeakView ).name() )
        return static_cast< MSPeakView * >(this);
    return 0;
}

void
MSPeakView::OnCreate( const adportable::Configuration& )
{
}

void
MSPeakView::OnInitialUpdate()
{
}

void
MSPeakView::onUpdate( boost::any& )
{
}

void
MSPeakView::OnFinalClose()
{
}

bool
MSPeakView::getContents( boost::any& ) const
{
    return false;
}

bool
MSPeakView::setContents( boost::any& )
{
    return false;
}

