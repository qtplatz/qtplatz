// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "../tools/qtcreatorcrashhandler/crashhandlersetup.h"

#include <app/app_version.h>
#include <extensionsystem/iplugin.h>
#include <extensionsystem/pluginerroroverview.h>
#include <extensionsystem/pluginmanager.h>
#include <extensionsystem/pluginspec.h>
#include <qtsingleapplication.h>

#include <utils/algorithm.h>
#include <utils/environment.h>
#include <utils/fileutils.h>
#include <utils/fsengine/fsengine.h>
#include <utils/hostosinfo.h>
#include <utils/qtcsettings.h>
#include <utils/singleton.h>
#include <utils/temporarydirectory.h>
#include <utils/terminalcommand.h>

#include <QDebug>
#include <QDir>
#include <QFontDatabase>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QScopeGuard>
#include <QStyle>
#include <QTextStream>
#include <QThreadPool>
#include <QTimer>
#include <QTranslator>
#include <QUrl>
#include <QVariant>

#include <QSysInfo>

#include <QNetworkProxyFactory>

#include <QApplication>
#include <QMessageBox>
#include <QPixmapCache>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTextCodec>

#include <iterator>
#include <optional>
#include <string>
#include <vector>

#ifdef ENABLE_QT_BREAKPAD
#include <qtsystemexceptionhandler.h>
#endif

#ifdef ENABLE_CRASHPAD
#define NOMINMAX
#include "client/crashpad_client.h"
#include "client/crash_report_database.h"
#include "client/settings.h"
#endif

using namespace ExtensionSystem;

enum { OptionIndent = 4, DescriptionIndent = 34 };

const char corePluginNameC[] = "Core";
const char fixedOptionsC[]
    = " [OPTION]... [FILE]...\n"
      "Options:\n"
      "    -help                         Display this help\n"
      "    -version                      Display program version\n"
      "    -client                       Attempt to connect to already running first instance\n"
      "    -settingspath <path>          Override the default path where user settings are stored\n"
      "    -installsettingspath <path>   Override the default path from where user-independent "
      "settings are read\n"
      "    -temporarycleansettings, -tcs Use clean settings for debug or testing reasons\n"
      "    -pid <pid>                    Attempt to connect to instance given by pid\n"
      "    -block                        Block until editor is closed\n"
      "    -pluginpath <path>            Add a custom search path for plugins\n"
      "    -language <locale>            Set the UI language\n";

const char HELP_OPTION1[] = "-h";
const char HELP_OPTION2[] = "-help";
const char HELP_OPTION3[] = "/h";
const char HELP_OPTION4[] = "--help";
const char VERSION_OPTION[] = "-version";
const char VERSION_OPTION2[] = "--version";
const char CLIENT_OPTION[] = "-client";
const char SETTINGS_OPTION[] = "-settingspath";
const char INSTALL_SETTINGS_OPTION[] = "-installsettingspath";
const char TEST_OPTION[] = "-test";
const char TEMPORARY_CLEAN_SETTINGS1[] = "-temporarycleansettings";
const char TEMPORARY_CLEAN_SETTINGS2[] = "-tcs";
const char PID_OPTION[] = "-pid";
const char BLOCK_OPTION[] = "-block";
const char PLUGINPATH_OPTION[] = "-pluginpath";
const char LANGUAGE_OPTION[] = "-language";
const char USER_LIBRARY_PATH_OPTION[] = "-user-library-path"; // hidden option for qtcreator.sh

using PluginSpecSet = QVector<PluginSpec *>;

// Helpers for displaying messages. Note that there is no console on Windows.

// Format as <pre> HTML
static inline QString toHtml(const QString &t)
{
    QString res = t;
    res.replace(QLatin1Char('&'), QLatin1String("&amp;"));
    res.replace(QLatin1Char('<'), QLatin1String("&lt;"));
    res.replace(QLatin1Char('>'), QLatin1String("&gt;"));
    res.insert(0, QLatin1String("<html><pre>"));
    res.append(QLatin1String("</pre></html>"));
    return res;
}

