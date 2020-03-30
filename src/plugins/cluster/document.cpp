/**************************************************************************
** Copyright (C) 2013-2020 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "document.hpp"
#include "constants.hpp"
#include "player.hpp"
#include "uimediator.hpp"
// #include <mpxprocessor/processor.hpp>
// #include <mpxcontrols/constants.hpp>
// #include <mpxcontrols/histogrammethod.hpp>
// #include <mpxinterpreter/datareader.hpp>
#include <adcontrols/contoursmethod.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/mappedimage.hpp>
#include <adextension/isessionmanager.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/sqlite.hpp>
#include <adlog/logger.hpp>
#include <adportfolio/folium.hpp>
#include <adportable/profile.hpp>
#include <adportable/debug.hpp>
#include <adprocessor/dataprocessor.hpp>
#include <qtwrapper/settings.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <qtwrapper/progresshandler.hpp>
#include <coreplugin/progressmanager/progressmanager.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>
#include <boost/lexical_cast.hpp>
#include <app/app_version.h>
#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QString>
#include <QFuture>
#include <QMetaType>
#include <algorithm>

Q_DECLARE_METATYPE( portfolio::Folium )

using namespace cluster;

namespace cluster {

    struct user_preference {
        static boost::filesystem::path path( QSettings * settings ) {
            boost::filesystem::path dir( settings->fileName().toStdWString() );
            return dir.remove_filename() / "cluster";
        }
    };

    struct processor {
        std::weak_ptr< adprocessor::dataprocessor > dp_;
    };

    class document::impl {
    public:
        ~impl() {}
        impl() : settings_( std::make_shared< QSettings >( QSettings::IniFormat
                                                           , QSettings::UserScope
                                                           , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR )
                                                           , QLatin1String( "cluster.proc" ) ) )
                 //, dpIt_( dataprocessors_.end() )
               , axis_{ { document::spectrum, document::axis_time }, { document::timetrace, document::axis_time } }
                 //, histMethod_( std::make_unique< mpxcontrols::HistogramMethod >() )
               , zAutoScaleEnabled_{ true, true }
               , zAutoScale_{ 0, 0 }
               , zScale_{ 0, 0 }
               , player_( std::make_unique< Player >() )
               , momentsEnabled_( false )
            {}

        bool is_malpix_data( adfs::sqlite& db );

        std::shared_ptr< QSettings > settings_;

        std::map< QString, processor > dataprocessors_;
        QString currFile_;
        std::map< document::plotType, document::axisType > axis_;

        std::shared_ptr< const adcontrols::MappedSpectra > mappedSpectra_;
        std::shared_ptr< const adcontrols::MappedImage > mappedImage_;

        // QRect imageRect_;
        // std::unique_ptr< mpxcontrols::HistogramMethod > histMethod_;

        std::array< bool, 2 > zAutoScaleEnabled_;
        std::array< uint32_t, 2 > zAutoScale_;
        std::array< uint32_t, 2 > zScale_;

        std::unique_ptr< Player > player_;
        adcontrols::ContoursMethod contoursMethod_;
        bool momentsEnabled_;

    private:

    };
}

using namespace cluster;

std::mutex document::mutex_;

document::~document()
{
    delete impl_;
}

document::document() : impl_( new impl() )
{
    // impl_->histMethod_->setCellSelectionEnable( false );
    // impl_->histMethod_->setTimeWindowEnable( false );
}

document *
document::instance()
{
	static document __instance;
	return &__instance;
}

void
document::initialSetup()
{
    impl_->contoursMethod_.setSizeFactor( settings().value( "ContoursMethod/sizeFactor", 1 ).toInt() );
    impl_->contoursMethod_.setBlurSize( settings().value( "ContoursMethod/blurSize", 0 ).toInt() );
    impl_->contoursMethod_.setCannyThreshold( settings().value( "ContoursMethod/cannyThreshold", 0 ).toInt() );
    impl_->contoursMethod_.setMinSizeThreshold( settings().value( "ContoursMethod/minSizeThreshold", 0 ).toInt() );
    impl_->contoursMethod_.setMaxSizeThreshold( settings().value( "ContoursMethod/maxSizeThreshold", std::numeric_limits<int>::max() ).toInt() );
}

void
document::finalClose()
{
}

QSettings&
document::settings()
{
    return *impl_->settings_;
}

void
document::setCurrentFile( const QString& filename )
{
    if ( ! impl_->dataprocessors_.empty() ) {

        auto it = impl_->dataprocessors_.find( filename );
        if ( it != impl_->dataprocessors_.end() ) {
            if ( auto dp = it->second.dp_.lock() )
                impl_->currFile_ = filename;
            // auto p = it->second;
            // double trigInterval = p->duration() / p->trigCounts();
            // player()->setTrigInterval( trigInterval );
            emit currentProcessorChanged();
        }
    }
}

std::shared_ptr< adprocessor::dataprocessor >
document::currentProcessor()
{
    auto it = impl_->dataprocessors_.find( impl_->currFile_ );
    if ( it != impl_->dataprocessors_.end() )
        return it->second.dp_.lock();
    return {};
}

void
document::attach_file( const QString& dbfile, const QString& attach_file )
{
}

QString
document::lastDataDir() const
{
    return qtwrapper::settings( *impl_->settings_ ).recentFile( Constants::GRP_DATA_FILES, Constants::KEY_FILES );
}

bool
document::impl::is_malpix_data( adfs::sqlite& db )
{
    adfs::stmt sql( db );

    // is v3.6.0 data?
    sql.prepare( "SELECT COUNT(*) FROM AcquiredConf WHERE trace_id = '1.image.malpix.ms-cheminfo.com'" );
    while ( sql.step() == adfs::sqlite_row ) {
        auto count = sql.get_column_value< int64_t >( 0 );
        if ( count )
            return true;
    }

    // is old implementation
    sql.prepare( "SELECT COUNT(*) FROM AcquiredConf WHERE trace_id IN ( 'MALPIX4.TDC', 'MALPIX4.TIC' )" );
    while ( sql.step() == adfs::sqlite_row ) {
        auto count = sql.get_column_value< int64_t >( 0 );
        if ( count )
            return true;
    }

    return false;
}

document::axisType
document::axis( plotType plot ) const
{
	return impl_->axis_[plot];
}

void
document::setAxis( plotType plot, axisType axis )
{
    impl_->axis_[ plot ] = axis;
}

void
document::handleProcessed( adextension::iSessionManager * mgr, const QString& file, const portfolio::Folium& folium )
{
    std::shared_ptr< adprocessor::dataprocessor > dp;
    if ( mgr && ( dp = mgr->getDataprocessor( file ) ) ) {
        ADDEBUG() << __FUNCTION__ << ":\t" << dp->filename() << ", " << folium.name();
    }
}

void
document::handleCheckStateChanged( adextension::iSessionManager * mgr, const QString& file, const portfolio::Folium& folium, bool isChecked )
{
    ADDEBUG() << __FUNCTION__ << "\t##### " << file.toStdString() << ", folium: " << folium.fullpath() << ", isChecked: " << isChecked;
    std::shared_ptr< adprocessor::dataprocessor > dp;
    if ( mgr && ( dp = mgr->getDataprocessor( file ) ) ) {
        emit checkStateChanged( folium );
    }
}

void
document::handleSelectionChanged( adextension::iSessionManager * mgr, const QString& file, const portfolio::Folium& folium )
{
    ADDEBUG() << __FUNCTION__ << "\t##### " << file.toStdString() << ", folium: " << folium.fullpath();
    setCurrentFile( file );
    emit dataChanged( folium );
}

void
document::handleAddProcessor( adextension::iSessionManager * mgr, const QString& file )
{
    ADDEBUG() << __FUNCTION__ << "\t" << file.toStdString();

    // garbage correction
    if ( ! impl_->dataprocessors_.empty() ) {
        for ( auto it = impl_->dataprocessors_.begin(); it != impl_->dataprocessors_.end(); ) {
            if ( ! it->second.dp_.lock() )
                it = impl_->dataprocessors_.erase( it );
            else
                ++it;
        }
    }
    
    if ( mgr ) {
        if ( auto dp = mgr->getDataprocessor( file ) ) {
            impl_->dataprocessors_[ file ].dp_ = dp;
            setCurrentFile( file );
        }
    }
}

void
document::setCellSelection( const QRect& rect )
{
    // auto& m = *impl_->histMethod_;
    // m.setX( rect.x() );
    // m.setY( rect.y() );
    // m.setWidth( rect.width() );
    // m.setHeight( rect.height() );
}

void
document::setCellSelectionEnabled( bool enable )
{
    // auto& m = *impl_->histMethod_;
    // m.setCellSelectionEnable( enable );
}

void
document::setHistogramWindowEnabled( bool enable )
{
    // auto& m = *impl_->histMethod_;
    // m.setTimeWindowEnable( enable );
    // ADDEBUG() << __FUNCTION__ << " (enable=" << enable << ")";
    emit tofWindowEnabled( enable );
}

bool
document::histogramWindowEnabled() const
{
    // ADDEBUG() << __FUNCTION__ << " (enable=" << impl_->histMethod_->isTimeWindowEnable() << ")";
//    return impl_->histMethod_->isTimeWindowEnable();
    return false;
}

void
document::setHistogramWindow( double tof, double width )
{
    // auto& m = *impl_->histMethod_;
    // m.setTimeDelay( tof );          // hitogram start (left edge)
    // m.setTimeWindow( width );       // histogram window (duration)

    emit tofWindowChanged( tof, width );
}

std::pair< double, double >
document::histogramWindow() const
{
    //return std::make_pair( impl_->histMethod_->timeDelay(), impl_->histMethod_->timeWindow() );
    return {};
}

// const mpxcontrols::HistogramMethod&
// document::histogramMethod() const
// {
//     //return *impl_->histMethod_;
// }

void
document::setMappedImage( std::shared_ptr< const adcontrols::MappedImage > image
                          , std::shared_ptr< const adcontrols::MappedSpectra > spectra
                          , const std::pair< double, double >& trig
                          , const std::pair< double, double >& tof )
{
    if ( spectra )
        impl_->mappedSpectra_ = spectra;

    if ( ( impl_->mappedImage_ = image ) )
        impl_->zAutoScale_[ zMapRaw ] = uint32_t( image->mergeCount() );

    emit mappedImageChanged();
}

std::shared_ptr< const adcontrols::MappedImage >
document::mappedImage() const
{
    return impl_->mappedImage_;
}

std::shared_ptr< const adcontrols::MappedSpectra >
document::mappedSpectra() const
{
    return impl_->mappedSpectra_;
}

Player *
document::player()
{
    return impl_->player_.get();
}

void
document::setZAutoScaleEnable( ZMapId id, bool enable )
{
    impl_->zAutoScaleEnabled_[ id ] = enable;
}

bool
document::zAutoScaleEnable( ZMapId id ) const
{
    return impl_->zAutoScaleEnabled_[ id ];
}

void
document::setZScale( ZMapId id, uint32_t value )
{
    impl_->zScale_[ id ] = value;
}

uint32_t
document::zScale( ZMapId id ) const
{
    return impl_->zScale_[ id ];
}

uint32_t
document::zAutoScale( ZMapId id ) const
{
    return impl_->zAutoScale_[ id ];
}

adcontrols::ContoursMethod&
document::contoursMethod()
{
    return impl_->contoursMethod_;
}

const adcontrols::ContoursMethod&
document::contoursMethod() const
{
    return impl_->contoursMethod_;
}

void
document::updateContoursMethod()
{
    settings().setValue( "ContoursMethod/sizeFactor", impl_->contoursMethod_.sizeFactor() );
    settings().setValue( "ContoursMethod/blurSize", impl_->contoursMethod_.blurSize() );
    settings().setValue( "ContoursMethod/cannyThreshold", impl_->contoursMethod_.cannyThreshold() );
    settings().setValue( "ContoursMethod/minSizeThreshold", impl_->contoursMethod_.minSizeThreshold() );
    settings().setValue( "ContoursMethod/maxSizeThreshold", impl_->contoursMethod_.maxSizeThreshold() );

    emit contoursMethodChanged();
}

void
document::setMomentsEnabled( bool enable )
{
    impl_->momentsEnabled_ = enable;
}

bool
document::momentsEnabled() const
{
    return impl_->momentsEnabled_;
}
