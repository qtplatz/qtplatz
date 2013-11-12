/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "filepropertywidget.hpp"
#include <adportable/is_type.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <coreplugin/minisplitter.h>
#include <QTreeView>
#include <QTextEdit>
#include <QStandardItemModel>
#include <QVBoxLayout>

using namespace dataproc;

FilePropertyWidget::FilePropertyWidget(QWidget *parent) : QWidget(parent)
                                                        , textEdit_( new QTextEdit )
                                                        , treeView_( new QTreeView )
                                                        , model_( new QStandardItemModel )
{
    if ( Core::MiniSplitter * splitter = new Core::MiniSplitter ) {

        treeView_->setModel( model_.get() );
        splitter->addWidget( treeView_.get() );
        splitter->addWidget( textEdit_.get() );

        auto layout = new QVBoxLayout( this );
        layout->setMargin( 0 );
        layout->setSpacing( 2 );
        layout->addWidget( splitter );

    }
}

FilePropertyWidget::~FilePropertyWidget()
{
}

// adplugin::LifeCycle
void
FilePropertyWidget::OnCreate( const adportable::Configuration& )
{
}

void
FilePropertyWidget::OnInitialUpdate()
{
}

void
FilePropertyWidget::OnFinalClose()
{
}

bool
FilePropertyWidget::getContents( boost::any& ) const
{
    return false;
}

bool
FilePropertyWidget::setContents( boost::any& a )
{
    if ( adportable::a_type< adcontrols::MSCalibrateResult >::is_a( a ) ) {
        const adcontrols::MSCalibrateResult& calibResult = boost::any_cast< const adcontrols::MSCalibrateResult& >( a );
        (void)calibResult;
        return true;
    }
    return false;
}



