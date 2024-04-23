// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "languageutils_global.h"
#include "componentversion.h"

#include <QString>
#include <QStringList>
#include <QList>
#include <QHash>
#include <QSharedPointer>

QT_BEGIN_NAMESPACE
class QCryptographicHash;
QT_END_NAMESPACE

namespace LanguageUtils {

class LANGUAGEUTILS_EXPORT FakeMetaEnum {
    QString m_name;
    QStringList m_keys;

public:
    FakeMetaEnum();
    explicit FakeMetaEnum(const QString &name);

    bool isValid() const;

    QString name() const;
    void setName(const QString &name);

    void addKey(const QString &key);
    QString key(int index) const;
    int keyCount() const;
    QStringList keys() const;
    bool hasKey(const QString &key) const;
    void addToHash(QCryptographicHash &hash) const;

    QString describe(int baseIndent = 0) const;
    QString toString() const;
};

class LANGUAGEUTILS_EXPORT FakeMetaMethod {
public:
    enum {
        Signal,
        Slot,
        Method
    };

    enum {
        Private,
        Protected,
        Public
    };

public:
    FakeMetaMethod();
    explicit FakeMetaMethod(const QString &name, const QString &returnType = QString());

    QString methodName() const;
    void setMethodName(const QString &name);

    void setReturnType(const QString &type);

    QStringList parameterNames() const;
    QStringList parameterTypes() const;
    void addParameter(const QString &name, const QString &type);

    int methodType() const;
    void setMethodType(int methodType);

    int access() const;

    int revision() const;
    void setRevision(int r);
    void addToHash(QCryptographicHash &hash) const;

    QString describe(int baseIndent = 0) const;
    QString toString() const;
private:
    QString m_name;
    QString m_returnType;
    QStringList m_paramNames;
    QStringList m_paramTypes;
    int m_methodTy;
    int m_methodAccess;
    int m_revision;
};

class LANGUAGEUTILS_EXPORT FakeMetaProperty {
    QString m_propertyName;
    QString m_type;
    bool m_isList;
    bool m_isWritable;
    bool m_isPointer;
    int m_revision;

public:
    FakeMetaProperty(const QString &name, const QString &type, bool isList, bool isWritable, bool isPointer, int revision);

    QString name() const;
    QString typeName() const;

    bool isList() const;
    bool isWritable() const;
    bool isPointer() const;
    int revision() const;
    void addToHash(QCryptographicHash &hash) const;

    QString describe(int baseIndent = 0) const;
    QString toString() const;
};

class LANGUAGEUTILS_EXPORT FakeMetaObject {
    Q_DISABLE_COPY(FakeMetaObject);

public:
    typedef QSharedPointer<FakeMetaObject> Ptr;
    typedef QSharedPointer<const FakeMetaObject> ConstPtr;

    class LANGUAGEUTILS_EXPORT Export {
    public:
        Export();

        QString package;
        QString type;
        ComponentVersion version;
        int metaObjectRevision;

        bool isValid() const;
        void addToHash(QCryptographicHash &hash) const;

        QString describe(int baseIndent = 0) const;
        QString toString() const;
    };

private:
    QString m_className;
    QString m_filePath;
    QList<Export> m_exports;
    QString m_superName;
    QList<FakeMetaEnum> m_enums;
    QHash<QString, int> m_enumNameToIndex;
    QList<FakeMetaProperty> m_props;
    QHash<QString, int> m_propNameToIdx;
    QList<FakeMetaMethod> m_methods;
    QString m_defaultPropertyName;
    QString m_attachedTypeName;
    QString m_extensionTypeName;
    QByteArray m_fingerprint;
    bool m_isSingleton;
    bool m_isCreatable;
    bool m_isComposite;

public:
    FakeMetaObject();

    QString className() const;
    void setClassName(const QString &name);

    QString filePath() const;
    void setFilePath(const QString &path);

    void addExport(const QString &name, const QString &package, ComponentVersion version);
    void setExportMetaObjectRevision(int exportIndex, int metaObjectRevision);
    const QList<Export> exports() const;
    Export exportInPackage(const QString &package) const;

    void setSuperclassName(const QString &superclass);
    QString superclassName() const;

    void addEnum(const FakeMetaEnum &fakeEnum);
    int enumeratorCount() const;
    int enumeratorOffset() const;
    FakeMetaEnum enumerator(int index) const;
    int enumeratorIndex(const QString &name) const;

    void addProperty(const FakeMetaProperty &property);
    int propertyCount() const;
    int propertyOffset() const;
    FakeMetaProperty property(int index) const;
    int propertyIndex(const QString &name) const;

    void addMethod(const FakeMetaMethod &method);
    int methodCount() const;
    int methodOffset() const;
    FakeMetaMethod method(int index) const;
    int methodIndex(const QString &name) const; // Note: Returns any method with that name in case of overloads

    QString defaultPropertyName() const;
    void setDefaultPropertyName(const QString &defaultPropertyName);

    QString attachedTypeName() const;
    void setAttachedTypeName(const QString &name);
    QString extensionTypeName() const;
    void setExtensionTypeName(const QString &name);
    QByteArray calculateFingerprint() const;
    void updateFingerprint();
    QByteArray fingerprint() const;

    bool isSingleton() const;
    bool isCreatable() const;
    bool isComposite() const;
    void setIsSingleton(bool value);
    void setIsCreatable(bool value);
    void setIsComposite(bool value);

    QString describe(bool printDetails = true, int baseIndent = 0) const;
    QString toString() const;
};

} // namespace LanguageUtils
