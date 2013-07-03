/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#pragma once

#include <QWidget>
#include <memory>
#include <thread>
#include <array>
#include <mutex>

namespace TOFSignal { struct tofDATA; }
namespace TOF { struct ControlMethod; }

namespace adcontrols {
    class MassSpectrum;
    class Trace;
}

namespace adwplot {
	class ChromatogramWidget;
	class SpectrumWidget;
    class TraceWidget;
}

namespace toftune {

	class tofSignalMonitorView : public QWidget {
		Q_OBJECT
		explicit tofSignalMonitorView(QWidget *parent = 0);
	public:
		~tofSignalMonitorView();
        static tofSignalMonitorView * Create( QWidget * parent = 0 );

        void setData( const adcontrols::MassSpectrum& );
        void setData( const adcontrols::Trace&, const std::wstring& );
    
    signals:
        void analyzed( double h, double a );
    
	public slots:

	private:

        std::size_t nbrSamples_;
        std::size_t lower_;
        std::size_t upper_;
        double area_;
        double height_;

        adwplot::ChromatogramWidget * tplot_; // time trace
		adwplot::SpectrumWidget * splot_;     // full spectrum
        adwplot::SpectrumWidget * sp1_;       // zoom 1
        adwplot::SpectrumWidget * sp2_;       // zoom 2
        adwplot::SpectrumWidget * sp3_;       // zoom 3

        std::mutex mutex_;
	};

}
