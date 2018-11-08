/**************************************************************************
** Copyright (C) 2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2017 MS-Cheminformatics LLC, Toin, Mie Japan
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
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
**************************************************************************/

#include "dft2d.hpp"
#include <adportable/debug.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// http://docs.opencv.org/2.4/doc/tutorials/core/discrete_fourier_transform/discrete_fourier_transform.html

using namespace video;

cv::Mat
dft2d::dft( const cv::Mat& I )
{
    cv::Mat padded;                            //expand input image to optimal size
    int m = cv::getOptimalDFTSize( I.rows );
    int n = cv::getOptimalDFTSize( I.cols ); // on the border add zero values
    cv::copyMakeBorder(I, padded, 0, m - I.rows, 0, n - I.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

    cv::Mat planes[] = {cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F)};
    cv::Mat complexI;
    cv::merge(planes, 2, complexI);         // Add to the expanded another plane with

    cv::dft(complexI, complexI);            // this way the result may fit in the source matrix

    // compute the magnitude and switch to logarithmic scale
    // => log(1 + sqrt(Re(DFT(I))^2 + Im(DFT(I))^2))
    cv::split(complexI, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
    cv::magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
    cv::Mat magI = planes[0];

    magI += cv::Scalar::all(1);                    // switch to logarithmic scale
    log(magI, magI);

    // crop the spectrum, if it has an odd number of rows or columns
    magI = magI(cv::Rect(0, 0, magI.cols & -2, magI.rows & -2));

    // rearrange the quadrants of Fourier image  so that the origin is at the image center
    int cx = magI.cols/2;
    int cy = magI.rows/2;

    cv::Mat q0(magI, cv::Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
    cv::Mat q1(magI, cv::Rect(cx, 0, cx, cy));  // Top-Right
    cv::Mat q2(magI, cv::Rect(0, cy, cx, cy));  // Bottom-Left
    cv::Mat q3(magI, cv::Rect(cx, cy, cx, cy)); // Bottom-Right

    cv::Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);

    q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
    q2.copyTo(q1);
    tmp.copyTo(q2);

    cv::normalize(magI, magI, 0, 1, cv::NORM_MINMAX); // Transform the matrix with float values into a
                                                // viewable image form (float between values 0 and 1).

    //cv::imshow("Input Image"       , I   );    // Show the result
    //cv::imshow("spectrum magnitude", magI);
    return magI;
}

cv::Mat
dft2d::appod( const cv::Mat& I )
{
    cv::Mat padded;                            //expand input image to optimal size
    int m = cv::getOptimalDFTSize( I.rows );
    int n = cv::getOptimalDFTSize( I.cols ); // on the border add zero values
    cv::copyMakeBorder(I, padded, 0, m - I.rows, 0, n - I.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

    cv::Mat planes[] = {cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F)};
    cv::Mat complexI;
    cv::merge( planes, 2, complexI );         // Add to the expanded another plane with

    cv::dft( complexI, complexI );            // this way the result may fit in the source matrix

    for ( int i = 1; i < m - 1; ++i ) {
        complexI.at< cv::Complex< float > >( i, 1 ) = 0;
        complexI.at< cv::Complex< float > >( i, n - 2 ) = 0;
    }
    for ( int i = 1; i < n - 1; ++i ) {
        complexI.at< cv::Complex< float > >( 1, i ) = 0;
        complexI.at< cv::Complex< float > >( m - 2, i ) = 0;
    }

    cv::Mat invDFT;
    idft( complexI, invDFT, cv::DFT_SCALE | cv::DFT_REAL_OUTPUT ); // Applying IDFT

    return invDFT;
}

cv::Mat
dft2d::zerofill( const cv::Mat& I, const int N )
{
    cv::Mat padded;                            //expand input image to optimal size
    int m = cv::getOptimalDFTSize( I.rows );
    int n = cv::getOptimalDFTSize( I.cols ); // on the border add zero values
    cv::copyMakeBorder(I, padded, 0, m - I.rows, 0, n - I.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

    cv::Mat planes[] = {cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F)};
    cv::Mat complexI;
    cv::merge( planes, 2, complexI );         // Add to the expanded another plane with

    cv::dft( complexI, complexI );            // this way the result may fit in the source matrix

    cv::Mat expanded( m * N, n * N, complexI.type(), cv::Scalar(0) );

    for ( int i = 0; i < m; ++i ) {
        int x = i < m/2 ? i : i + (m * N - m);
        for ( int j = 0; j < m; ++j ) {
            int y = j < n/2 ? j : j + (n * N - n);
            expanded.at< cv::Complex<float > >( x, y ) = complexI.at< cv::Complex<float> >( i, j );
            //expanded.at< float >( x, y ) = I.at< float >( i, j );
        }
    }

    cv::Mat iDFT;
    cv::idft( expanded, iDFT, cv::DFT_SCALE | cv::DFT_REAL_OUTPUT ); // Applying IDFT

    return iDFT;
}
