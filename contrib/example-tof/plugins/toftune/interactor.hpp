/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#ifndef INTERACTOR_HPP
#define INTERACTOR_HPP

#if !defined Q_MOC_RUN
#include <boost/variant.hpp>
#endif
#include <memory>

namespace toftune {

    class DoubleSpinSlider;
    class SpinSlider;
    class PowerSpinSlider;

    template<class T> struct Interactor {
        std::shared_ptr< T > ptr_;
        Interactor( T * ptr ) : ptr_( ptr ) {
        }
    };

    typedef boost::variant< Interactor< DoubleSpinSlider >
                          , Interactor< SpinSlider >
                          , Interactor< PowerSpinSlider >  > Interactor_t;
}

#endif // INTERACTOR_HPP