static void displayHelpText(const QString &t)
{
    if (Utils::HostOsInfo::isWindowsHost() && qApp)
        QMessageBox::information(nullptr, QLatin1String(Core::Constants::IDE_DISPLAY_NAME), toHtml(t));
    else
        printf("%s", qPrintable(t));
}

static void displayError(const QString &t)
{
    if (Utils::HostOsInfo::isWindowsHost() && qApp)
        QMessageBox::critical(nullptr, QLatin1String(Core::Constants::IDE_DISPLAY_NAME), t);
    else
        qCritical("%s", qPrintable(t));
}

static void printVersion(const PluginSpec *coreplugin)
{
    QString version;
    QTextStream str(&version);
    str << '\n' << Core::Constants::IDE_DISPLAY_NAME << ' ' << coreplugin->version()<< " based on Qt " << qVersion() << "\n\n";
    PluginManager::formatPluginVersions(str);
    str << '\n' << coreplugin->copyright() << '\n';
    displayHelpText(version);
}

static void printHelp(const QString &a0)
{
    QString help;
    QTextStream str(&help);
    str << "Usage: " << a0 << fixedOptionsC;
    PluginManager::formatOptions(str, OptionIndent, DescriptionIndent);
    PluginManager::formatPluginOptions(str, OptionIndent, DescriptionIndent);
    displayHelpText(help);
}

QString applicationDirPath(char *arg = nullptr)
{
    static QString dir;

    if (arg)
        dir = QFileInfo(QString::fromLocal8Bit(arg)).dir().absolutePath();

    if (QCoreApplication::instance())
        return QApplication::applicationDirPath();

    return dir;
}

static QString resourcePath()
{
    return QDir::cleanPath(applicationDirPath() + '/' + RELATIVE_DATA_PATH);
}

static inline QString msgCoreLoadFailure(const QString &why)
{
    return QCoreApplication::translate("Application", "Failed to load core: %1").arg(why);
}

static inline int askMsgSendFailed()
{
    return QMessageBox::question(nullptr, QApplication::translate("Application","Could not send message"),
                QCoreApplication::translate("Application", "Unable to send command line arguments "
                                            "to the already running instance. It does not appear to "
                                            "be responding. Do you want to start a new instance of "
                                            "%1?").arg(Core::Constants::IDE_DISPLAY_NAME),
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Retry,
                QMessageBox::Retry);
}

static inline QStringList getPluginPaths()
{
    QStringList rc(QDir::cleanPath(QApplication::applicationDirPath()
                                   + '/' + RELATIVE_PLUGIN_PATH));
    // Local plugin path: <localappdata>/plugins/<ideversion>
    //    where <localappdata> is e.g.
    //    "%LOCALAPPDATA%\QtProject\qtcreator" on Windows Vista and later
    //    "$XDG_DATA_HOME/data/QtProject/qtcreator" or "~/.local/share/data/QtProject/qtcreator" on Linux
    //    "~/Library/Application Support/QtProject/Qt Creator" on Mac
    QString pluginPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    if (Utils::HostOsInfo::isAnyUnixHost() && !Utils::HostOsInfo::isMacHost())
        pluginPath += QLatin1String("/data");
    pluginPath += QLatin1Char('/')
            + QLatin1String(Core::Constants::IDE_SETTINGSVARIANT_STR)
            + QLatin1Char('/');
    pluginPath += QLatin1String(Utils::HostOsInfo::isMacHost() ?
                                    Core::Constants::IDE_DISPLAY_NAME :
                                    Core::Constants::IDE_ID);
    pluginPath += QLatin1String("/plugins/");
    // Qt Creator X.Y.Z can load plugins from X.Y.(Z-1) etc, so add current and previous
    // patch versions
    const QString minorVersion = QString::number(IDE_VERSION_MAJOR) + '.'
                                 + QString::number(IDE_VERSION_MINOR) + '.';
    const int minPatchVersion
        = qMin(IDE_VERSION_RELEASE,
               QVersionNumber::fromString(Core::Constants::IDE_VERSION_COMPAT).microVersion());
    for (int patchVersion = IDE_VERSION_RELEASE; patchVersion >= minPatchVersion; --patchVersion)
        rc.push_back(pluginPath + minorVersion + QString::number(patchVersion));
    return rc;
}

