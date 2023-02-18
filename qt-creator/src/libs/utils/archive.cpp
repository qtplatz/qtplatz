// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "archive.h"

#include "algorithm.h"
#include "mimeutils.h"
#include "qtcassert.h"
#include "qtcprocess.h"

#include <QSettings>

namespace Utils {

namespace {

struct Tool
{
    CommandLine command;
    QStringList supportedMimeTypes;
    FilePaths additionalSearchDirs;
};

} // anon

static FilePaths additionalInstallDirs(const QString &registryKey, const QString &valueName)
{
#if defined(Q_OS_WIN)
    const QSettings settings64(registryKey, QSettings::Registry64Format);
    const QSettings settings32(registryKey, QSettings::Registry32Format);
    return {FilePath::fromVariant(settings64.value(valueName)),
            FilePath::fromVariant(settings32.value(valueName))};
#else
    Q_UNUSED(registryKey)
    Q_UNUSED(valueName)
    return {};
#endif
}

static const QVector<Tool> &sTools()
{
    static QVector<Tool> tools;
    if (tools.isEmpty()) {
        if (HostOsInfo::isWindowsHost()) {
            tools << Tool{{"powershell", "-command Expand-Archive -Force '%{src}' '%{dest}'", CommandLine::Raw},
                          {"application/zip"},
                          {}};
        }
        tools << Tool{{"unzip", {"-o", "%{src}", "-d", "%{dest}"}}, {"application/zip"}, {}};
        tools << Tool{{"7z", {"x", "-o%{dest}", "-y", "-bb", "%{src}"}},
                      {"application/zip", "application/x-7z-compressed"},
                      additionalInstallDirs("HKEY_CURRENT_USER\\Software\\7-Zip", "Path")};
        tools << Tool{{"tar", {"xvf", "%{src}"}},
                      {"application/zip", "application/x-tar", "application/x-7z-compressed"},
                      {}};
        tools << Tool{{"tar", {"xvzf", "%{src}"}}, {"application/x-compressed-tar"}, {}};
        tools << Tool{{"tar", {"xvJf", "%{src}"}}, {"application/x-xz-compressed-tar"}, {}};
        tools << Tool{{"tar", {"xvjf", "%{src}"}}, {"application/x-bzip-compressed-tar"}, {}};

        const FilePaths additionalCMakeDirs =
                additionalInstallDirs("HKEY_LOCAL_MACHINE\\SOFTWARE\\Kitware\\CMake",
                                      "InstallDir");
        tools << Tool{{"cmake", {"-E", "tar", "xvf", "%{src}"}},
                      {"application/zip", "application/x-tar", "application/x-7z-compressed"},
                      additionalCMakeDirs};
        tools << Tool{{"cmake", {"-E", "tar", "xvzf", "%{src}"}},
                      {"application/x-compressed-tar"},
                      additionalCMakeDirs};
        tools << Tool{{"cmake", {"-E", "tar", "xvJf", "%{src}"}},
                      {"application/x-xz-compressed-tar"},
                      additionalCMakeDirs};
        tools << Tool{{"cmake", {"-E", "tar", "xvjf", "%{src}"}},
                      {"application/x-bzip-compressed-tar"},
                      additionalCMakeDirs};
    }
    return tools;
}

static QVector<Tool> toolsForMimeType(const MimeType &mimeType)
{
    return Utils::filtered(sTools(), [mimeType](const Tool &tool) {
        return Utils::anyOf(tool.supportedMimeTypes,
                            [mimeType](const QString &mt) { return mimeType.inherits(mt); });
    });
}

static QVector<Tool> toolsForFilePath(const FilePath &fp)
{
    return toolsForMimeType(mimeTypeForFile(fp));
}

static std::optional<Tool> resolveTool(const Tool &tool)
{
    const FilePath executable =
        tool.command.executable().withExecutableSuffix().searchInPath(tool.additionalSearchDirs);
    Tool resolvedTool = tool;
    resolvedTool.command.setExecutable(executable);
    return executable.isEmpty() ? std::nullopt : std::make_optional(resolvedTool);
}

static std::optional<Tool> unzipTool(const FilePath &src, const FilePath &dest)
{
    const QVector<Tool> tools = toolsForFilePath(src);
    for (const Tool &tool : tools) {
        const std::optional<Tool> resolvedTool = resolveTool(tool);
        if (resolvedTool) {
            Tool result = *resolvedTool;
            const QString srcStr = src.toString();
            const QString destStr = dest.toString();
            const QString args = result.command.arguments().replace("%{src}", srcStr).replace("%{dest}", destStr);
            result.command.setArguments(args);
            return result;
        }
    }
    return {};
}

bool Archive::supportsFile(const FilePath &filePath, QString *reason)
{
    const QVector<Tool> tools = toolsForFilePath(filePath);
    if (tools.isEmpty()) {
        if (reason)
            *reason = tr("File format not supported.");
        return false;
    }
    if (!anyOf(tools, [tools](const Tool &t) { return resolveTool(t); })) {
        if (reason) {
            const QStringList execs = transform<QStringList>(tools, [](const Tool &tool) {
                return tool.command.executable().toString();
            });
            *reason = tr("Could not find any unarchiving executable in PATH (%1).")
                          .arg(execs.join(", "));
        }
        return false;
    }
    return true;
}

Archive::Archive(const FilePath &src, const FilePath &dest)
{
    const std::optional<Tool> tool = unzipTool(src, dest);
    if (!tool)
        return;
    m_commandLine = tool->command;
    m_workingDirectory = dest.absoluteFilePath();
}

Archive::~Archive() = default;

bool Archive::isValid() const
{
    return !m_commandLine.isEmpty();
}

void Archive::unarchive()
{
    QTC_ASSERT(isValid(), return);
    QTC_ASSERT(!m_process, return);

    m_workingDirectory.ensureWritableDir();

    m_process.reset(new QtcProcess);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    QObject::connect(m_process.get(), &QtcProcess::readyReadStandardOutput, this, [this] {
        emit outputReceived(QString::fromUtf8(m_process->readAllStandardOutput()));
    });
    QObject::connect(m_process.get(), &QtcProcess::done, this, [this] {
        const bool successfulFinish = m_process->result() == ProcessResult::FinishedWithSuccess;
        if (!successfulFinish)
            emit outputReceived(tr("Command failed."));
        emit finished(successfulFinish);
    });

    emit outputReceived(tr("Running %1\nin \"%2\".\n\n", "Running <cmd> in <workingdirectory>")
                 .arg(m_commandLine.toUserOutput(), m_workingDirectory.toUserOutput()));

    m_process->setCommand(m_commandLine);
    m_process->setWorkingDirectory(m_workingDirectory);
    m_process->start();
}

} // namespace Utils
