// -*- C++ -*-
/**************************************************************************
**
** MIT License
** Copyright (c) 2021-2022 Toshinobu Hondo, Ph.D

** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:

** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
**************************************************************************/

#include <iostream>
#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <string>

// reaction  ::=  reactants ">>" products
// reactants ::=  molecules
// products  ::=  molecules
// molecules ::=  molecule
//                molecules "." molecule
// molecule  ::=  a valid SMARTS string without "." characters
//                 "(" a valid SMARTS string without "." characters ")"

namespace reaction_grammer {

    namespace x3 = boost::spirit::x3;
    using x3::char_;
    x3::rule< struct reaction_
              , std::tuple< std::vector<std::string>, std::vector<std::string> > > reaction{"reaction"};
    x3::rule< struct molecules_, std::vector< std::string > > molecules{ "molecules" };

    auto add_reactants = [](auto& ctx){ std::get<0>(_val(ctx)) = _attr(ctx); };
    auto add_products = [](auto& ctx){ std::get<1>(_val(ctx)) = _attr(ctx); };
    auto add_molecule = [](auto& ctx){ _val(ctx).emplace_back( _attr(ctx) ); };

    auto const molecule = *~x3::char_(".>");

    auto const reactants = molecules;
    auto const products = molecules;

    auto const molecules_def =  molecule [add_molecule] >> *( '.' >> molecule [add_molecule] );

    auto const reaction_def =
        reactants [add_reactants]
        >> x3::char_('>')
        >> x3::char_('>')
        >> products [add_products];

    BOOST_SPIRIT_DEFINE(reaction, molecules)
}
