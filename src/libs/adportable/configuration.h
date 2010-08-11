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

    class Module {
    public:
        Module( const std::wstring& xml = L"" );
        Module( const Module& );
        inline const std::wstring& xml() const { return xml_; }
        inline const std::wstring& library_filename() const { return library_filename_; }
        void xml( const std::wstring& );
        void library_filename( const std::wstring& );

    private:
        std::wstring xml_;
        std::wstring library_filename_;
    };

    class Configuration {
    public:
        ~Configuration(void);
        Configuration(void);
        Configuration( const Configuration& );

        typedef std::vector<Configuration> vector_type;
        typedef std::map<std::wstring, std::wstring> attributes_type;

        const std::wstring& component() const;

		const std::wstring& interface() const;
		void interface( const std::wstring& );

        const std::wstring& attribute( const std::wstring& key ) const;

        const std::wstring& name() const;
        void name( const std::wstring& );

        const std::wstring& title() const;
		void title( const std::wstring& );

        void attribute( const std::wstring& key, const std::wstring& value );
        bool readonly() const;
        bool hasChild() const;
		Configuration& append( const Configuration& );
		void xml( const std::wstring& );
        void module( const Module& );

		bool isComponent() const;
		bool isPlugin() const;

		inline const std::wstring& xml() const { return xml_; }
        inline const attributes_type& attributes() const { return attributes_; }
        inline const Module& module() const { return module_; }
        inline vector_type::iterator begin() { return children_.begin(); }
        inline vector_type::iterator end()   { return children_.end(); }
        inline vector_type::const_iterator begin() const { return children_.begin(); }
        inline vector_type::const_iterator end() const  { return children_.end(); }

        static const Configuration * find( const Configuration&, const std::wstring& );

    private:
		std::wstring xml_;
        std::wstring name_;
        std::wstring text_;
		std::wstring interface_;  // <Component interface="value"/>
        attributes_type attributes_;
        std::vector< Configuration > children_;
        Module module_;
    };

}
