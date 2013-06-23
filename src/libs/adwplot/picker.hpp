/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#ifndef PLOTPICKER_HPP
#define PLOTPICKER_HPP

#include <qwt_plot_picker.h>

class QwtPlotCanvas;

namespace adwplot {

	class Picker : public QwtPlotPicker {
		Q_OBJECT
	public:
#if QWT_VERSION >= 0x060100
		explicit Picker( QWidget * parent = 0 );
#else // 6.0.3 or earlier
		explicit Picker( QwtPlotCanvas * parent = 0 );
#endif
		// QwtPlotPicker
		void widgetMousePressEvent( QMouseEvent * ) override;
		void widgetMouseReleaseEvent( QMouseEvent * ) override;
		void widgetMouseDoubleClickEvent( QMouseEvent * ) override;
		void widgetMouseMoveEvent( QMouseEvent * ) override;
        bool end( bool ok ) override;

    signals:
    
    public slots:
    
    };

}

#endif // PLOTPICKER_HPP