static void setupInstallSettings(QString &installSettingspath)
{
    if (!installSettingspath.isEmpty() && !QFileInfo(installSettingspath).isDir()) {
        displayError(QString("-installsettingspath \"%0\" needs to be the path where a %1/%2.ini exist.").arg(installSettingspath,
            QLatin1String(Core::Constants::IDE_SETTINGSVARIANT_STR), QLatin1String(Core::Constants::IDE_CASED_ID)));
        installSettingspath.clear();
    }
    static const char kInstallSettingsKey[] = "Settings/InstallSettings";
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope,
        installSettingspath.isEmpty() ? resourcePath() : installSettingspath);

    // Check if the default install settings contain a setting for the actual install settings.
    // This can be an absolute path, or a path relative to applicationDirPath().
    // The result is interpreted like -settingspath, but for SystemScope.
    //
    // Through the sdktool split that is upcoming, the new install settings might redirect
    // yet a second time. So try this a few times.
    // (Only the first time with QSettings::UserScope, to allow setting the install settings path
    // in the user settings.)
    QSettings::Scope scope = QSettings::UserScope;
    int count = 0;
    bool containsInstallSettingsKey = false;
    do {
        QSettings installSettings(QSettings::IniFormat, scope,
                                  QLatin1String(Core::Constants::IDE_SETTINGSVARIANT_STR),
                                  QLatin1String(Core::Constants::IDE_CASED_ID));
        containsInstallSettingsKey = installSettings.contains(kInstallSettingsKey);
        if (containsInstallSettingsKey) {
            QString newInstallSettingsPath = installSettings.value(kInstallSettingsKey).toString();
            if (QDir::isRelativePath(newInstallSettingsPath))
                newInstallSettingsPath = applicationDirPath() + '/' + newInstallSettingsPath;
            QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, newInstallSettingsPath);
        }
        scope = QSettings::SystemScope; // UserScope only the first time we check
        ++count;
    } while (containsInstallSettingsKey && count < 3);
}

static Utils::QtcSettings *createUserSettings()
{
    return new Utils::QtcSettings(QSettings::IniFormat,
                                  QSettings::UserScope,
                                  QLatin1String(Core::Constants::IDE_SETTINGSVARIANT_STR),
                                  QLatin1String(Core::Constants::IDE_CASED_ID));
}

static void setHighDpiEnvironmentVariable()
{

    if (Utils::HostOsInfo::isMacHost())
        return;

    std::unique_ptr<QSettings> settings(createUserSettings());

    const bool defaultValue = Utils::HostOsInfo::isWindowsHost();
    const bool enableHighDpiScaling = settings->value("Core/EnableHighDpiScaling", defaultValue).toBool();

    static const char ENV_VAR_QT_DEVICE_PIXEL_RATIO[] = "QT_DEVICE_PIXEL_RATIO";
    if (enableHighDpiScaling
            && !qEnvironmentVariableIsSet(ENV_VAR_QT_DEVICE_PIXEL_RATIO) // legacy in 5.6, but still functional
            && !qEnvironmentVariableIsSet("QT_AUTO_SCREEN_SCALE_FACTOR")
            && !qEnvironmentVariableIsSet("QT_SCALE_FACTOR")
            && !qEnvironmentVariableIsSet("QT_SCREEN_SCALE_FACTORS")) {
    } else {
        QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
            Qt::HighDpiScaleFactorRoundingPolicy::Floor);
    }
}

void setPixmapCacheLimit()
{
    const int originalLimit = QPixmapCache::cacheLimit();
    const qreal dpr = qApp->devicePixelRatio();
    const qreal multiplier = std::clamp(dpr * dpr, 1.0, 4.0);
    QPixmapCache::setCacheLimit(originalLimit * multiplier);
}

void loadFonts()
{
    const QDir dir(resourcePath() + "/fonts/");

    const QFileInfoList fonts = dir.entryInfoList(QStringList("*.ttf"), QDir::Files);
    for (const QFileInfo &fileInfo : fonts)
        QFontDatabase::addApplicationFont(fileInfo.absoluteFilePath());
}

