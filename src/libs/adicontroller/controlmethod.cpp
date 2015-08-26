/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "controlmethod.hpp"

namespace adicontroller {
    
    namespace ControlMethod {

        MethodLine::MethodLine() : unitnumber_( 0 )
                                 , isInitialCondition_( true )
                                 , time_(0)
                                 , funcid_(0)
        {
        }

        MethodLine::MethodLine( const MethodLine& t ) : modelname_( t.modelname_ )
                                                      , description_( t.description_ )
                                                      , unitnumber_( t.unitnumber_ )
                                                      , isInitialCondition_( t.isInitialCondition_ )
                                                      , time_( t.time_ )
                                                      , funcid_( t.funcid_ )
                                                      , itemlabel_( t.itemlabel_ )
                                                      , xdata_( xdata_ )
        {
        }
        
        Method::Method()
        {
        }

        Method::Method( const Method& t ) : subject_( t.subject_ )
                                          , description_( t.description_ )
                                          , lines_( t.lines_ )
        {
        }
        
    } // namespace ControlMethod
} // namespace adicontroller
