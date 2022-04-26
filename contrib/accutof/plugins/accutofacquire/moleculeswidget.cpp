/**************************************************************************
 ** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
 ** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "moleculeswidget.hpp"
#include <adwidgets/create_widget.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/isocluster.hpp>
#include <adcontrols/isotopecluster.hpp>
#include <adcontrols/lapfinder.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/molecule.hpp>
#include <adcontrols/moltable.hpp>
#include <adcontrols/mssimulatormethod.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adcontrols/targeting.hpp>
#include <adportable/debug.hpp>
#include <adportable/is_type.hpp>
#include <adportfolio/folium.hpp>
#include <adutils/constants.hpp> // clsid for massspectrometer
#include <adwidgets/moltable.hpp>
#include <boost/json.hpp>
#include <QAbstractItemModel>
#include <QBoxLayout>
#include <QGroupBox>
#include <QItemSelectionModel>
#include <QMenu>
#include <QRadioButton>
#include <QSplitter>
#include <QStandardItemModel>

namespace {

    struct modelFinder {
        template< typename T = QTableView * >
        QAbstractItemModel * model( QWidget * widget ) const {
            if ( auto table = widget->findChild< T >() ) {
                return table->model();
            }
            return nullptr;
        }
    };

}

namespace {

    using adwidgets::create_widget;

    class MoleculesForm : public QWidget {
        Q_OBJECT

    public:
        explicit MoleculesForm( QWidget *parent = 0 ) : QWidget( parent ) {
            auto layout = new QVBoxLayout( this );
            if ( auto gb = create_widget<QGroupBox>( "polarity", tr("Polarity"), this ) ) {
                layout->addWidget( gb );

                auto pos = create_widget< QRadioButton >( "radio_pos", tr("&Positive") );
                auto neg = create_widget< QRadioButton >( "radio_neg", tr("&Negative") );
                pos->setChecked( true );
                auto layout2 = new QVBoxLayout( gb );
                layout2->addWidget( pos );
                layout2->addWidget( neg );
                connect( pos, &QRadioButton::toggled, this, [&]( bool checked ){
                    emit polarityToggled( checked ? adcontrols::polarity_positive : adcontrols::polarity_negative );
                    emit dataChanged();
                });
            }
            layout->addItem( new QSpacerItem( 40, 20, QSizePolicy::Maximum, QSizePolicy::Expanding ) );
        }
        ~MoleculesForm() {}

    private:
        MoleculesForm( const MoleculesForm& ) = delete;

    signals:
        void polarityToggled( adcontrols::ion_polarity );
        void dataChanged();
    public:

    };
}


namespace accutof {
    class MoleculesWidget::impl {
    public:
        std::weak_ptr< const adcontrols::MassSpectrometer > massSpectrometer_;
    };
}


using namespace accutof;

MoleculesWidget::MoleculesWidget(QWidget *parent) : QWidget(parent)
                                                  , impl_( new impl() )
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setMargin(0);
        layout->setSpacing(2);

        if ( QSplitter * splitter = new QSplitter ) {
            splitter->addWidget( ( new MoleculesForm ) );
            splitter->addWidget( ( new adwidgets::MolTable ) );
            splitter->setStretchFactor( 0, 0 );
            splitter->setStretchFactor( 1, 3 );
            splitter->setOrientation ( Qt::Horizontal );
            layout->addWidget( splitter );
        }
    }
    if ( auto form = findChild< MoleculesForm * >() ) {
        if ( auto table = findChild< adwidgets::MolTable * >() ) {
            connect( form, &MoleculesForm::polarityToggled, table, &adwidgets::MolTable::handlePolarity );
            connect( form, &MoleculesForm::dataChanged, [&](){
                emit valueChanged ( QString::fromStdString( readJson() ) );
            });
        }
    }
}

MoleculesWidget::~MoleculesWidget()
{
    delete impl_;
}

QWidget *
MoleculesWidget::create( QWidget * parent )
{
    return new MoleculesWidget( parent );
}

void
MoleculesWidget::OnCreate( const adportable::Configuration& )
{
}

void
MoleculesWidget::OnInitialUpdate()
{
    using adwidgets::MolTable;

    // if ( auto form = findChild< MSSimulatorForm * >() )
    //     form->OnInitialUpdate();
    if ( auto table = findChild< adwidgets::MolTable *>() ) {
        table->onInitialUpdate();

        std::vector< std::pair< MolTable::fields, bool > > hides = { {  { MolTable::c_abundance, true } } };
        table->setColumHide( hides );

        // connect( table, &adwidgets::MolTable::onContextMenu, this, &MoleculesWidget::handleContextMenu );
        if ( auto model = table->model() ) {
            connect( model, &QAbstractItemModel::dataChanged, this, &MoleculesWidget::handleDataChanged ); //
            connect( model, &QAbstractItemModel::rowsRemoved, this, &MoleculesWidget::handleRowsRemoved );
        }

        connect( table->selectionModel(), &QItemSelectionModel::currentChanged, this
                 , []( const QModelIndex& curr, const QModelIndex &prev ){
                     ADDEBUG() << "currentChanged " << std::make_pair( curr.row(), curr.column() );
                 });
    }
}

void
MoleculesWidget::onUpdate( boost::any&& )
{
}

void
MoleculesWidget::OnFinalClose()
{
}

bool
MoleculesWidget::getContents( boost::any& a ) const
{
    return false;
}

bool
MoleculesWidget::setContents( boost::any&& a )
{
    if ( auto table = findChild< adwidgets::MolTable * >() ) {

        if ( a.type() == typeid( std::string ) ) {
            // adcontrols::moltable mols;
            auto json = boost::any_cast< std::string >( a );
            if ( auto mols = json_to_moltable( json ) )
                table->setContents( *mols );
#if 0
            boost::system::error_code ec;
            auto jv = boost::json::parse( json, ec );
            if ( !ec )  {
                if ( jv.is_object() && jv.as_object().contains( "molecules" ) ) {

                    auto ja = jv.as_object()[ "molecules" ].as_array();

                    adcontrols::moltable::value_type mol;
                    for ( const auto& ji: ja ) {
                        for ( const auto& it: ji.as_object() ) {
                            if ( it.key() == "smiles" ) {
                                mol.smiles() = it.value().as_string().data();
                            }
                            if ( it.key() == "formula" ) {
                                mol.formula() = it.value().as_string().data();
                            }
                            if ( it.key() == "adducts" ) {
                                mol.adducts() = it.value().as_string().data();
                            }
                            if ( it.key() == "enable" ) {
                                mol.enable() = it.value().as_bool();
                            }
                            if ( it.key() == "synonym" ) {
                                mol.synonym() = it.value().as_string().data();
                            }
                            if ( it.key() == "mass" ) {
                                mol.mass() = it.value().as_double();
                            }
                        }
                        mols << mol;
                    }
                }
                ADDEBUG() << "----------- set " << mols.data().size() << " molecules";
                table->setContents( mols );
                return true;
            } else {
                ADDEBUG() << "error: json.parse() : " << ec.message() << "\n"
                          << json;
            }
#endif
        }
    }
    return false;
}

// This API added for mssimulator to determinse profile spectrum;  it used to use the interface for mspeakinfo, which take centroid
// but also nullptr in order to clear existing centroid
bool
MoleculesWidget::setContents( boost::any&& a, const std::string& dataSource )
{
    return false;
}

void
MoleculesWidget::handleDataChanged(const QModelIndex& topLeft, const QModelIndex& end, const QVector<int>& roles )
{
    ADDEBUG() << "handleDataChanged: "
              << std::make_pair( topLeft.row(), topLeft.column() ) << " -- "  << std::make_pair( end.row(), end.column() );

    if ( ( std::find( roles.begin(), roles.end(), Qt::CheckStateRole ) != roles.end() ) ||

         ( topLeft.column() != adwidgets::MolTable::c_mass ) ) {

        emit valueChanged ( QString::fromStdString( readJson() ) );
    }
}

void
MoleculesWidget::handleRowsRemoved(const QModelIndex&, int first, int last )
{
    ADDEBUG() << "handleRowsRemoved: " << std::make_pair( first, last );
    emit valueChanged ( QString::fromStdString( readJson() ) );
}

void
MoleculesWidget::handlePolarity( adcontrols::ion_polarity )
{
    // if ( impl_->current_polarity_ != polarity ) {
    //     impl_->current_polarity_ = polarity;

    //     auto model = impl_->model_;

    //     for ( int row = 0; row < model->rowCount(); ++row ) {
    //         auto adducts = model->index( row, c_adducts ).data( Qt::UserRole + 1 ).value< adducts_type >();
    //         model->setData( model->index( row, c_adducts ), adducts.get( polarity ) );
    //         impl_->formulaChanged( row );
    //     }
    // }
    // this->viewport()->repaint();
}

void
MoleculesWidget::setMassSpectrometer( std::shared_ptr< const adcontrols::MassSpectrometer > p )
{
    impl_->massSpectrometer_ = p;

    if ( p ) {
        if ( p->massSpectrometerClsid() == qtplatz::infitof::iids::uuid_massspectrometer ) {
            std::vector< std::pair< adwidgets::MolTable::fields, bool > > hides
                = { { adwidgets::MolTable::c_nlaps, false }, { adwidgets::MolTable::c_apparent_mass, false }, { adwidgets::MolTable::c_time, false } };
            if ( auto table = findChild< adwidgets::MolTable * >() )
                table->setColumHide( hides );
        } else {
            std::vector< std::pair< adwidgets::MolTable::fields, bool > > hides
                = { { adwidgets::MolTable::c_nlaps, true }, { adwidgets::MolTable::c_apparent_mass, true }, { adwidgets::MolTable::c_time, true } };
            if ( auto table = findChild< adwidgets::MolTable * >() )
                table->setColumHide( hides );
        }
    }
}

std::string
MoleculesWidget::readJson() const
{
    using adwidgets::MolTable;

    if ( auto table = findChild< adwidgets::MolTable *>() ) {

        if ( auto model = table->model() ) {
            boost::json::array ja;

            for ( size_t row = 0; row < model->rowCount(); ++row ) {
                auto formula = model->index( row, MolTable::c_formula ).data( Qt::EditRole ).toString();
                if ( ! formula.isEmpty() ) {
                    bool enable = model->index( row, MolTable::c_formula ).data( Qt::CheckStateRole ).toInt() == Qt::Checked;
                    double mass = model->index( row, MolTable::c_mass ).data( Qt::EditRole ).toDouble();
                    ja.emplace_back(
                        boost::json::object{
                            {   "formula", formula.toStdString() }
                            , { "enable", enable }
                            , { "mass", mass }
                            , { "synonym", model->index( row, MolTable::c_synonym ).data( Qt::EditRole ).toString().toStdString() }
                            , { "smiles",  model->index( row, MolTable::c_smiles ).data( Qt::EditRole ).toString().toStdString() }
                            , { "adducts", model->index( row, MolTable::c_adducts ).data( Qt::EditRole ).toString().toStdString() }
                        });
                }
            }

            // ADDEBUG() << "readJson -- rowCount: " << model->rowCount() << "--> size: " << ja.size();
            auto json = boost::json::serialize( boost::json::value{{ "molecules", ja }} );
            return json;
        }
    }
    return {};
}

boost::optional< adcontrols::moltable >
MoleculesWidget::json_to_moltable( const std::string& json )
{
    adcontrols::moltable mols;

    boost::system::error_code ec;
    auto jv = boost::json::parse( json, ec );
    if ( !ec )  {
        if ( jv.is_object() && jv.as_object().contains( "molecules" ) ) {

            auto ja = jv.as_object()[ "molecules" ].as_array();

            adcontrols::moltable::value_type mol;
            for ( const auto& ji: ja ) {
                for ( const auto& it: ji.as_object() ) {
                    if ( it.key() == "smiles" ) {
                        mol.smiles() = it.value().as_string().data();
                    }
                    if ( it.key() == "formula" ) {
                        mol.formula() = it.value().as_string().data();
                    }
                    if ( it.key() == "adducts" ) {
                        mol.adducts() = it.value().as_string().data();
                    }
                    if ( it.key() == "enable" ) {
                        mol.enable() = it.value().as_bool();
                    }
                    if ( it.key() == "synonym" ) {
                        mol.synonym() = it.value().as_string().data();
                    }
                    if ( it.key() == "mass" ) {
                        mol.mass() = it.value().as_double();
                    }
                }
                mols << mol;
            }
        }
        return mols;
    }
    return {};
}

#include "moleculeswidget.moc"
