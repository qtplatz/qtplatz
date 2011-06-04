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

#include "isotopeform.hpp"
#include "ui_isotopeform.h"
#include "isotopedelegate.hpp"
#include "standarditemhelper.hpp"
#include <adcontrols/isotopemethod.hpp>
#include <adcontrols/processmethod.hpp>
#include <adportable/configuration.hpp>
#include <QStandardItemModel>

using namespace qtwidgets;

IsotopeForm::IsotopeForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::IsotopeForm)
    , pModel_( new QStandardItemModel )
    , pConfig_( new adportable::Configuration )
    , pMethod_( new adcontrols::IsotopeMethod ) 
{
    ui->setupUi(this);
    ui->treeView->setModel( pModel_.get() );
}

IsotopeForm::~IsotopeForm()
{
    delete ui;
}

void
IsotopeForm::OnCreate( const adportable::Configuration& config )
{
    *pConfig_ = config;
}

void
IsotopeForm::OnInitialUpdate()
{
    QStandardItemModel& model = *pModel_;
    adcontrols::IsotopeMethod& method = *pMethod_;
    (void)method;

    //------------ add dummy data for debug ---------------
    //-----------------------------------------------------
    // method.addFormula( adcontrols::IsotopeMethod::Formula( L"Xe", L"", 1, 1.0 ) );
    //-----------------------------------------------------

    QStandardItem * rootNode = model.invisibleRootItem();

    ui->treeView->setItemDelegate( pDelegate_.get() );

    rootNode->setColumnCount(2);
    model.setHeaderData( 0, Qt::Horizontal, "Isotope" );

    QStandardItem * item = StandardItemHelper::appendRow( rootNode, "Isotope" );
    (void)item;
}

void
IsotopeForm::OnFinalClose()
{
}

void
IsotopeForm::getContents( adcontrols::ProcessMethod& pm )
{
    pm.appendMethod< adcontrols::IsotopeMethod >( *pMethod_ );
}

QSize
IsotopeForm::sizeHint() const
{
    return QSize( 300, 250 );
}
