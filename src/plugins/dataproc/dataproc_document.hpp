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

#ifndef DATAPROC_DOCUMENT_HPP
#define DATAPROC_DOCUMENT_HPP

#include <QObject>
#include <memory>
#include <set>

class QSettings;

namespace adcontrols {
    class MSQPeaks;
    class Chromatogram;
}

namespace dataproc {

    class Dataprocessor;

    class dataproc_document : public QObject
    {
        Q_OBJECT
        explicit dataproc_document(QObject *parent = 0);
        static dataproc_document * instance_;
    public:
        static dataproc_document * instance();
        void setMSQuanTable( const adcontrols::MSQPeaks& );
        adcontrols::MSQPeaks * msQuanTable();
        const adcontrols::MSQPeaks * msQuanTable() const;

        static size_t findCheckedTICs( Dataprocessor *, std::set< int >& vfcn );
        static const std::shared_ptr< adcontrols::Chromatogram > findTIC( Dataprocessor *, int );

    private:    
        std::shared_ptr< adcontrols::MSQPeaks > quant_;
        std::shared_ptr< QSettings > settings_;  // user scope settings

    signals:

    public slots:

    };

}

#endif // DATAPROC_DOCUMENT_HPP