struct Options
{
    QString settingsPath;
    QString installSettingsPath;
    QStringList customPluginPaths;
    QString uiLanguage;
    // list of arguments that were handled and not passed to the application or plugin manager
    QStringList preAppArguments;
    // list of arguments to be passed to the application or plugin manager
    std::vector<char *> appArguments;
    std::optional<QString> userLibraryPath;
    bool hasTestOption = false;
    bool wantsCleanSettings = false;
};

Options parseCommandLine(int argc, char *argv[])
{
    Options options;
    auto it = argv;
    const auto end = argv + argc;
    while (it != end) {
        const auto arg = QString::fromLocal8Bit(*it);
        const bool hasNext = it + 1 != end;
        const auto nextArg = hasNext ? QString::fromLocal8Bit(*(it + 1)) : QString();

        if (arg == SETTINGS_OPTION && hasNext) {
            ++it;
            options.settingsPath = QDir::fromNativeSeparators(nextArg);
            options.preAppArguments << arg << nextArg;
        } else if (arg == INSTALL_SETTINGS_OPTION && hasNext) {
            ++it;
            options.installSettingsPath = QDir::fromNativeSeparators(nextArg);
            options.preAppArguments << arg << nextArg;
        } else if (arg == PLUGINPATH_OPTION && hasNext) {
            ++it;
            options.customPluginPaths += QDir::fromNativeSeparators(nextArg);
            options.preAppArguments << arg << nextArg;
        } else if (arg == LANGUAGE_OPTION && hasNext) {
            ++it;
            options.uiLanguage = nextArg;
            options.preAppArguments << arg << nextArg;
        } else if (arg == USER_LIBRARY_PATH_OPTION && hasNext) {
            ++it;
            options.userLibraryPath = nextArg;
            options.preAppArguments << arg << nextArg;
        } else if (arg == TEMPORARY_CLEAN_SETTINGS1 || arg == TEMPORARY_CLEAN_SETTINGS2) {
            options.wantsCleanSettings = true;
            options.preAppArguments << arg;
        } else { // arguments that are still passed on to the application
            if (arg == TEST_OPTION)
                options.hasTestOption = true;
            options.appArguments.push_back(*it);
        }
        ++it;
    }
    return options;
}

class Restarter
{
public:
    Restarter(int argc, char *argv[])
    {
        Q_UNUSED(argc)
        m_executable = QString::fromLocal8Bit(argv[0]);
        m_workingPath = QDir::currentPath();
    }

    void setArguments(const QStringList &args) { m_args = args; }

    QString executable() const { return m_executable; }
    QStringList arguments() const { return m_args; }
    QString workingPath() const { return m_workingPath; }

    int restartOrExit(int exitCode)
    {
        return qApp->property("restart").toBool() ? restart(exitCode) : exitCode;
    }

    int restart(int exitCode)
    {
        QProcess::startDetached(m_executable, m_args, m_workingPath);
        return exitCode;
    }

private:
    QString m_executable;
    QStringList m_args;
    QString m_workingPath;
};

QStringList lastSessionArgument()
{
    // using insider information here is not particularly beautiful, anyhow
    const bool hasProjectExplorer = Utils::anyOf(PluginManager::plugins(),
                                                 Utils::equal(&PluginSpec::name,
                                                              QString("ProjectExplorer")));
    return hasProjectExplorer ? QStringList({"-lastsession"}) : QStringList();
}

