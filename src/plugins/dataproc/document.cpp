/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <adcontrols/msqpeaks.hpp>

using namespace dataproc;

document * document::instance_ = 0;

document::document(QObject *parent) : QObject(parent)
                                    , quant_( std::make_shared< adcontrols::MSQPeaks >() )
{
}

document * 
document::instance()
{
    if ( instance_ == 0 )
        instance_ = new document;
    return instance_;
}

adcontrols::MSQPeaks *
document::msQuanTable()
{
    return quant_.get();
}

const adcontrols::MSQPeaks *
document::msQuanTable() const
{
    return quant_.get();
}

void
document::setMSQuanTable( const adcontrols::MSQPeaks& v )
{
    quant_ = std::make_shared< adcontrols::MSQPeaks >( v );
}
