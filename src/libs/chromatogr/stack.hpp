/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
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

#pragma once


#include <deque>
#include <exception>

namespace chromatogr {

    class stack_underflow : public std::exception {
    public:
        virtual const char* what() const throw() {
            return "read empty stack";
        }
    };
    class subscript_out_of_range : public std::exception {
    public:
        virtual const char* what() const throw() {
            return "subscript out of range";
        }
    };
    
    template <class T, class Container = std::deque<T> >
    class stack  {
        typedef typename Container::value_type value_type;
        typedef typename Container::size_type  size_type;
        typedef          Container             container_type;
    public:
        explicit stack(const Container& = Container()) {  };
        virtual ~stack() { };
    protected:
        Container c;
    public:
        inline bool empty() const             { return c.empty(); };
        inline void clear()                   { c.clear();        };
        inline size_type size() const         { return c.size();  };
        inline void push(const value_type& x) { c.push_front(x);  };
        inline void pop()                     { 
            if (c.empty())
                throw stack_underflow();
            c.pop_front(); 
        }
        inline value_type& top()              { 
            if (c.empty())
                throw stack_underflow();
            return c.front(); 
        }
        inline const value_type& top() const  { 
            if (c.empty())
                throw stack_underflow();
            return c.front();
        }
        inline const value_type& operator[](int idx) const {
            if ( (idx < 0) || (c.size() <= size_t(idx)) )
                throw subscript_out_of_range();
            return c[idx];
        }
    };
}
