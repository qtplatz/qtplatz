/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#ifndef ELEMENT_HPP
#define ELEMENT_HPP

namespace detail { struct element; }

class element {
    const detail::element * element_;
public:
    element( const detail::element * e = 0 );

    struct isotope { double mass; double abundance; };

    operator bool () const { return element_ != 0; }

    const char * symbol() const;
    const char * name() const;
    int atomicNumber() const;
    int valence() const;
    int isotopeCount() const;
    const isotope * isotopes() const;
};


#endif // ELEMENT_HPP
