/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <adcontrols/datareader.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/mappedspectra.hpp>
#include <adcontrols/mappeddataframe.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/debug.hpp>
#include <adprocessor/dataprocessor.hpp>
#include <boost/any.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <QApplication>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/ml.hpp>


namespace adprocessor { class dataprocessor; }


namespace counting2d {
    // See malpix/malpix/mpxcontrols/constants.hpp    
    // malpix_observer = name_generator( "{6AE63365-1A4D-4504-B0CD-38AE86309F83}" )( "1.image.malpix.ms-cheminfo.com" )

    static const boost::uuids::uuid malpix_observer = boost::uuids::string_generator()( "{62ede8f7-dfa3-54c3-a034-e012173e2d10}" );
}

using namespace counting2d;

document *
document::instance()
{
    static document __instance;
    return &__instance;
}

document::document() : QObject( 0 )
                     , cursor_( mappedDataFrame_.end() )
                     , em_model_( cv::ml::EM::create() )
{
}

bool
document::setDataprocessor( std::shared_ptr< adprocessor::dataprocessor > dp )
{
    if ( auto rawfile = dp->rawdata() ) {
        
        if ( rawfile->dataformat_version() >= 3 ) {

            // is this contains MALPIX image data?
            adfs::stmt sql( *dp->db() );
            sql.prepare( "SELECT COUNT(*) FROM AcquiredConf WHERE objuuid = ?" );
            sql.bind( 1 ) = malpix_observer;
            while ( sql.step() == adfs::sqlite_row ) {
                // has MALPIX raw image (dataFrame) for each trigger
                if ( auto malpixReader = rawfile->dataReader( malpix_observer ) ) {
                    processor_ = dp;
                    return true;
                }
            }
        }
    }
    return false;
}

bool
document::fetch()
{
    if ( auto dp = processor_ ) {
        if ( auto reader = dp->rawdata()->dataReader( malpix_observer ) ) {
	  
            for ( auto it = reader->begin(); it != reader->end(); ++it ) {
                boost::any a = reader->getData( it->rowid() );
                if ( auto ptr = boost::any_cast< std::shared_ptr< adcontrols::MappedDataFrame > >( a ) ) {
                    if ( ! ptr->empty() ) {
                        mappedDataFrame_.emplace_back( ptr );
                        cursor_ = mappedDataFrame_.end() - 1;
                        makeGrayScale( *ptr );
                        emit dataChanged();
                        QApplication::processEvents();
                    }
                }
#if 0
                if ( auto map = reader->getMappedSpectra( it->rowid() ) ) {
                    if ( !map->empty() ) {
                        mappedSpectra_.emplace_back( map );
                        emit dataChanged();
                        QApplication::processEvents();
                    }
                }
#endif
            }
            rewind();
        }
        return true;
    }
    return false;
}

void
document::em( std::shared_ptr< const adcontrols::MappedDataFrame > dframe )
{
    const int N = 4;
    const int N1 = static_cast< int >( std::sqrt( double( N ) ) );
    const cv::Scalar colors[] =  {  cv::Scalar(0,0,255)
                                    , cv::Scalar(0,255,0)
                                    , cv::Scalar(0,255,255)
                                    , cv::Scalar(255,255,0)
    };

    int i, j;
    int nsamples = 100;
    cv::Mat samples( nsamples, 2, CV_32FC1 );
    cv::Mat labels;
    cv::Mat img = cv::Mat::zeros( cv::Size( dframe->size1(), dframe->size2() ), CV_8UC3 );

    samples = samples.reshape(2, 0);
    for( i = 0; i < N; i++ )  {
        // form the training samples
        cv::Mat samples_part = samples.rowRange( i * nsamples / N, (i+1) * nsamples / N );

        cv::Scalar mean( ( ( i % N1 ) + 1 ) * img.rows / ( N1 + 1 ),  ( ( i / N1 ) + 1 ) * img.rows / ( N1 + 1 ) );
        cv::Scalar sigma( 30, 30 );
        randn( samples_part, mean, sigma );
    }
    samples = samples.reshape(1, 0);

    // cluster the data
    // cv::Ptr<cv::ml::EM> em_model = cv::ml::EM::create();
    em_model_->setClustersNumber( N );
    em_model_->setCovarianceMatrixType( cv::ml::EM::COV_MAT_SPHERICAL );
    em_model_->setTermCriteria( cv::TermCriteria( cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 300, 0.1) );
    em_model_->trainEM( samples, cv::noArray(), labels, cv::noArray() );

    cv::Mat sample( 1, 2, CV_32FC1 );

    // classify every image pixel
    for( i = 0; i < img.rows; i++ ) {
        for( j = 0; j < img.cols; j++ )  {
            sample.at<float>(0) = (float)j;
            sample.at<float>(1) = (float)i;
            int response = cvRound(em_model_->predict2( sample, cv::noArray() )[1]);
            cv::Scalar c = colors[response];

            circle( img, cv::Point(j, i), 1, c*0.75, cv::FILLED );
        }
    }

    //draw the clustered samples
    for( i = 0; i < nsamples; i++ )  {
        cv::Point pt(cvRound(samples.at<float>(i, 0)), cvRound(samples.at<float>(i, 1)));
        circle( img, pt, 1, colors[labels.at<int>(i)], cv::FILLED );
    }

    imshow( "EM-clustering result", img );

    emit dataChanged();
}

void
document::kmean( std::shared_ptr< const adcontrols::MappedDataFrame > dframe )
{
    cv::Mat features( dframe->size1(), dframe->size2(), CV_32FC1 );
    
    for ( size_t i = 0; i < dframe->matrix().size1(); ++i ) {
       for ( size_t j = 0; j < dframe->matrix().size2(); ++j ) {
           features.at< float >( i, j ) = dframe->matrix()( i, j );
       }
    }
    
    emit dataChanged();    
}

void
document::contours( std::shared_ptr< const adcontrols::MappedDataFrame > dframe )
{
    emit dataChanged();    
}

// todo:
// http://stackoverflow.com/questions/30278473/how-to-efficiently-detect-the-center-of-clusters-in-an-image-with-opencv

// see 12, blur filter 
// http://stackoverflow.com/questions/356035/algorithm-for-detecting-clusters-of-dots

const cv::Mat&
document::mat() const
{
    return mat_;
}

void
document::makeGrayScale( const adcontrols::MappedDataFrame& df )
{
    mat_ = cv::Mat( df.size1(), df.size2(), CV_8UC1 );
    for ( size_t i = 0; i < df.size1(); ++i ) {
        for ( size_t j = 0; j < df.size2(); ++j ) {
            mat_.at< unsigned char >( i, j ) = df( i, j ) ? 255 : 0;
        }
    }
}
