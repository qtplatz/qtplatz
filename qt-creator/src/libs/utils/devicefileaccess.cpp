// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "devicefileaccess.h"

#include "algorithm.h"
#include "qtcassert.h"
#include "hostosinfo.h"
#include "commandline.h"

#include <QCoreApplication>
#include <QOperatingSystemVersion>
#include <QRegularExpression>
#include <QStorageInfo>

#ifdef Q_OS_WIN
#ifdef QTCREATOR_PCH_H
#define CALLBACK WINAPI
#endif
#include <qt_windows.h>
#include <shlobj.h>
#else
#include <qplatformdefs.h>
#endif

namespace Utils {

// DeviceFileAccess

DeviceFileAccess::~DeviceFileAccess() = default;

QString DeviceFileAccess::mapToDevicePath(const FilePath &filePath) const
{
    return filePath.path();
}

bool DeviceFileAccess::isExecutableFile(const FilePath &filePath) const
{
    Q_UNUSED(filePath)
    QTC_CHECK(false);
    return false;
}

bool DeviceFileAccess::isReadableFile(const FilePath &filePath) const
{
    Q_UNUSED(filePath)
    QTC_CHECK(false);
    return false;
}

bool DeviceFileAccess::isWritableFile(const FilePath &filePath) const
{
    Q_UNUSED(filePath)
    QTC_CHECK(false);
    return false;
}

bool DeviceFileAccess::isReadableDirectory(const FilePath &filePath) const
{
    Q_UNUSED(filePath)
    QTC_CHECK(false);
    return false;
}

bool DeviceFileAccess::isWritableDirectory(const FilePath &filePath) const
{
    Q_UNUSED(filePath)
    QTC_CHECK(false);
    return false;
}

bool DeviceFileAccess::isFile(const FilePath &filePath) const
{
    Q_UNUSED(filePath)
    QTC_CHECK(false);
    return false;
}

bool DeviceFileAccess::isDirectory(const FilePath &filePath) const
{
    Q_UNUSED(filePath)
    QTC_CHECK(false);
    return false;
}

bool DeviceFileAccess::isSymLink(const FilePath &filePath) const
{
    Q_UNUSED(filePath)
    QTC_CHECK(false);
    return false;
}

bool DeviceFileAccess::ensureWritableDirectory(const FilePath &filePath) const
{
    if (isWritableDirectory(filePath))
        return true;
    return createDirectory(filePath);
}

bool DeviceFileAccess::ensureExistingFile(const FilePath &filePath) const
{
    Q_UNUSED(filePath)
    QTC_CHECK(false);
    return false;
}

bool DeviceFileAccess::createDirectory(const FilePath &filePath) const
{
    Q_UNUSED(filePath)
    QTC_CHECK(false);
    return false;
}

bool DeviceFileAccess::exists(const FilePath &filePath) const
{
    Q_UNUSED(filePath)
    QTC_CHECK(false);
    return false;
}

bool DeviceFileAccess::removeFile(const FilePath &filePath) const
{
    Q_UNUSED(filePath)
    QTC_CHECK(false);
    return false;
}

bool DeviceFileAccess::removeRecursively(const FilePath &filePath, QString *error) const
{
    Q_UNUSED(filePath)
    Q_UNUSED(error)
    QTC_CHECK(false);
    return false;
}

bool DeviceFileAccess::copyFile(const FilePath &filePath, const FilePath &target) const
{
    Q_UNUSED(filePath)
    Q_UNUSED(target)
    QTC_CHECK(false);
    return false;
}

bool DeviceFileAccess::renameFile(const FilePath &filePath, const FilePath &target) const
{
    Q_UNUSED(filePath)
    Q_UNUSED(target)
    QTC_CHECK(false);
    return false;
}

OsType DeviceFileAccess::osType(const FilePath &filePath) const
{
    Q_UNUSED(filePath)
    return OsTypeOther;
}

FilePath DeviceFileAccess::symLinkTarget(const FilePath &filePath) const
{
    Q_UNUSED(filePath)
    QTC_CHECK(false);
    return {};
}

void DeviceFileAccess::iterateDirectory(
        const FilePath &filePath,
        const FilePath::IterateDirCallback &callBack,
        const FileFilter &filter) const
{
    Q_UNUSED(filePath)
    Q_UNUSED(callBack)
    Q_UNUSED(filter)
    QTC_CHECK(false);
}

std::optional<QByteArray> DeviceFileAccess::fileContents(
        const FilePath &filePath,
        qint64 limit,
        qint64 offset) const
{
    Q_UNUSED(filePath)
    Q_UNUSED(limit)
    Q_UNUSED(offset)
    QTC_CHECK(false);
    return {};
}

bool DeviceFileAccess::writeFileContents(
        const FilePath &filePath,
        const QByteArray &data,
        qint64 offset) const
{
    Q_UNUSED(filePath)
    Q_UNUSED(data)
    Q_UNUSED(offset)
    QTC_CHECK(false);
    return false;
}

FilePathInfo DeviceFileAccess::filePathInfo(const FilePath &filePath) const
{
    Q_UNUSED(filePath)
    QTC_CHECK(false);
    return {};
}

QDateTime DeviceFileAccess::lastModified(const FilePath &filePath) const
{
    Q_UNUSED(filePath)
    QTC_CHECK(false);
    return {};
}

QFile::Permissions DeviceFileAccess::permissions(const FilePath &filePath) const
{
    Q_UNUSED(filePath)
    QTC_CHECK(false);
    return {};
}

bool DeviceFileAccess::setPermissions(const FilePath &filePath, QFile::Permissions) const
{
    Q_UNUSED(filePath)
    QTC_CHECK(false);
    return false;
}

qint64 DeviceFileAccess::fileSize(const FilePath &filePath) const
{
    Q_UNUSED(filePath)
    QTC_CHECK(false);
    return false;
}

qint64 DeviceFileAccess::bytesAvailable(const FilePath &filePath) const
{
    Q_UNUSED(filePath)
    QTC_CHECK(false);
    return -1;
}

QByteArray DeviceFileAccess::fileId(const FilePath &filePath) const
{
    Q_UNUSED(filePath);
    QTC_CHECK(false);
    return {};
}

void DeviceFileAccess::asyncFileContents(
        const FilePath &filePath,
        const Continuation<std::optional<QByteArray>> &cont,
        qint64 limit,
        qint64 offset) const
{
    cont(fileContents(filePath, limit, offset));
}

void DeviceFileAccess::asyncWriteFileContents(
        const FilePath &filePath,
        const Continuation<bool> &cont,
        const QByteArray &data,
        qint64 offset) const
{
    cont(writeFileContents(filePath, data, offset));
}

void DeviceFileAccess::asyncCopyFile(
        const FilePath &filePath,
        const Continuation<bool> &cont,
        const FilePath &target) const
{
    cont(copyFile(filePath, target));
}


// DesktopDeviceFileAccess

DesktopDeviceFileAccess::~DesktopDeviceFileAccess() = default;

DesktopDeviceFileAccess *DesktopDeviceFileAccess::instance()
{
    static DesktopDeviceFileAccess theInstance;
    return &theInstance;
}

bool DesktopDeviceFileAccess::isExecutableFile(const FilePath &filePath) const
{
    const QFileInfo fi(filePath.path());
    return fi.isExecutable() && !fi.isDir();
}

bool DesktopDeviceFileAccess::isReadableFile(const FilePath &filePath) const
{
    const QFileInfo fi(filePath.path());
    return fi.exists() && fi.isFile() && fi.isReadable();
}

bool DesktopDeviceFileAccess::isWritableFile(const FilePath &filePath) const
{
    const QFileInfo fi(filePath.path());
    return fi.exists() && fi.isFile() && fi.isWritable();
}

bool DesktopDeviceFileAccess::isReadableDirectory(const FilePath &filePath) const
{
    const QFileInfo fi(filePath.path());
    return fi.exists() && fi.isDir() && fi.isReadable();
}

bool DesktopDeviceFileAccess::isWritableDirectory(const FilePath &filePath) const
{
    const QFileInfo fi(filePath.path());
    return fi.exists() && fi.isDir() && fi.isWritable();
}

bool DesktopDeviceFileAccess::isFile(const FilePath &filePath) const
{
    const QFileInfo fi(filePath.path());
    return fi.isFile();
}

bool DesktopDeviceFileAccess::isDirectory(const FilePath &filePath) const
{
    const QFileInfo fi(filePath.path());
    return fi.isDir();
}

bool DesktopDeviceFileAccess::isSymLink(const FilePath &filePath) const
{
    const QFileInfo fi(filePath.path());
    return fi.isSymLink();
}

bool DesktopDeviceFileAccess::ensureWritableDirectory(const FilePath &filePath) const
{
    const QFileInfo fi(filePath.path());
    if (fi.isDir() && fi.isWritable())
        return true;
    return QDir().mkpath(filePath.path());
}

bool DesktopDeviceFileAccess::ensureExistingFile(const FilePath &filePath) const
{
    QFile f(filePath.path());
    if (f.exists())
        return true;
    f.open(QFile::WriteOnly);
    f.close();
    return f.exists();
}

bool DesktopDeviceFileAccess::createDirectory(const FilePath &filePath) const
{
    QDir dir(filePath.path());
    return dir.mkpath(dir.absolutePath());
}

bool DesktopDeviceFileAccess::exists(const FilePath &filePath) const
{
    return !filePath.isEmpty() && QFileInfo::exists(filePath.path());
}

bool DesktopDeviceFileAccess::removeFile(const FilePath &filePath) const
{
    return QFile::remove(filePath.path());
}

bool DesktopDeviceFileAccess::removeRecursively(const FilePath &filePath, QString *error) const
{
    QTC_ASSERT(!filePath.needsDevice(), return false);
    QFileInfo fileInfo = filePath.toFileInfo();
    if (!fileInfo.exists() && !fileInfo.isSymLink())
        return true;

    QFile::setPermissions(fileInfo.absoluteFilePath(), fileInfo.permissions() | QFile::WriteUser);

    if (fileInfo.isDir()) {
        QDir dir(fileInfo.absoluteFilePath());
        dir.setPath(dir.canonicalPath());
        if (dir.isRoot()) {
            if (error) {
                *error = QCoreApplication::translate("Utils::FileUtils",
                    "Refusing to remove root directory.");
            }
            return false;
        }
        if (dir.path() == QDir::home().canonicalPath()) {
            if (error) {
                *error = QCoreApplication::translate("Utils::FileUtils",
                    "Refusing to remove your home directory.");
            }
            return false;
        }

        const QStringList fileNames = dir.entryList(
                    QDir::Files | QDir::Hidden | QDir::System | QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &fileName : fileNames) {
            if (!removeRecursively(filePath / fileName, error))
                return false;
        }
        if (!QDir::root().rmdir(dir.path())) {
            if (error) {
                *error = QCoreApplication::translate("Utils::FileUtils", "Failed to remove directory \"%1\".")
                        .arg(filePath.toUserOutput());
            }
            return false;
        }
    } else {
        if (!QFile::remove(filePath.toString())) {
            if (error) {
                *error = QCoreApplication::translate("Utils::FileUtils", "Failed to remove file \"%1\".")
                        .arg(filePath.toUserOutput());
            }
            return false;
        }
    }
    return true;
}

bool DesktopDeviceFileAccess::copyFile(const FilePath &filePath, const FilePath &target) const
{
    return QFile::copy(filePath.path(), target.path());
}

bool DesktopDeviceFileAccess::renameFile(const FilePath &filePath, const FilePath &target) const
{
    return QFile::rename(filePath.path(), target.path());
}

FilePathInfo DesktopDeviceFileAccess::filePathInfo(const FilePath &filePath) const
{
    FilePathInfo result;

    QFileInfo fi(filePath.path());
    result.fileSize = fi.size();
    result.lastModified = fi.lastModified();
    result.fileFlags = (FilePathInfo::FileFlag) int(fi.permissions());

    if (fi.isDir())
        result.fileFlags |= FilePathInfo::DirectoryType;
    if (fi.isFile())
        result.fileFlags |= FilePathInfo::FileType;
    if (fi.exists())
        result.fileFlags |= FilePathInfo::ExistsFlag;
    if (fi.isSymbolicLink())
        result.fileFlags |= FilePathInfo::LinkType;
    if (fi.isBundle())
        result.fileFlags |= FilePathInfo::BundleType;
    if (fi.isHidden())
        result.fileFlags |= FilePathInfo::HiddenFlag;
    if (fi.isRoot())
        result.fileFlags |= FilePathInfo::RootFlag;

    return result;
}

FilePath DesktopDeviceFileAccess::symLinkTarget(const FilePath &filePath) const
{
    const QFileInfo info(filePath.path());
    if (!info.isSymLink())
        return {};
    return FilePath::fromString(info.symLinkTarget());
}

void DesktopDeviceFileAccess::iterateDirectory(
        const FilePath &filePath,
        const FilePath::IterateDirCallback &callBack,
        const FileFilter &filter) const
{
    QDirIterator it(filePath.path(), filter.nameFilters, filter.fileFilters, filter.iteratorFlags);
    while (it.hasNext()) {
        const FilePath path = FilePath::fromString(it.next());
        bool res = false;
        if (callBack.index() == 0)
            res = std::get<0>(callBack)(path);
        else
            res = std::get<1>(callBack)(path, path.filePathInfo());
        if (!res)
            return;
    }
}

std::optional<QByteArray> DesktopDeviceFileAccess::fileContents(
        const FilePath &filePath,
        qint64 limit,
        qint64 offset) const
{
    const QString path = filePath.path();
    QFile f(path);
    if (!f.exists())
        return {};

    if (!f.open(QFile::ReadOnly))
        return {};

    if (offset != 0)
        f.seek(offset);

    if (limit != -1)
        return f.read(limit);

    return f.readAll();
}

bool DesktopDeviceFileAccess::writeFileContents(
        const FilePath &filePath,
        const QByteArray &data,
        qint64 offset) const
{
    QFile file(filePath.path());
    QTC_ASSERT(file.open(QFile::WriteOnly | QFile::Truncate), return false);
    if (offset != 0)
        file.seek(offset);
    qint64 res = file.write(data);
    return res == data.size();
}

QDateTime DesktopDeviceFileAccess::lastModified(const FilePath &filePath) const
{
    return QFileInfo(filePath.path()).lastModified();
}

QFile::Permissions DesktopDeviceFileAccess::permissions(const FilePath &filePath) const
{
    return QFileInfo(filePath.path()).permissions();
}

bool DesktopDeviceFileAccess::setPermissions(const FilePath &filePath,
                                             QFile::Permissions permissions) const
{
    return QFile(filePath.path()).setPermissions(permissions);
}

qint64 DesktopDeviceFileAccess::fileSize(const FilePath &filePath) const
{
    return QFileInfo(filePath.path()).size();
}

qint64 DesktopDeviceFileAccess::bytesAvailable(const FilePath &filePath) const
{
    return QStorageInfo(filePath.path()).bytesAvailable();
}

// Copied from qfilesystemengine_win.cpp
#ifdef Q_OS_WIN

// File ID for Windows up to version 7.
static inline QByteArray fileIdWin7(HANDLE handle)
{
    BY_HANDLE_FILE_INFORMATION info;
    if (GetFileInformationByHandle(handle, &info)) {
        char buffer[sizeof "01234567:0123456701234567\0"];
        qsnprintf(buffer, sizeof(buffer), "%lx:%08lx%08lx",
                  info.dwVolumeSerialNumber,
                  info.nFileIndexHigh,
                  info.nFileIndexLow);
        return QByteArray(buffer);
    }
    return QByteArray();
}

// File ID for Windows starting from version 8.
static QByteArray fileIdWin8(HANDLE handle)
{
    QByteArray result;
    FILE_ID_INFO infoEx;
    if (GetFileInformationByHandleEx(handle,
                                     static_cast<FILE_INFO_BY_HANDLE_CLASS>(18), // FileIdInfo in Windows 8
                                     &infoEx, sizeof(FILE_ID_INFO))) {
        result = QByteArray::number(infoEx.VolumeSerialNumber, 16);
        result += ':';
        // Note: MinGW-64's definition of FILE_ID_128 differs from the MSVC one.
        result += QByteArray(reinterpret_cast<const char *>(&infoEx.FileId), int(sizeof(infoEx.FileId))).toHex();
    }
    return result;
}

static QByteArray fileIdWin(HANDLE fHandle)
{
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows8 ?
                fileIdWin8(HANDLE(fHandle)) : fileIdWin7(HANDLE(fHandle));
}
#endif

QByteArray DesktopDeviceFileAccess::fileId(const FilePath &filePath) const
{
    QByteArray result;

#ifdef Q_OS_WIN
    const HANDLE handle =
            CreateFile((wchar_t*)filePath.toUserOutput().utf16(), 0,
                       FILE_SHARE_READ, NULL, OPEN_EXISTING,
                       FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (handle != INVALID_HANDLE_VALUE) {
        result = fileIdWin(handle);
        CloseHandle(handle);
    }
#else // Copied from qfilesystemengine_unix.cpp
    if (Q_UNLIKELY(filePath.isEmpty()))
        return result;

    QT_STATBUF statResult;
    if (QT_STAT(filePath.toString().toLocal8Bit().constData(), &statResult))
        return result;
    result = QByteArray::number(quint64(statResult.st_dev), 16);
    result += ':';
    result += QByteArray::number(quint64(statResult.st_ino));
#endif
    return result;
}

OsType DesktopDeviceFileAccess::osType(const FilePath &filePath) const
{
    Q_UNUSED(filePath);
    return HostOsInfo::hostOs();
}


// UnixDeviceAccess

UnixDeviceFileAccess::~UnixDeviceFileAccess() = default;

bool UnixDeviceFileAccess::runInShellSuccess(const CommandLine &cmdLine,
                                             const QByteArray &stdInData) const
{
    return runInShell(cmdLine, stdInData).exitCode == 0;
}

bool UnixDeviceFileAccess::isExecutableFile(const FilePath &filePath) const
{
    const QString path = filePath.path();
    return runInShellSuccess({"test", {"-x", path}, OsType::OsTypeLinux});
}

bool UnixDeviceFileAccess::isReadableFile(const FilePath &filePath) const
{
    const QString path = filePath.path();
    return runInShellSuccess({"test", {"-r", path, "-a", "-f", path}, OsType::OsTypeLinux});
}

bool UnixDeviceFileAccess::isWritableFile(const FilePath &filePath) const
{
    const QString path = filePath.path();
    return runInShellSuccess({"test", {"-w", path, "-a", "-f", path}, OsType::OsTypeLinux});
}

bool UnixDeviceFileAccess::isReadableDirectory(const FilePath &filePath) const
{
    const QString path = filePath.path();
    return runInShellSuccess({"test", {"-r", path, "-a", "-d", path}, OsType::OsTypeLinux});
}

bool UnixDeviceFileAccess::isWritableDirectory(const FilePath &filePath) const
{
    const QString path = filePath.path();
    return runInShellSuccess({"test", {"-w", path, "-a", "-d", path}, OsType::OsTypeLinux});
}

bool UnixDeviceFileAccess::isFile(const FilePath &filePath) const
{
    const QString path = filePath.path();
    return runInShellSuccess({"test", {"-f", path}, OsType::OsTypeLinux});
}

bool UnixDeviceFileAccess::isDirectory(const FilePath &filePath) const
{
    const QString path = filePath.path();
    return runInShellSuccess({"test", {"-d", path}, OsType::OsTypeLinux});
}

bool UnixDeviceFileAccess::isSymLink(const FilePath &filePath) const
{
    const QString path = filePath.path();
    return runInShellSuccess({"test", {"-h", path}, OsType::OsTypeLinux});
}

bool UnixDeviceFileAccess::ensureExistingFile(const FilePath &filePath) const
{
    const QString path = filePath.path();
    return runInShellSuccess({"touch", {path}, OsType::OsTypeLinux});
}

bool UnixDeviceFileAccess::createDirectory(const FilePath &filePath) const
{
    const QString path = filePath.path();
    return runInShellSuccess({"mkdir", {"-p", path}, OsType::OsTypeLinux});
}

bool UnixDeviceFileAccess::exists(const FilePath &filePath) const
{
    const QString path = filePath.path();
    return runInShellSuccess({"test", {"-e", path}, OsType::OsTypeLinux});
}

bool UnixDeviceFileAccess::removeFile(const FilePath &filePath) const
{
    return runInShellSuccess({"rm", {filePath.path()}, OsType::OsTypeLinux});
}

bool UnixDeviceFileAccess::removeRecursively(const FilePath &filePath, QString *error) const
{
    QTC_ASSERT(filePath.path().startsWith('/'), return false);

    const QString path = filePath.cleanPath().path();
    // We are expecting this only to be called in a context of build directories or similar.
    // Chicken out in some cases that _might_ be user code errors.
    QTC_ASSERT(path.startsWith('/'), return false);
    int levelsNeeded = path.startsWith("/home/") ? 4 : 3;
    if (path.startsWith("/tmp/"))
        levelsNeeded = 2;
    QTC_ASSERT(path.count('/') >= levelsNeeded, return false);

    RunResult result = runInShell({"rm", {"-rf", "--", path}, OsType::OsTypeLinux});
    if (error)
        *error = QString::fromUtf8(result.stdErr);
    return result.exitCode == 0;
}

bool UnixDeviceFileAccess::copyFile(const FilePath &filePath, const FilePath &target) const
{
    return runInShellSuccess({"cp", {filePath.path(), target.path()}, OsType::OsTypeLinux});
}

bool UnixDeviceFileAccess::renameFile(const FilePath &filePath, const FilePath &target) const
{
    return runInShellSuccess({"mv", {filePath.path(), target.path()}, OsType::OsTypeLinux});
}

FilePath UnixDeviceFileAccess::symLinkTarget(const FilePath &filePath) const
{
    const RunResult result = runInShell({"readlink", {"-n", "-e", filePath.path()}, OsType::OsTypeLinux});
    const QString out = QString::fromUtf8(result.stdOut);
    return out.isEmpty() ? FilePath() : filePath.withNewPath(out);
}

std::optional<QByteArray> UnixDeviceFileAccess::fileContents(
        const FilePath &filePath,
        qint64 limit,
        qint64 offset) const
{
    QStringList args = {"if=" + filePath.path(), "status=none"};
    if (limit > 0 || offset > 0) {
        const qint64 gcd = std::gcd(limit, offset);
        args += QString("bs=%1").arg(gcd);
        args += QString("count=%1").arg(limit / gcd);
        args += QString("seek=%1").arg(offset / gcd);
    }

    const RunResult r = runInShell({"dd", args, OsType::OsTypeLinux});

    if (r.exitCode != 0)
        return {};

    return r.stdOut;
}

bool UnixDeviceFileAccess::writeFileContents(
        const FilePath &filePath,
        const QByteArray &data,
        qint64 offset) const
{
    QStringList args = {"of=" + filePath.path()};
    if (offset != 0) {
        args.append("bs=1");
        args.append(QString("seek=%1").arg(offset));
    }
    return runInShellSuccess({"dd", args, OsType::OsTypeLinux}, data);
}

OsType UnixDeviceFileAccess::osType(const FilePath &filePath) const
{
    Q_UNUSED(filePath)
    return OsTypeLinux;
}

QDateTime UnixDeviceFileAccess::lastModified(const FilePath &filePath) const
{
    const RunResult result = runInShell({"stat", {"-L", "-c", "%Y", filePath.path()}, OsType::OsTypeLinux});
    qint64 secs = result.stdOut.toLongLong();
    const QDateTime dt = QDateTime::fromSecsSinceEpoch(secs, Qt::UTC);
    return dt;
}

QFile::Permissions UnixDeviceFileAccess::permissions(const FilePath &filePath) const
{
    const RunResult result = runInShell({"stat", {"-L", "-c", "%a", filePath.path()}, OsType::OsTypeLinux});
    const uint bits = result.stdOut.toUInt(nullptr, 8);
    QFileDevice::Permissions perm = {};
#define BIT(n, p) if (bits & (1<<n)) perm |= QFileDevice::p
    BIT(0, ExeOther);
    BIT(1, WriteOther);
    BIT(2, ReadOther);
    BIT(3, ExeGroup);
    BIT(4, WriteGroup);
    BIT(5, ReadGroup);
    BIT(6, ExeUser);
    BIT(7, WriteUser);
    BIT(8, ReadUser);
#undef BIT
    return perm;
}

bool UnixDeviceFileAccess::setPermissions(const FilePath &filePath, QFile::Permissions perms) const
{
    const int flags = int(perms);
    return runInShellSuccess({"chmod", {QString::number(flags, 16), filePath.path()}, OsType::OsTypeLinux});
}

qint64 UnixDeviceFileAccess::fileSize(const FilePath &filePath) const
{
    const RunResult result = runInShell({"stat", {"-L", "-c", "%s", filePath.path()}, OsType::OsTypeLinux});
    return result.stdOut.toLongLong();
}

qint64 UnixDeviceFileAccess::bytesAvailable(const FilePath &filePath) const
{
    const RunResult result = runInShell({"df", {"-k", filePath.path()}, OsType::OsTypeLinux});
    return FileUtils::bytesAvailableFromDFOutput(result.stdOut);
}

QByteArray UnixDeviceFileAccess::fileId(const FilePath &filePath) const
{
    const RunResult result = runInShell({"stat", {"-L", "-c", "%D:%i", filePath.path()}, OsType::OsTypeLinux});
    if (result.exitCode != 0)
        return {};

    return result.stdOut;
}

FilePathInfo UnixDeviceFileAccess::filePathInfo(const FilePath &filePath) const
{
    const RunResult stat = runInShell({"stat", {"-L", "-c", "%f %Y %s", filePath.path()}, OsType::OsTypeLinux});
    return FileUtils::filePathInfoFromTriple(QString::fromLatin1(stat.stdOut));
}

// returns whether 'find' could be used.
bool UnixDeviceFileAccess::iterateWithFind(
        const FilePath &filePath,
        const FileFilter &filter,
        const FilePath::IterateDirCallback &callBack) const
{
    QTC_CHECK(filePath.isAbsolutePath());

    CommandLine cmdLine{"find", filter.asFindArguments(filePath.path()), OsType::OsTypeLinux};

    // TODO: Using stat -L will always return the link target, not the link itself.
    // We may wan't to add the information that it is a link at some point.
    if (callBack.index() == 1)
        cmdLine.addArgs(R"(-exec echo -n \"{}\"" " \; -exec stat -L -c "%f %Y %s" "{}" \;)", CommandLine::Raw);

    const RunResult result = runInShell(cmdLine);
    const QString out = QString::fromUtf8(result.stdOut);
    if (result.exitCode != 0) {
        // Find returns non-zero exit code for any error it encounters, even if it finds some files.

        if (!out.startsWith('"' + filePath.path())) {
            if (!filePath.exists()) // File does not exist, so no files to find.
                return true;

            // If the output does not start with the path we are searching in, find has failed.
            // Possibly due to unknown options.
            return false;
        }
    }

    QStringList entries = out.split("\n", Qt::SkipEmptyParts);
    if (entries.isEmpty())
        return true;

    const auto toFilePath = [&filePath, &callBack](const QString &entry) -> bool {
        if (callBack.index() == 0)
            return std::get<0>(callBack)(filePath.withNewPath(entry));

        const QString fileName = entry.mid(1, entry.lastIndexOf('\"') - 1);
        const QString infos = entry.mid(fileName.length() + 3);

        const FilePathInfo fi = FileUtils::filePathInfoFromTriple(infos);
        if (!fi.fileFlags)
            return true;

        const FilePath fp = filePath.withNewPath(fileName);
        // Do not return the entry for the directory we are searching in.
        if (fp.path() == filePath.path())
            return true;
        return std::get<1>(callBack)(fp, fi);
    };

    // Remove the first line, this can be the directory we are searching in.
    // as long as we do not specify "mindepth > 0"
    if (entries.front() == filePath.path())
        entries.pop_front();

    for (const QString &entry : entries) {
        if (!toFilePath(entry))
            break;
    }

    return true;
}

void UnixDeviceFileAccess::findUsingLs(
        const QString &current,
        const FileFilter &filter,
        QStringList *found) const
{
    const RunResult result = runInShell({"ls", {"-1", "-p", "--", current}, OsType::OsTypeLinux});
    const QStringList entries  = QString::fromUtf8(result.stdOut).split('\n', Qt::SkipEmptyParts);
    for (QString entry : entries) {
        const QChar last = entry.back();
        if (last == '/') {
            entry.chop(1);
            if (filter.iteratorFlags.testFlag(QDirIterator::Subdirectories))
                findUsingLs(current + '/' + entry, filter, found);
        }
        found->append(entry);
    }
}

// Used on 'ls' output on unix-like systems.
static void iterateLsOutput(const FilePath &base,
                            const QStringList &entries,
                            const FileFilter &filter,
                            const FilePath::IterateDirCallback &callBack)
{
    const QList<QRegularExpression> nameRegexps =
            transform(filter.nameFilters, [](const QString &filter) {
        QRegularExpression re;
        re.setPattern(QRegularExpression::wildcardToRegularExpression(filter));
        QTC_CHECK(re.isValid());
        return re;
    });

    const auto nameMatches = [&nameRegexps](const QString &fileName) {
        for (const QRegularExpression &re : nameRegexps) {
            const QRegularExpressionMatch match = re.match(fileName);
            if (match.hasMatch())
                return true;
        }
        return nameRegexps.isEmpty();
    };

    // FIXME: Handle filters. For now bark on unsupported options.
    QTC_CHECK(filter.fileFilters == QDir::NoFilter);

    for (const QString &entry : entries) {
        if (!nameMatches(entry))
            continue;
        const FilePath current = base.pathAppended(entry);
        bool res = false;
        if (callBack.index() == 0)
            res = std::get<0>(callBack)(current);
        else
            res = std::get<1>(callBack)(current, current.filePathInfo());
        if (!res)
            break;
    }
}

void UnixDeviceFileAccess::iterateDirectory(
        const FilePath &filePath,
        const FilePath::IterateDirCallback &callBack,
        const FileFilter &filter) const
{
    // We try to use 'find' first, because that can filter better directly.
    // Unfortunately, it's not installed on all devices by default.
    if (m_tryUseFind) {
        if (iterateWithFind(filePath, filter, callBack))
            return;
        m_tryUseFind = false; // remember the failure for the next time and use the 'ls' fallback below.
    }

    // if we do not have find - use ls as fallback
    QStringList entries;
    findUsingLs(filePath.path(), filter, &entries);
    iterateLsOutput(filePath, entries, filter, callBack);
}

} // Utils