#ifdef ENABLE_CRASHPAD
bool startCrashpad(const QString &libexecPath, bool crashReportingEnabled)
{
    using namespace crashpad;

    // Cache directory that will store crashpad information and minidumps
    QString databasePath = QDir::cleanPath(libexecPath + "/crashpad_reports");
    QString handlerPath = QDir::cleanPath(libexecPath + "/crashpad_handler");
#ifdef Q_OS_WIN
    handlerPath += ".exe";
    base::FilePath database(databasePath.toStdWString());
    base::FilePath handler(handlerPath.toStdWString());
#elif defined(Q_OS_MACOS) || defined(Q_OS_LINUX)
    base::FilePath database(databasePath.toStdString());
    base::FilePath handler(handlerPath.toStdString());
#endif

    std::unique_ptr<CrashReportDatabase> db = CrashReportDatabase::Initialize(database);
    if (db && db->GetSettings())
        db->GetSettings()->SetUploadsEnabled(crashReportingEnabled);

    // URL used to submit minidumps to
    std::string url(CRASHPAD_BACKEND_URL);

    // Optional annotations passed via --annotations to the handler
    std::map<std::string, std::string> annotations;
    annotations["app-version"] = Core::Constants::IDE_VERSION_DISPLAY;
    annotations["qt-version"] = QT_VERSION_STR;

    // Optional arguments to pass to the handler
    std::vector<std::string> arguments;
    arguments.push_back("--no-rate-limit");

    CrashpadClient *client = new CrashpadClient();
    bool success = client->StartHandler(
        handler,
        database,
        database,
        url,
        annotations,
        arguments,
        /* restartable */ true,
        /* asynchronous_start */ true
    );

    return success;
}
#endif

int main(int argc, char **argv)
{
    Restarter restarter(argc, argv);
    Utils::Environment::systemEnvironment(); // cache system environment before we do any changes

    Utils::FSEngine fileSystemEngine;

    // Manually determine various command line options
    // We can't use the regular way of the plugin manager,
    // because settings can change the way plugin manager behaves
    Options options = parseCommandLine(argc, argv);
    applicationDirPath(argv[0]);

    if (qEnvironmentVariableIsSet("QTC_DO_NOT_PROPAGATE_LD_PRELOAD")) {
        Utils::Environment::modifySystemEnvironment(
            {{"LD_PRELOAD", "", Utils::EnvironmentItem::Unset}});
    }

    if (options.userLibraryPath) {
        if ((*options.userLibraryPath).isEmpty()) {
            Utils::Environment::modifySystemEnvironment(
                {{"LD_LIBRARY_PATH", "", Utils::EnvironmentItem::Unset}});
        } else {
            Utils::Environment::modifySystemEnvironment(
                {{"LD_LIBRARY_PATH", *options.userLibraryPath, Utils::EnvironmentItem::SetEnabled}});
        }
    }

    qputenv("QSG_RHI_BACKEND", "opengl");
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
                Qt::HighDpiScaleFactorRoundingPolicy::Round);

    if (qEnvironmentVariableIsSet("QTCREATOR_DISABLE_NATIVE_MENUBAR")
            || qgetenv("XDG_CURRENT_DESKTOP").startsWith("Unity")) {
        QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
    }

    if (Utils::HostOsInfo::isRunningUnderRosetta()) {
        // work around QTBUG-97085: QRegularExpression jitting is not reentrant under Rosetta
        qputenv("QT_ENABLE_REGEXP_JIT", "0");
    }

    if (Utils::HostOsInfo::isLinuxHost() && !qEnvironmentVariableIsSet("GTK_THEME"))
        // Work around QTCREATORBUG-28497:
        // Prevent Qt's GTK3 platform theme plugin from enforcing a dark palette
        qputenv("GTK_THEME", ":light");

#if defined(QTC_FORCE_XCB)
    if (Utils::HostOsInfo::isLinuxHost() && !qEnvironmentVariableIsSet("QT_QPA_PLATFORM")) {
        // Enforce XCB on Linux/Gnome, if the user didn't override via QT_QPA_PLATFORM
        // This was previously done in Qt, but removed in Qt 6.3. We found that bad things can still happen,
        // like the Wayland session simply crashing when starting Qt Creator.
        // TODO: Reconsider when Qt/Wayland is reliably working on the supported distributions
        const bool hasWaylandDisplay = qEnvironmentVariableIsSet("WAYLAND_DISPLAY");
        const bool isWaylandSessionType = qgetenv("XDG_SESSION_TYPE") == "wayland";
        const QByteArray currentDesktop = qgetenv("XDG_CURRENT_DESKTOP").toLower();
        const QByteArray sessionDesktop = qgetenv("XDG_SESSION_DESKTOP").toLower();
        const bool isGnome = currentDesktop.contains("gnome") || sessionDesktop.contains("gnome");
        const bool isWayland = hasWaylandDisplay || isWaylandSessionType;
        if (isGnome && isWayland) {
            qInfo() << "Warning: Ignoring WAYLAND_DISPLAY on Gnome."
                    << "Use QT_QPA_PLATFORM=wayland to run on Wayland anyway.";
            qputenv("QT_QPA_PLATFORM", "xcb");
        }
    }
