/**************************************************************************
** Copyright (C) 2010-2025 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2025 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "peakdwidget.hpp"
#include "tableview.hpp"
#include "create_widget.hpp"
#include <QtCore/qnamespace.h>
#include <adcontrols/peakd/ra.hpp>
#include <adportable/debug.hpp>
#include <adportable/json_helper.hpp>
#include <QBoxLayout>
#include <QHeaderView>
#include <QSplitter>
#include <QStandardItemModel>
#include <QtCore/qtmetamacros.h>
#include <boost/json.hpp>
#include <numeric>

class QStandardItemModel;
class QStandardItem;
class QModelIndex;

namespace adwidgets {

    namespace peakd {

        class PeakdForm : public QWidget {
            Q_OBJECT
        public:
            PeakdForm( QWidget * parent = nullptr ) : QWidget( parent ) {
                if ( auto layout = new QVBoxLayout( this ) ) {
                    layout->setContentsMargins( {} );
                }
            }
            ~PeakdForm() {}
            QBoxLayout * topLayout_;
        };

    } // namespace peakd

    class PeakdWidget::impl {
    public:
        adwidgets::TableView * tv_;
        QStandardItemModel * model_;
        std::map< std::string, std::vector< adcontrols::peakd::RA > > raList_;
        std::map< std::string, std::pair< double, std::vector< adcontrols::peakd::RA >::const_iterator > > map_;
        impl() : tv_( nullptr ) {}
    };

}

using namespace adwidgets;

PeakdWidget::PeakdWidget( QWidget * parent ) : QWidget( parent )
                                             , impl_( std::make_unique< impl >() )
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setContentsMargins( {} );
        layout->setSpacing(2);

        if ( QSplitter * splitter = new QSplitter ) {
            if ( auto form = add_widget( splitter, create_widget< peakd::PeakdForm >( "peakdForm" ) ) ) {
            }
            if ( auto tv = add_widget( splitter, create_widget< adwidgets::TableView >( "Abudances" ) ) ) {
                impl_->model_ = new QStandardItemModel();
                impl_->model_->setRowCount( 1 );
                impl_->model_->setColumnCount( 1 );
                tv->setModel( impl_->model_ );
                impl_->tv_ = tv;
            }
            splitter->setStretchFactor( 0, 0 );
            splitter->setStretchFactor( 1, 2 );
            splitter->setOrientation ( Qt::Horizontal );
            layout->addWidget( splitter );
        }
    }
    // if ( auto form = findChild< adwidgets::TargetingForm * >() ) {
    //     connect( form, &adwidgets::TargetingForm::triggerProcess, [&] { emit triggered(); } );
    // }
}

PeakdWidget::~PeakdWidget()
{
}

void
PeakdWidget::OnCreate( const adportable::Configuration& )
{
}

void
PeakdWidget::OnInitialUpdate()
{
}

void
PeakdWidget::OnFinalClose()
{
}

bool
PeakdWidget::getContents( boost::any& ) const
{
    return {};
}

bool
PeakdWidget::setContents( boost::any&& )
{
    return {};
}

void
PeakdWidget::handleOnNotifyRelativeAbundance( const QByteArray& json )
{
    auto ra = boost::json::value_to< adcontrols::peakd::RA >( adportable::json_helper::parse( json.toStdString() ) );

    impl_->raList_[ ra.ident() ].emplace_back( ra );

    auto it = std::max_element( impl_->raList_.begin(), impl_->raList_.end(), [](const auto& a, const auto& b){ return a.second.size() < b.second.size(); });
    auto nCols = it->second.size();

    auto nRows = std::accumulate( impl_->raList_.begin(), impl_->raList_.end(), 0
                                  , []( size_t a, const auto& b ){ return a + b.second.at(0).values().size(); });

    impl_->model_->setColumnCount( nCols + 1 );
    impl_->model_->setRowCount( nRows );

    // ADDEBUG() << boost::json::value_from( ra );
    // ADDEBUG() << "ra2 -- dataSource: " << ra.dataSource();
    auto model = impl_->tv_->model();
    int row_i{ 0 };
    for ( const auto& list: impl_->raList_ ) {
        int col{1};
        int row;
        for ( const auto& ra: list.second ) {
            row = row_i;
            for ( const auto& value: ra.values() ) {
                auto [relA,srcA,name,id] = value;
                model->setHeaderData( row, Qt::Vertical, QString::fromStdString( name ) );
                model->setData( model->index(row, 0), QString::fromStdString( name ) );
                model->setData( model->index(row, col), relA );
                ADDEBUG() << "row,col=" << std::make_pair( row, col ) << ", value: " << value;
                ++row;
            }
            col++;
        }
        row_i = row;
    }
    impl_->tv_->horizontalHeader()->resizeSections( QHeaderView::ResizeToContents );

}

#include "peakdwidget.moc"
