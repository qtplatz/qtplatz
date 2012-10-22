/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
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

#include "sdfileview.hpp"
#include "ui_sdfileview.h"
#include "sdfilemodel.hpp"
#include "sdfiledelegate.hpp"
#include <adchem/mol.hpp>
#include <openbabel/obconversion.h>
#include <openbabel/mol.h>

#include <qdebug.h>

using namespace chemistry;

SDFileView::SDFileView(QWidget *parent) :  QWidget(parent)
	                                    , ui(new Ui::SDFileView)
										, model_( new SDFileModel( this ) )
										, delegate_( new SDFileDelegate )
{
    ui->setupUi(this);
	ui->tableView->setModel( model_ );
	ui->tableView->setItemDelegate( delegate_ );
	ui->tableView->verticalHeader()->setDefaultSectionSize( 80 );
    ui->tableView->horizontalHeader()->setDefaultSectionSize( 200 );

	connect( ui->tableView->verticalHeader(), SIGNAL( sectionClicked( int ) ), this, SLOT( handleRawClicked( int ) ) );
}

SDFileView::~SDFileView()
{
	delete delegate_;
	delete model_;
    delete ui;
}

void
SDFileView::file( boost::shared_ptr< ChemFile >& file )
{
	model_->file( file );
}

void
SDFileView::setData( const std::vector< adchem::Mol >& v )
{
	model_->data( v );
}

void
SDFileView::handleRawClicked( int raw )
{
	emit rawClicked( raw, model_ );
}