#endif

    Utils::TemporaryDirectory::setMasterTemporaryDirectory(QDir::tempPath() + "/" + Core::Constants::IDE_CASED_ID + "-XXXXXX");

#ifdef Q_OS_MACOS
    // increase the number of file that can be opened in Qt Creator.
    struct rlimit rl;
    getrlimit(RLIMIT_NOFILE, &rl);

    rl.rlim_cur = qMin((rlim_t)OPEN_MAX, rl.rlim_max);
    setrlimit(RLIMIT_NOFILE, &rl);
#endif

    QScopedPointer<Utils::TemporaryDirectory> temporaryCleanSettingsDir;
    if (options.settingsPath.isEmpty() && (options.hasTestOption || options.wantsCleanSettings)) {
        temporaryCleanSettingsDir.reset(new Utils::TemporaryDirectory("qtc-test-settings"));
        if (!temporaryCleanSettingsDir->isValid())
            return 1;
        options.settingsPath = temporaryCleanSettingsDir->path().path();
    }
    if (!options.settingsPath.isEmpty())
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, options.settingsPath);

    // Must be done before any QSettings class is created
    QSettings::setDefaultFormat(QSettings::IniFormat);
    setupInstallSettings(options.installSettingsPath);
    // plugin manager takes control of this settings object

    setHighDpiEnvironmentVariable();

    SharedTools::QtSingleApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    int numberofArguments = static_cast<int>(options.appArguments.size());

    SharedTools::QtSingleApplication app((QLatin1String(Core::Constants::IDE_DISPLAY_NAME)),
                                         numberofArguments,
                                         options.appArguments.data());
    QCoreApplication::setApplicationName(Core::Constants::IDE_CASED_ID);
    QCoreApplication::setApplicationVersion(QLatin1String(Core::Constants::IDE_VERSION_LONG));
    QCoreApplication::setOrganizationName(QLatin1String(Core::Constants::IDE_SETTINGSVARIANT_STR));
    QGuiApplication::setApplicationDisplayName(Core::Constants::IDE_DISPLAY_NAME);

    auto cleanup = qScopeGuard([] { Utils::Singleton::deleteAll(); });

    const QStringList pluginArguments = app.arguments();

    /*Initialize global settings and resetup install settings with QApplication::applicationDirPath */
    setupInstallSettings(options.installSettingsPath);
    Utils::QtcSettings *settings = createUserSettings();
    Utils::QtcSettings *globalSettings
        = new Utils::QtcSettings(QSettings::IniFormat,
                                 QSettings::SystemScope,
                                 QLatin1String(Core::Constants::IDE_SETTINGSVARIANT_STR),
                                 QLatin1String(Core::Constants::IDE_CASED_ID));
    Utils::TerminalCommand::setSettings(settings);
    setPixmapCacheLimit();
    loadFonts();

    if (Utils::HostOsInfo::isWindowsHost()
            && !qFuzzyCompare(qApp->devicePixelRatio(), 1.0)
            && QApplication::style()->objectName().startsWith(
                QLatin1String("windows"), Qt::CaseInsensitive)) {
        QApplication::setStyle(QLatin1String("fusion"));
    }
    const int threadCount = QThreadPool::globalInstance()->maxThreadCount();
    QThreadPool::globalInstance()->setMaxThreadCount(qMax(4, 2 * threadCount));

    const QString libexecPath = QCoreApplication::applicationDirPath()
            + '/' + RELATIVE_LIBEXEC_PATH;
