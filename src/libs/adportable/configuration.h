// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <string>
#include <map>
#include <vector>

namespace adportable {

    class Configuration {
    public:
        ~Configuration(void);
        Configuration(void);
        Configuration( const Configuration& );

        typedef std::vector<Configuration> vector_type;
        typedef std::map<std::wstring, std::wstring> attributes_type;

        const std::wstring& component() const;
        const std::wstring& attribute( const std::wstring& key ) const;

        const std::wstring& name() const;
        void name( const std::wstring& );
        const std::wstring& text() const;
        void text( const std::wstring& );
        void attribute( const std::wstring& key, const std::wstring& value );
        bool readonly() const;
        bool hasChild() const;
        void append( const Configuration& );

        inline const attributes_type& attributes() const { return attributes_; }
        inline vector_type::iterator begin() { return children_.begin(); }
        inline vector_type::iterator end()   { return children_.end(); }
        inline vector_type::const_iterator begin() const { return children_.begin(); }
        inline vector_type::const_iterator end() const  { return children_.end(); }
    private:
        std::wstring name_;
        std::wstring text_;
        attributes_type attributes_;
        std::vector< Configuration > children_;
    };

}
