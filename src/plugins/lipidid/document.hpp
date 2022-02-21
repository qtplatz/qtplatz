/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include <QObject>
#include <memory>
#include <tuple>
#include <optional>

class QSettings;
class QSqlDatabase;

namespace adcontrols   { class MetIdMethod; class MassSpectrum; }
namespace adextension  { class iSessionManager; }
namespace portfolio    { class Folium; }

namespace lipidid {

    class simple_mass_spectrum;

    class document : public QObject    {
        Q_OBJECT
        explicit document( QObject *parent = 0 );
        document();
    public:
        ~document();

        static document * instance();
        static QSettings * settings();

        void initialSetup();
        void finalClose();

        QSqlDatabase sqlDatabase();
        bool load( const QString& file );
        bool find_all( adcontrols::MetIdMethod&& );

        std::shared_ptr< const adcontrols::MassSpectrum > reference_mass_spectrum() const;

        std::tuple< std::shared_ptr< const adcontrols::MassSpectrum > // acquired spectrum
                    , std::shared_ptr< const adcontrols::MassSpectrum > // reference (calculated) spectrum
                    , std::shared_ptr< const lipidid::simple_mass_spectrum > // reference (calculated) spectrum
                    > getResultSet() const;

        std::optional< std::string > find_svg( const std::string& InChIKey ) const;

    public slots:
        void handleAddProcessor( adextension::iSessionManager *, const QString& file );

        // change node (folium) selection
        void handleSelectionChanged( adextension::iSessionManager *, const QString& file, const portfolio::Folium& );

        // data contents changed
        void handleProcessed( adextension::iSessionManager *, const QString& file, const portfolio::Folium& );

        void handleCheckStateChanged( adextension::iSessionManager *, const QString& file, const portfolio::Folium&, bool );

    private:

    signals:
        void onConnectionChanged();
        void idCompleted();

        // souce iSessionManager
        void dataChanged( const portfolio::Folium& );
    private:
        class impl;
        std::unique_ptr< impl > impl_;
    };

}
