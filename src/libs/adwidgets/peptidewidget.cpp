/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#include "peptidewidget.hpp"
#include "targetingform.hpp"
#include "targetingtable.hpp"
#include "peptidetable.hpp"
#include <adportable/is_type.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <adprot/digestedpeptides.hpp>
#include <QSplitter>
#include <QBoxLayout>
#include <QMenu>
#include <QStandardItemModel>
#include <set>

using namespace adwidgets;

PeptideWidget::PeptideWidget(QWidget *parent) : QWidget(parent)
                                              , form_(0)
                                              , table_(0)
                                              , peptideTable_(0)
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setMargin(0);
        layout->setSpacing(0);
        
        if ( QSplitter * splitter = new QSplitter ) {

            splitter->addWidget( ( form_ = new TargetingForm ) ); 
            splitter->addWidget( ( table_ = new TargetingTable ) ); 
            splitter->addWidget( ( peptideTable_ = new PeptideTable ) ); 
            splitter->setStretchFactor( 0, 0 );
            splitter->setStretchFactor( 1, 1 );
            splitter->setStretchFactor( 2, 1 );
            splitter->setOrientation ( Qt::Horizontal );

            layout->addWidget( splitter );
        }
    }
    table_->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( table_, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( showContextMenu( const QPoint& ) ) );

}

PeptideWidget::~PeptideWidget()
{
    delete table_;
    delete form_;
    delete peptideTable_;
}

QWidget *
PeptideWidget::create( QWidget * parent )
{
    return new PeptideWidget( parent );
}

void
PeptideWidget::OnCreate( const adportable::Configuration& )
{
}

void
PeptideWidget::OnInitialUpdate()
{
	table_->onInitialUpdate();
	peptideTable_->onInitialUpdate();
}

void
PeptideWidget::onUpdate( boost::any&& )
{
}

void
PeptideWidget::OnFinalClose()
{
}

bool
PeptideWidget::getContents( boost::any& a ) const
{
    if ( adportable::a_type< adcontrols::ProcessMethod >::is_pointer( a ) ) {

        adcontrols::ProcessMethod *pm = boost::any_cast< adcontrols::ProcessMethod * >( a );
        adcontrols::TargetingMethod method( adcontrols::TargetingMethod::idTargetPeptide );
        form_->getContents( method );

        pm->appendMethod< adcontrols::TargetingMethod >( method );

        return true;
    }

    return false;
}

bool
PeptideWidget::setContents( boost::any&& a )
{
	if ( adportable::a_type< adprot::digestedPeptides >::is_a( a ) ) {

        auto digested = boost::any_cast< adprot::digestedPeptides >( a );
		table_->setContents( digested );
        
        return true;

    } else if ( adportable::a_type< adcontrols::ProcessMethod >::is_a( a ) ) {

        const adcontrols::ProcessMethod& pm = boost::any_cast< adcontrols::ProcessMethod& >( a );
        auto it = std::find_if( pm.begin(), pm.end(), []( const adcontrols::ProcessMethod::value_type& t ){
                return t.type() == typeid(adcontrols::TargetingMethod)
                                          && boost::get< adcontrols::TargetingMethod >( t ).targetId() == adcontrols::TargetingMethod::idTargetPeptide;
            });
        if ( it != pm.end() ) {
            const adcontrols::TargetingMethod& method = boost::get< adcontrols::TargetingMethod >(*it);
            form_->setContents( method );
            table_->setContents( method );
        }
    }
    return false;
}

void
PeptideWidget::showContextMenu( const QPoint& pt )
{
    QMenu menu;

    QModelIndexList indices = table_->selectionModel()->selectedIndexes();
    qSort( indices );
    if ( indices.size() < 1 )
        return;

    std::set< int > rows;
    for ( auto& index: indices )
        rows.insert( index.row() );

    QStandardItemModel& model = table_->model();

    QVector< QPair< QString, QString > > peptides;
    for ( auto& row: rows ) 
		peptides.push_back( QPair< QString, QString >( model.index(row, 0).data().toString(), model.index( row, 1 ).data().toString() ) );

    QAction * action(0);
    if ( peptides.size() == 1 )
        action = menu.addAction( QString("Find %1").arg( peptides[0].first ) );
    else
        action = menu.addAction( "Find peptides" );

    QAction * selected = menu.exec( table_->mapToGlobal( pt ) );
    if ( selected == action )
        emit triggerFind( peptides );
}
