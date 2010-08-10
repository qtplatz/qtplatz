// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <string>

namespace adportable {

    class Component {
    public:
        ~Component(void);
        Component(void);
        Component( const Component& );

        class Interface {
        public:
            ~Interface();
            Interface( const std::wstring& name = L""
                , const std::wstring& type = L""
                , const std::wstring& interface = L"" );
            Interface( const Interface& );
            const std::wstring type() const;  // "widget" | "object"
            void type( const std::wstring& );
            const std::wstring name() const;  // "adtofms::imonitor::ui"
            void name( const std::wstring& );
            const std::wstring interface() const;
            void interface( const std::wstring& );

        private:
            std::wstring name_;
            std::wstring type_;
            std::wstring interface_;
			std::wstring loader_;
			std::wstring module_;
        };

        const std::wstring& name() const;
        void name( const std::wstring& );
        const std::wstring& type() const;
        void type( const std::wstring& );
    private:
        std::wstring name_;
        std::wstring type_;
    };

}
