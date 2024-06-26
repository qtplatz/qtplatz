/**************************************************************************
** Copyright (C) 2022-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2023 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <filesystem>

class QSettings;
class QSqlDatabase;

namespace adcontrols   { class IonReactionMethod; class MetIdMethod; class MassSpectrum; }
namespace adfs         { class sqlite; }
namespace adextension  { class iSessionManager; }
namespace portfolio    { class Folium; }

namespace lipidid {

    class simple_mass_spectrum;
    class mol;

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
        adfs::sqlite * sqlite();
        bool load( const QString& file );
        bool find_all( adcontrols::MetIdMethod&& );
        bool export_ion_reactions( adcontrols::IonReactionMethod&&, bool testing );
        std::shared_ptr< const adcontrols::MassSpectrum > reference_mass_spectrum() const;
        std::shared_ptr< const adcontrols::MassSpectrum > matched_mass_spectrum() const; // overlay on reference spectrum
        std::shared_ptr< const adcontrols::MassSpectrum > overlay_mass_spectrum() const; // computed isotope pattern

        std::tuple< std::shared_ptr< const adcontrols::MassSpectrum > // acquired spectrum
                    , std::shared_ptr< const adcontrols::MassSpectrum > // reference (calculated) spectrum
                    , std::shared_ptr< const lipidid::simple_mass_spectrum > // reference (calculated) spectrum
                    > getResultSet() const;

        std::optional< std::string > find_svg( const std::string& InChIKey ) const;
        std::shared_ptr< lipidid::mol > find_mol( const std::string& InChIKey ) const;
        std::filesystem::path dataFilename() const;
        void save_all() const;
        void load_all() const;

    public slots:
        void handleAddProcessor( adextension::iSessionManager *, const QString& file );

        // change node (folium) selection
        void handleSelectionChanged( adextension::iSessionManager *, const QString& file, const portfolio::Folium& );

        // data contents changed
        void handleProcessed( adextension::iSessionManager *, const QString& file, const portfolio::Folium& );

        void handleCheckStateChanged( adextension::iSessionManager *, const QString& file, const portfolio::Folium&, bool );
        void handleFormulaSelected( const QString& formula, double abundance, int index /* on simple_mass_spectrum */ );
        void handleCheckState( int index, double mass, bool checked );

    private:

    signals:
        void onConnectionChanged() const;
        void metIdMethodChanged( const adcontrols::MetIdMethod& ) const;
        void idCompleted() const;
        void onZoomed( int, const QRectF& ) const;
        void onFormulaSelected( const QString&, double abundance ) const;
        void onMatchedSelected( int ) const;

        // souce iSessionManager
        void dataChanged( const portfolio::Folium& );
    private:
        class impl;
        std::unique_ptr< impl > impl_;
    };

}
