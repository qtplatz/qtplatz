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

#ifndef PEAKMETHODDELEGATE_HPP
#define PEAKMETHODDELEGATE_HPP

#include <QItemDelegate>
#if ! defined Q_MOC_RUN
#include <adcontrols/peakmethod.hpp>
#endif

namespace qtwidgets {

    enum { 
        c_header
        , c_value
        , c_num_columns
    };

    enum {
        r_slope
        , r_min_width
        , r_min_height
        , r_drift
        , r_min_area
        , r_doubling_time
        , r_void_time
        , r_pharmacopoeia
        , r_num_rows
    };

	class PeakMethodDelegate : public QItemDelegate {
		Q_OBJECT
	public:
		explicit PeakMethodDelegate(QObject *parent = 0);

        QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        void setEditorData(QWidget *editor, const QModelIndex &index) const override;
        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
        QSize sizeHint( const QStyleOptionViewItem&, const QModelIndex& ) const override;
    
    signals:
    
	public slots:

	public:
		// class PharmacopoeiaEnum {
		// public:
		// 	PharmacopoeiaEnum( adcontrols::chromatography::ePharmacopoeia t = adcontrols::chromatography::ePHARMACOPOEIA_JP );
		// 	adcontrols::chromatography::ePharmacopoeia methodValue() const;
        //     QString displayValue() const;
		// 	void setCurrentValue( adcontrols::chromatography::ePharmacopoeia );
        // private:
		// 	adcontrols::chromatography::ePharmacopoeia value_;
		// };
	};

}

// Q_DECLARE_METATYPE( qtwidgets::PeakMethodDelegate::PharmacopoeiaEnum )


#endif // PEAKMETHODDELEGATE_HPP
