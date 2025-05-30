/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef SPECTROGRAMWND_HPP
#define SPECTROGRAMWND_HPP

#include <qwidget.h>
#include <memory>

namespace adplot { class SpectrogramWidget; class SpectrumWidget; class ChromatogramWidget; }
namespace portfolio { class Folium; }
namespace adcontrols { class ProcessMethod; class MassSpectrum;  class MassSpectra; }

namespace dataproc {

	class Dataprocessor;

    class ContourWnd : public QWidget {
        Q_OBJECT
    public:
        explicit ContourWnd(QWidget *parent = 0);

    private:
        bool mslock( std::shared_ptr< adcontrols::MassSpectrum >, const QVector< QPair<int, int> >& );
        bool mslock();

    signals:

    public slots:
        void handleSessionAdded( Dataprocessor* );
        void handleProcessed( Dataprocessor*, portfolio::Folium& );
        void handleSelectionChanged( Dataprocessor*, portfolio::Folium& );
        void handleApplyMethod( const adcontrols::ProcessMethod& );
        void handleCheckStateChanged( Dataprocessor*, portfolio::Folium&, bool );
        void handlePrintCurrentView( const QString& outpdf );

        void handleChromatogramYScale( bool checked, double bottom, double top ) const;
        void handleChromatogramXScale( bool checked, double left, double right ) const;

    private slots:
		void handleSelected( const QPointF& );
		void handleSelected( const QRectF& );

    private:
        std::shared_ptr< adplot::SpectrogramWidget > plot_;
        std::shared_ptr< adplot::SpectrumWidget > sp_;
        std::shared_ptr< adplot::ChromatogramWidget > chromatogr_;
        std::wstring foliumId_;
        std::string fullpath_;
        std::weak_ptr< adcontrols::MassSpectra > data_;
        void init();
    };

}

#endif // SPECTROGRAMWND_HPP
