/****************************************************************************
**
** Copyright (C) 2016 Brian McGillion and Hugues Delorme
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "shellcommand.h"

#include "environment.h"
#include "qtcassert.h"
#include "qtcprocess.h"
#include "runextensions.h"

#include <QFuture>
#include <QFutureWatcher>
#include <QMutex>
#include <QTextCodec>
#include <QThread>
#include <QVariant>

#include <numeric>

/*!
    \fn void Utils::ProgressParser::parseProgress(const QString &text)

    Reimplement to parse progress as it appears in the standard output.
    If a progress string is detected, call \c setProgressAndMaximum() to update
    the progress bar accordingly.

    \sa Utils::ProgressParser::setProgressAndMaximum()
*/

/*!
    \fn void Utils::ProgressParser::setProgressAndMaximum(int value, int maximum)

    Sets progress \a value and \a maximum for current command. Called by \c parseProgress()
    when a progress string is detected.
*/

namespace Utils {
namespace Internal {

class ShellCommandPrivate
{
public:
    struct Job {
        explicit Job(const FilePath &wd, const CommandLine &command, int t,
                     const ExitCodeInterpreter &interpreter);

        FilePath workingDirectory;
        CommandLine command;
        ExitCodeInterpreter exitCodeInterpreter;
        int timeoutS;
    };

    ShellCommandPrivate(const FilePath &defaultWorkingDirectory, const Environment &environment)
        : m_defaultWorkingDirectory(defaultWorkingDirectory),
          m_environment(environment)
    {}

    ~ShellCommandPrivate() { delete m_progressParser; }

    QString m_displayName;
    const FilePath m_defaultWorkingDirectory;
    const Environment m_environment;
    QVariant m_cookie;
    QTextCodec *m_codec = nullptr;
    ProgressParser *m_progressParser = nullptr;
    QFutureWatcher<void> m_watcher;
    QList<Job> m_jobs;

    unsigned m_flags = 0;
    int m_defaultTimeoutS = 10;
    int m_lastExecExitCode = -1;

