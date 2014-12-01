/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "controlmethodcontainer.hpp"
#include <QBoxLayout>
#include <QLabel>

using namespace adwidgets;

ControlMethodContainer::~ControlMethodContainer()
{

}

ControlMethodContainer::ControlMethodContainer( QWidget * parent ) : QWidget( parent )
                                                                   , layout_(0)
{
    auto layout = new QVBoxLayout( this );
    layout->setMargin( 0 );
    layout->setSpacing( 0 );

    layout->addWidget( new QLabel );

    layout_ = new QHBoxLayout( this );
    layout->addLayout( layout_ );
}

void
ControlMethodContainer::OnCreate( const adportable::Configuration& )
{
}

void
ControlMethodContainer::OnInitialUpdate()
{
}

void
ControlMethodContainer::OnFinalClose()
{
}

bool
ControlMethodContainer::getContents( boost::any& ) const
{
    return false;
}

bool
ControlMethodContainer::setContents( boost::any& )
{
    return false;
}

void
ControlMethodContainer::addWidget( QWidget * widget, const QString& text )
{
    if ( auto label = findChild< QLabel * >() )
        label->setText( text );
    layout_->addWidget( widget );
}

QWidget *
ControlMethodContainer::widget()
{
    return 0;
}

