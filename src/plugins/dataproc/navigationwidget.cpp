//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "navigationwidget.h"
#include <QLabel>
#include <QTreeView>
#include <QStandardItemModel>
#include <QVBoxLayout>

using namespace dataproc;

NavigationWidget::NavigationWidget(QWidget *parent) : QWidget(parent)
                                                    , pTreeView_( new QTreeView )
                                                    , pModel_( new QStandardItemModel )
                                                    , pTitle_( new QLabel ) 
{
    pTreeView_->setModel( pModel_.get() );
    this->setFocusProxy( pTreeView_.get() );

    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget( pTitle_.get() );
    layout->addWidget( pTreeView_.get() );
    pTitle_->setMargin( 5 );
    layout->setSpacing( 0 );
    layout->setContentsMargins( 0, 0, 0, 0 );
    setLayout( layout );
}
