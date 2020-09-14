/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC, Toin, Mie Japan
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


#include <QJsonArray>
#include <QJsonObject>

namespace adwidgets {

    class ScanLawDialogData {
    public:
        ScanLawDialogData( double l, double a, double t ) : L_( l ), acclVolts_( a ), t0_( t ) {
            json_ = QJsonObject{ { "length", l }, { "accelVolts", a }, { "tDelay", t }, { "peaks", QJsonArray{} } }
        }

        typedef std::tuple<
            int            // spectrumIndex
            , QString      // formula
            , double       // time
            , double       // mass
            , int          // mode
            > item_type;

        void operator()( item_type&& item ) {
            json_[ "peaks" ].push_back( QJsonObject{
                    { "index",     std::get<0>(item) }
                    , { "formula", std::get<1>(item) }
                    , { "time",    std::get<2>(item) }
                    , { "mass",    std::get<3>(item) }
                    , { "mode",    std::get<4>(item) }
                } );
        }
        double length() const     { return json_[ "length" ].toDouble(); }
        double accelVolts() const { return json_[ "accelVolts" ].toDouble(); }
        double tDelay() const     { return json_[ "tDelay" ].toDouble(); }
        std::vector< item_type > operator()() const {
            std::vector< item_type > vec;
            for ( auto& pk: json_[ "peaks" ] )
                vec.emplace_back( { pk[ "index" ].toInt()
                        , pk[ "formula" ].toString()
                        , pk[ "time" ].toDouble()
                        , pk[ "mass" ].toDouble()
                        , pk[ "mode" ].toInt() } );
            return vec;
        }
    private:
        QJsonObject json_;
    };
};

