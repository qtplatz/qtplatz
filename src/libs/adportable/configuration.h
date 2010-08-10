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

	namespace internal {

		class xml_element {
		public:
			xml_element();
			xml_element( const xml_element& );

			const std::wstring& xml() const { return xml_; }
			void xml( const std::wstring& );
			const std::wstring& text() const { return text_; }
			void text( const std::wstring& );

			const std::wstring& attribute( const std::wstring& key ) const;
			void attribute( const std::wstring& key, const std::wstring& value );
		protected:
			std::wstring xml_;
			std::wstring text_;
			std::map< std::wstring, std::wstring > attributes_;
		};
	}

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
		Configuration& append( const Configuration& );
		void xml( const std::wstring& );
        
		inline internal::xml_element& module() { return module_; }
        
		inline const std::wstring& xml() const { return xml_; }
        inline const attributes_type& attributes() const { return attributes_; }
        inline vector_type::iterator begin() { return children_.begin(); }
        inline vector_type::iterator end()   { return children_.end(); }
        inline vector_type::const_iterator begin() const { return children_.begin(); }
        inline vector_type::const_iterator end() const  { return children_.end(); }
    private:
		std::wstring xml_;
        std::wstring name_;
        std::wstring text_;
        attributes_type attributes_;
        std::vector< Configuration > children_;
		internal::xml_element module_;
    };

}
