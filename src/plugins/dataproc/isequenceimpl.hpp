/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#ifndef ISEQUENCEIMPL_HPP
#define ISEQUENCEIMPL_HPP

#include <adextension/isequence.hpp>
#include <adextension/ieditorfactory.hpp>
#include <vector>
#include <memory>
#include <functional>
#include <tuple>

namespace dataproc {
    
    namespace detail {
        class iEditorFactoryImpl : public adextension::iEditorFactory {
            std::tuple< std::function< QWidget * (QWidget *) >, METHOD_TYPE, QString > d_;
        public:
            iEditorFactoryImpl( const iEditorFactoryImpl& t ) : d_( t.d_ ) {
            }
            iEditorFactoryImpl( std::function< QWidget *(QWidget * parent)> f, METHOD_TYPE mtype, const QString& title )
                : d_( std::make_tuple( f, mtype, title ) ) {
            }
            QWidget * createEditor( QWidget * parent ) override {
                return std::get<0>( d_ ) ? std::get<0>( d_ )(parent) : 0;
            }
            METHOD_TYPE method_type() const override { return std::get<1>(d_); }
            QString title() const override { return std::get<2>(d_); }
        };
    }

    class iSequenceImpl : public adextension::iSequence {
    public:
        iSequenceImpl();

        virtual size_t size() const;
        virtual reference operator [] ( size_t idx );

        iSequenceImpl& operator << ( detail::iEditorFactoryImpl );

    private:
        std::vector< detail::iEditorFactoryImpl > v_;
    };

}

#endif // ISEQUENCEIMPL_HPP