    bool m_lastExecSuccess = false;
    bool m_progressiveOutput = false;
    bool m_hadOutput = false;
    bool m_aborted = false;
    bool m_disableUnixTerminal = false;
};

ShellCommandPrivate::Job::Job(const FilePath &wd, const CommandLine &command,
                              int t, const ExitCodeInterpreter &interpreter) :
    workingDirectory(wd),
    command(command),
    exitCodeInterpreter(interpreter),
    timeoutS(t)
{
    // Finished cookie is emitted via queued slot, needs metatype
    static const int qvMetaId = qRegisterMetaType<QVariant>();
    Q_UNUSED(qvMetaId)
}

} // namespace Internal

ShellCommand::ShellCommand(const FilePath &workingDirectory, const Environment &environment) :
    d(new Internal::ShellCommandPrivate(workingDirectory, environment))
{
    connect(&d->m_watcher, &QFutureWatcher<void>::canceled, this, &ShellCommand::cancel);
}

ShellCommand::~ShellCommand()
{
    delete d;
}

QString ShellCommand::displayName() const
{
    if (!d->m_displayName.isEmpty())
        return d->m_displayName;
    if (!d->m_jobs.isEmpty()) {
        const Internal::ShellCommandPrivate::Job &job = d->m_jobs.at(0);
        QString result = job.command.executable().baseName();
        if (!result.isEmpty())
            result[0] = result.at(0).toTitleCase();
        else
            result = tr("UNKNOWN");

        if (!job.command.arguments().isEmpty())
            result += ' ' + job.command.splitArguments().at(0);

        return result;
    }
    return tr("Unknown");
}

void ShellCommand::setDisplayName(const QString &name)
{
    d->m_displayName = name;
}

const FilePath &ShellCommand::defaultWorkingDirectory() const
{
    return d->m_defaultWorkingDirectory;
}

const Environment ShellCommand::processEnvironment() const
{
    return d->m_environment;
}

int ShellCommand::defaultTimeoutS() const
{
    return d->m_defaultTimeoutS;
}

void ShellCommand::setDefaultTimeoutS(int timeout)
{
    d->m_defaultTimeoutS = timeout;
}

unsigned ShellCommand::flags() const
{
    return d->m_flags;
}

void ShellCommand::addFlags(unsigned f)
{
    d->m_flags |= f;
}

void ShellCommand::addJob(const CommandLine &command,
                          const FilePath &workingDirectory,
                          const ExitCodeInterpreter &interpreter)
{
    addJob(command, defaultTimeoutS(), workingDirectory, interpreter);
}

void ShellCommand::addJob(const CommandLine &command, int timeoutS,
                          const FilePath &workingDirectory,
                          const ExitCodeInterpreter &interpreter)
{
    d->m_jobs.push_back(Internal::ShellCommandPrivate::Job(workDirectory(workingDirectory), command,
                                                           timeoutS, interpreter));
}

void ShellCommand::execute()
{
    d->m_lastExecSuccess = false;
    d->m_lastExecExitCode = -1;

    if (d->m_jobs.empty())
        return;

    QFuture<void> task = runAsync(&ShellCommand::run, this);
    d->m_watcher.setFuture(task);
    if (!(d->m_flags & SuppressCommandLogging))
        addTask(task);
}

void ShellCommand::abort()
{
    d->m_aborted = true;
    d->m_watcher.future().cancel();
}

void ShellCommand::cancel()
{
    emit terminate();
}

void ShellCommand::addTask(QFuture<void> &future)
{
    Q_UNUSED(future)
}

int ShellCommand::timeoutS() const
{
    return std::accumulate(d->m_jobs.cbegin(), d->m_jobs.cend(), 0,
                           [](int sum, const Internal::ShellCommandPrivate::Job &job) {
        return sum + job.timeoutS;
    });
}

FilePath ShellCommand::workDirectory(const FilePath &wd) const
{
    if (!wd.isEmpty())
        return wd;
    return defaultWorkingDirectory();
}

bool ShellCommand::lastExecutionSuccess() const
{
    return d->m_lastExecSuccess;
}

int ShellCommand::lastExecutionExitCode() const
{
    return d->m_lastExecExitCode;
}

void ShellCommand::run(QFutureInterface<void> &future)
{
    // Check that the binary path is not empty
    QTC_ASSERT(!d->m_jobs.isEmpty(), return);

    QString stdOut;
    QString stdErr;

    emit started();
    if (d->m_progressParser)
        d->m_progressParser->setFuture(&future);
    else
        future.setProgressRange(0, 1);
    const int count = d->m_jobs.size();
    d->m_lastExecExitCode = -1;
    d->m_lastExecSuccess = true;
    for (int j = 0; j < count; j++) {
        const Internal::ShellCommandPrivate::Job &job = d->m_jobs.at(j);
        QtcProcess proc;
        proc.setExitCodeInterpreter(job.exitCodeInterpreter);
        proc.setTimeoutS(job.timeoutS);
        runCommand(proc, job.command, job.workingDirectory);
        stdOut += proc.cleanedStdOut();
        stdErr += proc.cleanedStdErr();
        d->m_lastExecExitCode = proc.exitCode();
        d->m_lastExecSuccess = proc.result() == ProcessResult::FinishedWithSuccess;
        if (!d->m_lastExecSuccess)
            break;
    }

    if (!d->m_aborted) {
        if (!d->m_progressiveOutput) {
            emit stdOutText(stdOut);
            if (!stdErr.isEmpty())
                emit stdErrText(stdErr);
        }

        emit finished(d->m_lastExecSuccess, d->m_lastExecExitCode, cookie());
        if (d->m_lastExecSuccess) {
            emit success(cookie());
            future.setProgressValue(future.progressMaximum());
        } else {
            future.cancel(); // sets the progress indicator red
        }
    }

    if (d->m_progressParser)
        d->m_progressParser->setFuture(nullptr);
    // As it is used asynchronously, we need to delete ourselves
    this->deleteLater();
}

void ShellCommand::runCommand(QtcProcess &proc,
                              const CommandLine &command,
                              const FilePath &workingDirectory)
{
    const FilePath dir = workDirectory(workingDirectory);

    if (command.executable().isEmpty()) {
        proc.setResult(ProcessResult::StartFailed);
        return;
    }

    if (!(d->m_flags & SuppressCommandLogging))
        emit appendCommand(dir, command);

    proc.setCommand(command);
    if ((d->m_flags & FullySynchronously)
            || (!(d->m_flags & NoFullySync)
                && QThread::currentThread() == QCoreApplication::instance()->thread())) {
        runFullySynchronous(proc, dir);
    } else {
        runSynchronous(proc, dir);
    }

    if (!d->m_aborted) {
        // Success/Fail message in appropriate window?
        if (proc.result() == ProcessResult::FinishedWithSuccess) {
            if (d->m_flags & ShowSuccessMessage)
                emit appendMessage(proc.exitMessage());
        } else if (!(d->m_flags & SuppressFailMessage)) {
            emit appendError(proc.exitMessage());
        }
    }
}

void ShellCommand::runFullySynchronous(QtcProcess &process, const FilePath &workingDirectory)
{
    // Set up process
    if (d->m_disableUnixTerminal)
        process.setDisableUnixTerminal();
    const FilePath dir = workDirectory(workingDirectory);
    if (!dir.isEmpty())
        process.setWorkingDirectory(dir);
    process.setEnvironment(processEnvironment());
    if (d->m_flags & MergeOutputChannels)
        process.setProcessChannelMode(QProcess::MergedChannels);
    if (d->m_codec)
        process.setCodec(d->m_codec);

    process.runBlocking();

    if (!d->m_aborted) {
        const QString stdErr = process.cleanedStdErr();
        if (!stdErr.isEmpty() && !(d->m_flags & SuppressStdErr))
            emit append(stdErr);

        const QString stdOut = process.cleanedStdOut();
        if (!stdOut.isEmpty() && d->m_flags & ShowStdOut) {
            if (d->m_flags & SilentOutput)
                emit appendSilently(stdOut);
            else
                emit append(stdOut);
        }
    }
}

void ShellCommand::runSynchronous(QtcProcess &process, const FilePath &workingDirectory)
{
    connect(this, &ShellCommand::terminate, &process, [&process] {
        process.stop();
        process.waitForFinished();
    });
    process.setEnvironment(processEnvironment());
    if (d->m_codec)
        process.setCodec(d->m_codec);
    if (d->m_disableUnixTerminal)
        process.setDisableUnixTerminal();
    const FilePath dir = workDirectory(workingDirectory);
    if (!dir.isEmpty())
        process.setWorkingDirectory(dir);
    // connect stderr to the output window if desired
    if (d->m_flags & MergeOutputChannels) {
        process.setProcessChannelMode(QProcess::MergedChannels);
    } else if (d->m_progressiveOutput || !(d->m_flags & SuppressStdErr)) {
        process.setStdErrCallback([this](const QString &text) {
            if (d->m_progressParser)
                d->m_progressParser->parseProgress(text);
            if (!(d->m_flags & SuppressStdErr))
                emit appendError(text);
            if (d->m_progressiveOutput)
                emit stdErrText(text);
        });
    }

    // connect stdout to the output window if desired
    if (d->m_progressParser || d->m_progressiveOutput || (d->m_flags & ShowStdOut)) {
        process.setStdOutCallback([this](const QString &text) {
            if (d->m_progressParser)
                d->m_progressParser->parseProgress(text);
            if (d->m_flags & ShowStdOut)
                emit append(text);
            if (d->m_progressiveOutput) {
                emit stdOutText(text);
                d->m_hadOutput = true;
            }
        });
    }

    process.setTimeOutMessageBoxEnabled(true);

    if (d->m_codec)
        process.setCodec(d->m_codec);

    process.runBlocking(EventLoopMode::On);
}

const QVariant &ShellCommand::cookie() const
{
    return d->m_cookie;
}

void ShellCommand::setCookie(const QVariant &cookie)
{
    d->m_cookie = cookie;
}

QTextCodec *ShellCommand::codec() const
{
    return d->m_codec;
}

void ShellCommand::setCodec(QTextCodec *codec)
{
    d->m_codec = codec;
}

//! Use \a parser to parse progress data from stdout. Command takes ownership of \a parser
void ShellCommand::setProgressParser(ProgressParser *parser)
{
    QTC_ASSERT(!d->m_progressParser, return);
    d->m_progressParser = parser;
}

bool ShellCommand::hasProgressParser() const
{
    return d->m_progressParser;
}

void ShellCommand::setProgressiveOutput(bool progressive)
{
    d->m_progressiveOutput = progressive;
}

void ShellCommand::setDisableUnixTerminal()
{
    d->m_disableUnixTerminal = true;
}

ProgressParser::ProgressParser() :
    m_futureMutex(new QMutex)
{ }

ProgressParser::~ProgressParser()
{
    delete m_futureMutex;
}

void ProgressParser::setProgressAndMaximum(int value, int maximum)
{
    QMutexLocker lock(m_futureMutex);
    if (!m_future)
        return;
    m_future->setProgressRange(0, maximum);
    m_future->setProgressValue(value);
}

void ProgressParser::setFuture(QFutureInterface<void> *future)
{
    QMutexLocker lock(m_futureMutex);
    m_future = future;
}

} // namespace Utils