#ifdef ENABLE_QT_BREAKPAD
    QtSystemExceptionHandler systemExceptionHandler(libexecPath);
#else
    // Display a backtrace once a serious signal is delivered (Linux only).
    CrashHandlerSetup setupCrashHandler(Core::Constants::IDE_DISPLAY_NAME,
                                        CrashHandlerSetup::EnableRestart, libexecPath);
#endif

#ifdef ENABLE_CRASHPAD
    bool crashReportingEnabled = settings->value("CrashReportingEnabled", false).toBool();
    startCrashpad(libexecPath, crashReportingEnabled);
#endif

    PluginManager pluginManager;
    PluginManager::setPluginIID(QLatin1String("org.qt-project.Qt.QtCreatorPlugin"));
    PluginManager::setGlobalSettings(globalSettings);
    PluginManager::setSettings(settings);

    QTranslator translator;
    QTranslator qtTranslator;
    QStringList uiLanguages = QLocale::system().uiLanguages();
    QString overrideLanguage = settings->value(QLatin1String("General/OverrideLanguage")).toString();
    if (!overrideLanguage.isEmpty())
        uiLanguages.prepend(overrideLanguage);
    if (!options.uiLanguage.isEmpty())
        uiLanguages.prepend(options.uiLanguage);
    const QString &creatorTrPath = resourcePath() + "/translations";
    for (QString locale : std::as_const(uiLanguages)) {
        locale = QLocale(locale).name();
        if (translator.load("qtcreator_" + locale, creatorTrPath)) {
            const QString &qtTrPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
            const QString &qtTrFile = QLatin1String("qt_") + locale;
            // Binary installer puts Qt tr files into creatorTrPath
            if (qtTranslator.load(qtTrFile, qtTrPath) || qtTranslator.load(qtTrFile, creatorTrPath)) {
                app.installTranslator(&translator);
                app.installTranslator(&qtTranslator);
                app.setProperty("qtc_locale", locale);
                break;
            }
            Q_UNUSED(translator.load(QString())); // unload()
        } else if (locale == QLatin1String("C") /* overrideLanguage == "English" */) {
            // use built-in
            break;
        } else if (locale.startsWith(QLatin1String("en")) /* "English" is built-in */) {
            // use built-in
            break;
        }
    }

    QByteArray overrideCodecForLocale = settings->value("General/OverrideCodecForLocale").toByteArray();
    if (!overrideCodecForLocale.isEmpty())
        QTextCodec::setCodecForLocale(QTextCodec::codecForName(overrideCodecForLocale));

    app.setDesktopFileName("org.qt-project.qtcreator.desktop");

    // Make sure we honor the system's proxy settings
    QNetworkProxyFactory::setUseSystemConfiguration(true);

    // Load
    const QStringList pluginPaths = getPluginPaths() + options.customPluginPaths;
    PluginManager::setPluginPaths(pluginPaths);
    QMap<QString, QString> foundAppOptions;
    if (pluginArguments.size() > 1) {
        QMap<QString, bool> appOptions;
        appOptions.insert(QLatin1String(HELP_OPTION1), false);
        appOptions.insert(QLatin1String(HELP_OPTION2), false);
        appOptions.insert(QLatin1String(HELP_OPTION3), false);
        appOptions.insert(QLatin1String(HELP_OPTION4), false);
        appOptions.insert(QLatin1String(VERSION_OPTION), false);
        appOptions.insert(QLatin1String(VERSION_OPTION2), false);
        appOptions.insert(QLatin1String(CLIENT_OPTION), false);
        appOptions.insert(QLatin1String(PID_OPTION), true);
        appOptions.insert(QLatin1String(BLOCK_OPTION), false);
        QString errorMessage;
        if (!PluginManager::parseOptions(pluginArguments, appOptions, &foundAppOptions, &errorMessage)) {
            displayError(errorMessage);
            printHelp(QFileInfo(app.applicationFilePath()).baseName());
            return -1;
        }
    }
    restarter.setArguments(options.preAppArguments + PluginManager::argumentsForRestart()
                           + lastSessionArgument());
    // if settingspath is not provided we need to pass on the settings in use
    const QString settingspath = options.preAppArguments.contains(QLatin1String(SETTINGS_OPTION))
            ? QString() : options.settingsPath;
    const PluginManager::ProcessData processData = { restarter.executable(),
            options.preAppArguments + PluginManager::argumentsForRestart(), restarter.workingPath(),
            settingspath};
    PluginManager::setCreatorProcessData(processData);

    const PluginSpecSet plugins = PluginManager::plugins();
    PluginSpec *coreplugin = nullptr;
    for (PluginSpec *spec : plugins) {
        if (spec->name() == QLatin1String(corePluginNameC)) {
            coreplugin = spec;
            break;
        }
    }
    if (!coreplugin) {
        QString nativePaths = QDir::toNativeSeparators(pluginPaths.join(QLatin1Char(',')));
        const QString reason = QCoreApplication::translate("Application", "Could not find Core plugin in %1").arg(nativePaths);
        displayError(msgCoreLoadFailure(reason));
        return 1;
    }
    if (!coreplugin->isEffectivelyEnabled()) {
        const QString reason = QCoreApplication::translate("Application", "Core plugin is disabled.");
        displayError(msgCoreLoadFailure(reason));
        return 1;
    }
    if (coreplugin->hasError()) {
        displayError(msgCoreLoadFailure(coreplugin->errorString()));
        return 1;
    }
    if (foundAppOptions.contains(QLatin1String(VERSION_OPTION))
            || foundAppOptions.contains(QLatin1String(VERSION_OPTION2))) {
        printVersion(coreplugin);
        return 0;
    }
    if (foundAppOptions.contains(QLatin1String(HELP_OPTION1))
            || foundAppOptions.contains(QLatin1String(HELP_OPTION2))
            || foundAppOptions.contains(QLatin1String(HELP_OPTION3))
            || foundAppOptions.contains(QLatin1String(HELP_OPTION4))) {
        printHelp(QFileInfo(app.applicationFilePath()).baseName());
        return 0;
    }

    qint64 pid = -1;
    if (foundAppOptions.contains(QLatin1String(PID_OPTION))) {
        QString pidString = foundAppOptions.value(QLatin1String(PID_OPTION));
        bool pidOk;
        qint64 tmpPid = pidString.toInt(&pidOk);
        if (pidOk)
            pid = tmpPid;
    }

    bool isBlock = foundAppOptions.contains(QLatin1String(BLOCK_OPTION));
    if (app.isRunning() && (pid != -1 || isBlock
                            || foundAppOptions.contains(QLatin1String(CLIENT_OPTION)))) {
        app.setBlock(isBlock);
        if (app.sendMessage(PluginManager::serializedArguments(), 5000 /*timeout*/, pid))
            return 0;

        // Message could not be send, maybe it was in the process of quitting
        if (app.isRunning(pid)) {
            // Nah app is still running, ask the user
            int button = askMsgSendFailed();
            while (button == QMessageBox::Retry) {
                if (app.sendMessage(PluginManager::serializedArguments(), 5000 /*timeout*/, pid))
                    return 0;
                if (!app.isRunning(pid)) // App quit while we were trying so start a new creator
                    button = QMessageBox::Yes;
                else
                    button = askMsgSendFailed();
            }
            if (button == QMessageBox::No)
                return -1;
        }
    }

    PluginManager::checkForProblematicPlugins();
    PluginManager::loadPlugins();
    if (coreplugin->hasError()) {
        displayError(msgCoreLoadFailure(coreplugin->errorString()));
        return 1;
    }

    // Set up remote arguments.
    QObject::connect(&app, &SharedTools::QtSingleApplication::messageReceived,
                     &pluginManager, &PluginManager::remoteArguments);

    QObject::connect(&app, SIGNAL(fileOpenRequest(QString)), coreplugin->plugin(),
                     SLOT(fileOpenRequest(QString)));

    // shutdown plugin manager on the exit
    QObject::connect(&app, &QCoreApplication::aboutToQuit, &pluginManager, &PluginManager::shutdown);

    return restarter.restartOrExit(app.exec());
}