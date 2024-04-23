// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "glsl.h"
#include <qstring.h>
#include <qstringlist.h>

namespace GLSL {

class GLSL_EXPORT Token
{
public:
    int kind;
    int position;
    int length;
    int line; // ### remove

    union {
        int matchingBrace;
        int i; // integer value
        const QString *string; // string value
        void *ptr;
    };

    Token()
        : kind(0), position(0), length(0), line(0), ptr(nullptr) {}

    bool is(int k) const { return k == kind; }
    bool isNot(int k) const { return k != kind; }

    int begin() const { return position; }
    int end() const { return position + length; }
};

class GLSL_EXPORT Lexer
{
public:
    Lexer(Engine *engine, const char *source, unsigned size);
    ~Lexer();

    // Extra flag bits added to tokens by Lexer::classify() that
    // indicate which variant of GLSL the keyword belongs to.
    static const int Variant_GLSL_120            = 0x00010000;   // 1.20 and higher
    static const int Variant_GLSL_150            = 0x00020000;   // 1.50 and higher
    static const int Variant_GLSL_400            = 0x00040000;   // 4.00 and higher
    static const int Variant_GLSL_ES_100         = 0x00080000;   // ES 1.00 and higher
    static const int Variant_VertexShader        = 0x00200000;
    static const int Variant_FragmentShader      = 0x00400000;
    static const int Variant_Reserved            = 0x80000000;
    static const int Variant_Mask                = 0xFFFF0000;
    static const int Variant_All                 = 0xFFFF0000;

    union Value {
        int i;
        const QString *string;
        void *ptr;
    };

    Engine *engine() const { return _engine; }

    int state() const { return _state; }
    void setState(int state) { _state = state; }

    int variant() const { return _variant; }
    void setVariant(int flags) { _variant = flags; }

    bool scanKeywords() const { return _scanKeywords; }
    void setScanKeywords(bool scanKeywords) { _scanKeywords = scanKeywords; }

    bool scanComments() const { return _scanComments; }
    void setScanComments(bool scanComments) { _scanComments = scanComments; }

    int yylex(Token *tk);
    int findKeyword(const char *word, int length) const;

    void *yyval() const { return _yyval.ptr; }

    static QStringList keywords(int variant);

private:
    static int classify(const char *s, int len);

    void yyinp();
    int yylex_helper(const char **position, int *line);

    void warning(int line, const QString &message);
    void error(int line, const QString &message);

private:
    Engine *_engine;
    const char *_source;
    const char *_it;
    int _size;
    int _yychar;
    int _lineno;
    int _state;
    int _variant;
    unsigned _scanKeywords: 1;
    unsigned _scanComments: 1;
    Value _yyval;
};

} // namespace GLSL
