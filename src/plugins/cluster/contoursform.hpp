/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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
#ifndef CONTOURSFORM_HPP
#define CONTOURSFORM_HPP

#include <QWidget>

namespace Ui {
class ContoursForm;
}

class ContoursForm : public QWidget
{
    Q_OBJECT

public:
    explicit ContoursForm(QWidget *parent = 0);
    ~ContoursForm();

    void setResize( int );
    void setBlurSize( int );
    void setCannyThreshold( int );
    void setMinSizeThreshold( unsigned );
    void setMaxSizeThreshold( unsigned );
    
    int resize() const;
    int blurSize() const;
    int cannyThreshold() const;
    unsigned minSizeThreshold() const;
    unsigned maxSizeThreshold() const;

    enum valueId { idResize, idBlurSize, idCannyThreshold, idMinSizeThreshold, idMaxSizeThreshold };

signals:
    void valueChanged( valueId, int value );

private:
    Ui::ContoursForm *ui;
};

#endif // CONTOURSFORM_HPP
