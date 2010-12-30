// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#ifndef ANNOTATIONS_H
#define ANNOTATIONS_H

namespace SAGRAPHICSLib {
    struct ISADPAnnotations;
}

namespace adwidgets {
    namespace ui {

        class Annotation;

        class Annotations {
        public:
            ~Annotations();
            Annotations( SAGRAPHICSLib::ISADPAnnotations * pi = 0 );
            Annotations( const Annotations& );

            void visible( bool );
            bool visible() const;
            bool annotateX() const;
            void annotateX( bool );
            bool annotateY() const;
            void annotateY( bool );
            int decimalsX() const;
            void decimalsX( int );
            int decimalsY() const;
            void decimalsY( int );
            double textAngle() const;
            void textAngle( double );
            bool centreHorizontal() const;
            bool centreVerticl() const;
            void centreHorizontal( bool );
            void centreVertical( bool );
            Annotation add();
            void remove( int );
            Annotation operator [] ( int idx );
        private:
            SAGRAPHICSLib::ISADPAnnotations * pi_;
        };

    }
}

#endif // ANNOTATIONS_H
