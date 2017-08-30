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

#include "deviceinfo.hpp"
#include <adportable/debug.hpp>
#if HAVE_CUDA
#include <cuda_runtime.h>
#include <helper_cuda.h>
#include <cuda.h>
#endif
#include <mutex>

using namespace advision;

deviceInfo::deviceInfo() : hasCUDA_( false )
{
#if HAVE_CUDA
    static std::once_flag flag;
    std::call_once( flag, [&](){
            int deviceCount = 0;
            cudaError_t errId = cudaGetDeviceCount( &deviceCount );
            if ( errId == cudaSuccess ) {
                hasCUDA_ = deviceCount;
            } else {
                ADDEBUG() << "Error: " << cudaGetErrorString( errId );
            }
        });
#endif
}

deviceInfo *    
deviceInfo::instance()
{
    static deviceInfo __deviceInfo;
    return &__deviceInfo;
}

bool
deviceInfo::hasCUDA() const
{
    return hasCUDA_;
}

