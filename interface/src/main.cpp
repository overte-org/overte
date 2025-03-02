//
//  main.cpp
//  interface/src
//
//  Copyright 2013 High Fidelity, Inc.
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <QCommandLineParser>
#include <QtCore/QProcess>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLocalSocket>
#include <QLocalServer>
#include <QSharedMemory>
#include <QTranslator>
#include <QStandardPaths>

#include <BuildInfo.h>
#include <SandboxUtils.h>
#include <SharedUtil.h>
#include <NetworkAccessManager.h>
#include <gl/GLHelpers.h>
#include <iostream>
#include <plugins/InputPlugin.h>
#include <plugins/PluginManager.h>
#include <plugins/DisplayPlugin.h>
#include <plugins/CodecPlugin.h>

#include "AddressManager.h"
#include "Application.h"
#include "crash-handler/CrashHandler.h"
#include "InterfaceLogging.h"
#include "UserActivityLogger.h"
#include "MainWindow.h"
#include "Profile.h"
#include "LogHandler.h"
#include "RunningMarker.h"
#include <plugins/PluginManager.h>
#include <plugins/DisplayPlugin.h>
#include <plugins/CodecPlugin.h>
#include "PathUtils.h"

#ifdef Q_OS_WIN
#include <Windows.h>
extern "C" {
    typedef int(__stdcall* CHECKMINSPECPROC)();
}
#endif

int main(int argc, const char* argv[]) {
#ifdef Q_OS_MAC
    auto format = getDefaultOpenGLSurfaceFormat();
    // Deal with some weirdness in the chromium context sharing on Mac.
    // The primary share context needs to be 3.2, so that the Chromium will
    // succeed in it's creation of it's command stub contexts.
    format.setVersion(3, 2);
    // This appears to resolve the issues with corrupted fonts on OSX.  No
    // idea why.
    qputenv("QT_ENABLE_GLYPH_CACHE_WORKAROUND", "true");
    // https://i.kym-cdn.com/entries/icons/original/000/008/342/ihave.jpg
    QSurfaceFormat::setDefaultFormat(format);
#endif

#if defined(Q_OS_WIN)
    // Check the minimum version of
    if (gl::getAvailableVersion() < gl::getRequiredVersion()) {
        MessageBoxA(nullptr, "Interface requires OpenGL 4.1 or higher", "Unsupported", MB_OK);
        return -1;
    }
#endif

    // Setup QCoreApplication settings, install log message handler
    setupHifiApplication(BuildInfo::INTERFACE_NAME);

    // Journald by default in user applications is probably a bit too modern still.
    LogHandler::getInstance().setShouldUseJournald(false);


    // Extend argv to enable WebGL rendering
    std::vector<const char*> argvExtended(&argv[0], &argv[argc]);
    argvExtended.push_back("--ignore-gpu-blocklist");
#ifdef Q_OS_ANDROID
    argvExtended.push_back("--suppress-settings-reset");
#endif
    int argcExtended = (int)argvExtended.size();

    QElapsedTimer startupTime;
    startupTime.start();

    QCommandLineParser parser;
    parser.setApplicationDescription("Overte -- A free/libre and open-source virtual worlds client");
    QCommandLineOption helpOption = parser.addHelpOption();
    QCommandLineOption versionOption = parser.addVersionOption();

    QCommandLineOption urlOption(
        "url",
        "Start at specified URL location.",
        "string"
    );
    QCommandLineOption protocolVersionOption(
        "protocolVersion",
        "Writes the protocol version base64 signature to a file",
        "path"
    );
    QCommandLineOption noUpdaterOption(
        "no-updater",
        "Do not show auto-updater."
    );
    QCommandLineOption checkMinSpecOption(
        "checkMinSpec",
        "Check if machine meets minimum specifications. The program will run if check passes."
    );
    QCommandLineOption runServerOption(
        "runServer",
        "Run the server."
    );
    QCommandLineOption listenPortOption(
        "listenPort",
        "Port to listen on.",
        "port_number"
    );
    QCommandLineOption serverContentPathOption(
        "serverContentPath",
        "Path to find server content.", // What content??
        "path"
    );
    QCommandLineOption overrideAppLocalDataPathOption(
        "cache",
        "Set test cache.",
        "dir"
    );
    QCommandLineOption scriptsOption(
        "scripts",
        "Set path for defaultScripts. These are probably scripts that run automatically. This parameter does not seem to work.",
        "dir"
    );
    QCommandLineOption allowMultipleInstancesOption(
        "allowMultipleInstances",
        "Allow multiple instances to run."
    );
    QCommandLineOption instanceOption(
        "instance",
        "Name of the instance to run. An instance has its own settings."
        "name"
    );
    QCommandLineOption displaysOption(
        "display",
        "Preferred display.",
        "displays"
    );
    QCommandLineOption disableDisplaysOption(
        "disableDisplayPlugins",
        "Displays to disable. Valid options include \"OpenVR (Vive)\" and \"Oculus Rift\"",
        "string"
    );
    QCommandLineOption disableInputsOption(
        "disableInputPlugins",
        "Inputs to disable. Valid options include \"OpenVR (Vive)\" and \"Oculus Rift\"",
        "string"
    );
    QCommandLineOption suppressSettingsResetOption(
        "suppress-settings-reset",
        "Suppress the prompt to reset interface settings."
    );
    QCommandLineOption oculusStoreOption(
        "oculus-store",
        "Let the Oculus plugin know if interface was run from the Oculus Store."
    );
    QCommandLineOption standaloneOption(
        "standalone",
        "Emulate a standalone device."
    );
    QCommandLineOption disableWatchdogOption(
        "disableWatchdog",
        "Disable the watchdog thread. The interface will crash on deadlocks."
    );
    QCommandLineOption systemCursorOption(
        "system-cursor",
        "Use the default system cursor."
    );
    QCommandLineOption concurrentDownloadsOption(
        "concurrent-downloads",
        "Maximum concurrent resource downloads. Default is 16, except for Android where it is 4.",
        "integer"
    );
    QCommandLineOption avatarURLOption(
        "avatarURL",
        "Override the avatar U.R.L.",
        "url"
    );
    QCommandLineOption replaceAvatarURLOption(
        "replaceAvatarURL",
        "Replaces the avatar U.R.L. When used with --avatarURL, this takes precedence.",
        "url"
    );
    QCommandLineOption setBookmarkOption(
        "setBookmark",
        "Set bookmark as key=value pair. Including the '=' symbol in either string is unsupported.",
        "string"
    );
    QCommandLineOption forceCrashReportingOption(
        "forceCrashReporting",
        "Force crash reporting to initialize."
    );
    // The documented "--disable-lod" does not seem to exist.
    // Below are undocumented.
    QCommandLineOption noLauncherOption(
       "no-launcher",
       "Supposedly does something for the server, unrelated to the application launcher. The feature may never have been implemented."
    );
    QCommandLineOption overrideScriptsPathOption(
       "overrideScriptsPath",
       "Specifies path to default directory where the application will look for scripts to load.",
       "string"
    );
    QCommandLineOption defaultScriptsOverrideOption(
       "defaultScriptsOverride",
       "Override default script to run automatically on start. Default is \"defaultsScripts.js\".",
       "string"
    );
    QCommandLineOption responseTokensOption(
       "tokens",
       "Set response tokens <json>.",
       "json"
    );
    QCommandLineOption displayNameOption(
       "displayName",
       "Set user display name <string>.",
       "string"
    );
    QCommandLineOption noLoginOption(
       "no-login-suggestion",
       "Do not show log-in dialogue."
    );
    QCommandLineOption traceFileOption(
       "traceFile",
       "Writes a trace to a file in the documents folder. Only works if \"--traceDuration\" is specified.",
       "path"
    );
    QCommandLineOption traceDurationOption(
       "traceDuration",
       "Automatically quit interface after duration. Only works if \"--traceFile\" is specified.",
       "seconds"
    );
    QCommandLineOption clockSkewOption(
       "clockSkew",
       "Forces client instance's clock to skew for demonstration purposes.",
       "integer"
    ); // This should probably be removed.
    QCommandLineOption testScriptOption(
       "testScript",
       "Undocumented. Accepts parameter as U.R.L.",
       "string"
    );
    QCommandLineOption testResultsLocationOption(
       "testResultsLocation",
       "Undocumented",
       "path"
    );
    QCommandLineOption quitWhenFinishedOption(
       "quitWhenFinished",
       "Only works if \"--testScript\" is provided."
    ); // Should probably also be made to work on testResultsLocationOption.
    QCommandLineOption fastHeartbeatOption(
       "fast-heartbeat",
       "Change stats polling interval from 10000ms to 1000ms."
    );
    QCommandLineOption logOption(
        "logOptions",
        "Logging options, comma separated: color,nocolor,process_id,thread_id,milliseconds,keep_repeats,journald,nojournald",
        "options"
    );
    QCommandLineOption getPluginsOption(
        "getPlugins",
        "Print out a list of plugins in JSON"
    );
    QCommandLineOption abortAfterStartupOption(
        "abortAfterStartup",
        "Debug option. Aborts right after startup."
    );
    QCommandLineOption abortAfterInitOption(
        "abortAfterInit",
        "Debug option. Aborts after initialization, right before the program starts running the event loop."
    );
    QCommandLineOption getProtocolVersionHashOption(
        "getProtocolVersionHash",
        "Debug option. Returns the network protocol version MD5 hash."
    );
    QCommandLineOption getProtocolVersionDataOption(
        "getProtocolVersionData",
        "Debug option. Returns the network protocol detailed data in JSON."
    );

    // "--qmljsdebugger", which appears in output from "--help-all".
    // Those below don't seem to be optional.
    //     --ignore-gpu-blocklist
    //     --suppress-settings-reset


    parser.addOption(urlOption);
    parser.addOption(protocolVersionOption);
    parser.addOption(noUpdaterOption);
    parser.addOption(checkMinSpecOption);
    parser.addOption(runServerOption);
    parser.addOption(listenPortOption);
    parser.addOption(serverContentPathOption);
    parser.addOption(overrideAppLocalDataPathOption);
    parser.addOption(scriptsOption);
    parser.addOption(allowMultipleInstancesOption);
    parser.addOption(instanceOption);
    parser.addOption(displaysOption);
    parser.addOption(disableDisplaysOption);
    parser.addOption(disableInputsOption);
    parser.addOption(suppressSettingsResetOption);
    parser.addOption(oculusStoreOption);
    parser.addOption(standaloneOption);
    parser.addOption(disableWatchdogOption);
    parser.addOption(systemCursorOption);
    parser.addOption(concurrentDownloadsOption);
    parser.addOption(avatarURLOption);
    parser.addOption(replaceAvatarURLOption);
    parser.addOption(setBookmarkOption);
    parser.addOption(forceCrashReportingOption);
    parser.addOption(noLauncherOption);
    parser.addOption(responseTokensOption);
    parser.addOption(displayNameOption);
    parser.addOption(noLoginOption);
    parser.addOption(overrideScriptsPathOption);
    parser.addOption(defaultScriptsOverrideOption);
    parser.addOption(traceFileOption);
    parser.addOption(traceDurationOption);
    parser.addOption(clockSkewOption);
    parser.addOption(testScriptOption);
    parser.addOption(testResultsLocationOption);
    parser.addOption(quitWhenFinishedOption);
    parser.addOption(fastHeartbeatOption);
    parser.addOption(logOption);
    parser.addOption(abortAfterStartupOption);
    parser.addOption(abortAfterInitOption);
    parser.addOption(getPluginsOption);
    parser.addOption(getProtocolVersionHashOption);
    parser.addOption(getProtocolVersionDataOption);


    QString applicationPath;
    QString instanceName = "main";


    // A temporary application instance is needed to get the location of the running executable
    // Tests using high_resolution_clock show that this takes about 30-50 microseconds (on my machine, YMMV)
    // If we wanted to avoid the QCoreApplication, we would need to write our own
    // cross-platform implementation.
    {
        QCoreApplication tempApp(argc, const_cast<char**>(argv));

        parser.process(QCoreApplication::arguments());  // Must be run after QCoreApplication is initalised.


#ifdef Q_OS_OSX
        if (QFileInfo::exists(QCoreApplication::applicationDirPath() + "/../../../config.json")) {
            applicationPath = QCoreApplication::applicationDirPath() + "/../../../";
        } else {
            applicationPath = QCoreApplication::applicationDirPath();
        }
#else
        applicationPath = QCoreApplication::applicationDirPath();
        qInfo() << "Application path is: " << applicationPath;
#endif


        // PathUtils::initialize wants a fully working QCoreApplication, so we've got to do this
        // in a scope where a QApplication exists. A bit hacky, but should be harmless and saves
        // us a bunch of work.

        if (parser.isSet(instanceOption)) {
            instanceName = parser.value(instanceOption).toUtf8();
        }
        PathUtils::initialize(PathUtils::FilesystemLayout::Auto, PathUtils::DataStorage::Auto, instanceName);
    }

    // TODO: We need settings for Application, but Settings needs an Application
    // to handle events. Needs splitting into two parts: enough initialization
    // for Application to work, and then thread start afterwards.
    Setting::init();
    Application app(argcExtended, const_cast<char**>(argvExtended.data()), parser, startupTime);

    if (parser.isSet("abortAfterStartup")) {
        return 99;
    }

    // We want to configure the logging system as early as possible
    auto& logHandler = LogHandler::getInstance();

    if (parser.isSet(logOption)) {
        if (!logHandler.parseOptions(parser.value(logOption).toUtf8(), logOption.names().first())) {
            QCoreApplication mockApp(argc, const_cast<char**>(argv));  // required for call to showHelp()
            parser.showHelp();
            Q_UNREACHABLE();
        }
    }

    app.initializePluginManager(parser);

    if (parser.isSet(getPluginsOption)) {
        auto pluginManager = PluginManager::getInstance();

        QJsonObject pluginsJson;
        for (const auto &plugin : pluginManager->getPluginInfo()) {
            QJsonObject data;
            data["data"] = plugin.metaData;
            data["loaded"] = plugin.loaded;
            data["disabled"] = plugin.disabled;
            data["filteredOut"] = plugin.filteredOut;
            data["wrongVersion"] = plugin.wrongVersion;
            pluginsJson[plugin.name] = data;
        }

        QJsonObject inputJson;
        for (const auto &plugin : pluginManager->getInputPlugins()) {
            QJsonObject data;
            data["subdeviceNames"] = QJsonArray::fromStringList(plugin->getSubdeviceNames());
            data["deviceName"] = plugin->getDeviceName();
            data["configurable"] = plugin->configurable();
            data["isHandController"] = plugin->isHandController();
            data["isHeadController"] = plugin->isHeadController();
            data["isActive"] = plugin->isActive();
            data["isSupported"] = plugin->isSupported();

            inputJson[plugin->getName()] = data;
        }

        QJsonObject displayJson;
        for (const auto &plugin : pluginManager->getDisplayPlugins()) {
            QJsonObject data;
            data["isHmd"] = plugin->isHmd();
            data["isStereo"] = plugin->isStereo();
            data["targetFramerate"] = plugin->getTargetFrameRate();
            data["hasAsyncReprojection"] = plugin->hasAsyncReprojection();
            data["isActive"] = plugin->isActive();
            data["isSupported"] = plugin->isSupported();

            displayJson[plugin->getName()] = data;
        }

        QJsonObject codecsJson;
        for (const auto &plugin : pluginManager->getCodecPlugins()) {
            QJsonObject data;
            data["isActive"] = plugin->isActive();
            data["isSupported"] = plugin->isSupported();

            codecsJson[plugin->getName()] = data;
        }

        QJsonObject platformsJson;
        platformsJson["steamAvailable"] = (pluginManager->getSteamClientPlugin() != nullptr);
        platformsJson["oculusAvailable"] = (pluginManager->getOculusPlatformPlugin() != nullptr);

        QJsonObject root;
        root["plugins"] = pluginsJson;
        root["inputs"] = inputJson;
        root["displays"] = displayJson;
        root["codecs"] = codecsJson;
        root["platforms"] = platformsJson;

        std::cout << QJsonDocument(root).toJson().toStdString() << "\n";

        return 0;
    }


    // Act on arguments for early termination.
    if (parser.isSet(versionOption)) {
        parser.showVersion();
        Q_UNREACHABLE();
    }
    if (parser.isSet(helpOption)) {
        QCoreApplication mockApp(argc, const_cast<char**>(argv));  // required for call to showHelp()
        parser.showHelp();
        Q_UNREACHABLE();
    }
    if (parser.isSet(protocolVersionOption)) {
        FILE* fp = fopen(parser.value(protocolVersionOption).toStdString().c_str(), "w");
        if (fp) {
            fputs(protocolVersionsSignatureBase64().toStdString().c_str(), fp);
            fclose(fp);
            return 0;
        } else {
            qWarning() << "Failed to open file specified for --protocolVersion.";
            return 1;
        }
    }
    if (parser.isSet(getProtocolVersionHashOption)) {
        std::cout << protocolVersionsSignatureHex().toStdString() << std::endl;
        return 0;
    }
    if (parser.isSet(getProtocolVersionDataOption)) {
        auto protocolMap = protocolVersionsSignatureMap();
        QMetaEnum packetMetaEnum = QMetaEnum::fromType<PacketTypeEnum::Value>();

        QJsonArray packetTypesList;
        auto keyList = protocolMap.keys();
        std::sort(keyList.begin(), keyList.end()); // Sort by numeric value

        for(const auto packet : keyList) {
            QJsonObject data;
            int intValue = static_cast<int>(packet);
            QString keyName = packetMetaEnum.valueToKey(intValue);

            data["name"] = keyName;
            data["value"] = intValue;
            data["version"] = versionForPacketType(packet);

            packetTypesList.append(data);
        }

        std::cout << QJsonDocument(packetTypesList).toJson().toStdString() << std::endl;
        return 0;
    }


    static const QString APPLICATION_CONFIG_FILENAME = "config.json";
    QDir applicationDir(applicationPath);
    QString configFileName = applicationDir.filePath(APPLICATION_CONFIG_FILENAME);
    QFile configFile(configFileName);
    QString launcherPath;
    if (configFile.exists()) {
        if (!configFile.open(QIODevice::ReadOnly)) {
            qWarning() << "Found application config, but could not open it";
        } else {
            auto contents = configFile.readAll();
            QJsonParseError error;

            auto doc = QJsonDocument::fromJson(contents, &error);
            if (error.error) {
                qWarning() << "Found application config, but could not parse it: " << error.errorString();
            } else {
                static const QString LAUNCHER_PATH_KEY = "launcherPath";
                launcherPath = doc.object()[LAUNCHER_PATH_KEY].toString();
                if (!launcherPath.isEmpty()) {
                    if (!parser.isSet(noLauncherOption)) {
                        qDebug() << "Found a launcherPath in application config. Starting launcher.";
                        QProcess launcher;
                        launcher.setProgram(launcherPath);
                        launcher.startDetached();
                        return 0;
                    } else {
                        qDebug() << "Found a launcherPath in application config, but the launcher"
                                    " has been suppressed. Continuing normal execution.";
                    }
                    configFile.close();
                }
            }
        }
    }

    // Early check for --traceFile argument
    auto tracer = DependencyManager::set<tracing::Tracer>();
    const char* traceFile = nullptr;
    float traceDuration = 0.0f;
    if (parser.isSet(traceFileOption)) {
        traceFile = parser.value(traceFileOption).toStdString().c_str();
        if (parser.isSet(traceDurationOption)) {
            traceDuration = parser.value(traceDurationOption).toFloat();
            tracer->startTracing();
        } else {
            qWarning() << "\"--traceDuration\" must be specified along with \"--traceFile\"...";
            return 1;
        }
    }

    PROFILE_SYNC_BEGIN(startup, "main startup", "");

#ifdef Q_OS_LINUX
    QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
#endif

#if defined(USE_GLES) && defined(Q_OS_WIN)
    // When using GLES on Windows, we can't create normal GL context in Qt, so
    // we force Qt to use angle.  This will cause the QML to be unable to be used
    // in the output window, so QML should be disabled.
    qputenv("QT_ANGLE_PLATFORM", "d3d11");
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
#endif

    // Instance UserActivityLogger now that the settings are loaded
    auto& ual = UserActivityLogger::getInstance();
    auto& ch = CrashHandler::getInstance();

    QObject::connect(&ch, &CrashHandler::enabledChanged, [](bool enabled) {
        Settings s;
        s.beginGroup("Crash");
        s.setValue("ReportingEnabled", enabled);
        s.endGroup();
    });

    // once the settings have been loaded, check if we need to flip the default for UserActivityLogger
    if (!ual.isDisabledSettingSet()) {
        // the user activity logger is opt-out for Interface
        // but it's defaulted to disabled for other targets
        // so we need to enable it here if it has never been disabled by the user
        ual.disable(false);
    }
    qDebug() << "UserActivityLogger is enabled:" << ual.isEnabled();

    if (parser.isSet(forceCrashReportingOption)) {
        qInfo() << "Crash reporting enabled on the command-line";
        ch.setEnabled(true);
    }

    {
        Settings crashSettings;
        crashSettings.beginGroup("Crash");
        if (crashSettings.value("ReportingEnabled").toBool()) {
            ch.setEnabled(true);
        }
        crashSettings.endGroup();
    }

    ch.setAnnotation("program", "interface");

    const QString& applicationName = getInterfaceSharedMemoryName();
    bool instanceMightBeRunning = true;
#ifdef Q_OS_WIN
    // Try to create a shared memory block - if it can't be created, there is an instance of
    // interface already running. We only do this on Windows for now because of the potential
    // for crashed instances to leave behind shared memory instances on unix.
    QSharedMemory sharedMemory{ applicationName };
    instanceMightBeRunning = !sharedMemory.create(1, QSharedMemory::ReadOnly);
#endif

    // allow multiple interfaces to run if this environment variable is set.
    bool allowMultipleInstances = parser.isSet(allowMultipleInstancesOption) ||
                                  QProcessEnvironment::systemEnvironment().contains("HIFI_ALLOW_MULTIPLE_INSTANCES");
    if (allowMultipleInstances) {
        instanceMightBeRunning = false;
    }
    // this needs to be done here in main, as the mechanism for setting the
    // scripts directory appears not to work.  See the bug report (dead link)
    // https://highfidelity.fogbugz.com/f/cases/5759/Issues-changing-scripts-directory-in-ScriptsEngine
    if (parser.isSet(overrideScriptsPathOption)) {
        QDir scriptsPath(parser.value(overrideScriptsPathOption));
        if (scriptsPath.exists()) {
            PathUtils::defaultScriptsLocation(scriptsPath.path());
        }
    }

    if (instanceMightBeRunning) {
        // Try to connect and send message to existing interface instance
        QLocalSocket socket;

        socket.connectToServer(applicationName);

        static const int LOCAL_SERVER_TIMEOUT_MS = 500;

        // Try to connect - if we can't connect, interface has probably just gone down
        if (socket.waitForConnected(LOCAL_SERVER_TIMEOUT_MS)) {
            if (parser.isSet(urlOption)) {
                QUrl url = QUrl(parser.value(urlOption));
                if (url.isValid() && (url.scheme() == URL_SCHEME_OVERTE || url.scheme() == URL_SCHEME_OVERTEAPP ||
                                      url.scheme() == HIFI_URL_SCHEME_HTTP || url.scheme() == HIFI_URL_SCHEME_HTTPS ||
                                      url.scheme() == HIFI_URL_SCHEME_FILE)) {
                    qDebug() << "Writing URL to local socket";
                    socket.write(url.toString().toUtf8());
                    if (!socket.waitForBytesWritten(5000)) {
                        qDebug() << "Error writing URL to local socket";
                    }
                }
            }

            socket.close();

            qDebug() << "Interface instance appears to be running, exiting";
            qDebug() << "Start with --allowMultipleInstances to allow running multiple instances at once.";

            return EXIT_SUCCESS;
        }

#ifdef Q_OS_WIN
        return EXIT_SUCCESS;
#endif
    }

    // FIXME this method of checking the OpenGL version screws up the `QOpenGLContext::globalShareContext()` value, which in turn
    // leads to crashes when creating the real OpenGL instance.  Disabling for now until we come up with a better way of checking
    // the GL version on the system without resorting to creating a full Qt application
#if 0
    // Check OpenGL version.
    // This is done separately from the main Application so that start-up and shut-down logic within the main Application is
    // not made more complicated than it already is.
    bool overrideGLCheck = false;

    QJsonObject glData;
    {
        OpenGLVersionChecker openGLVersionChecker(argc, const_cast<char**>(argv));
        bool valid = true;
        glData = openGLVersionChecker.checkVersion(valid, overrideGLCheck);
        if (!valid) {
            if (overrideGLCheck) {
                auto glVersion = glData["version"].toString();
                qCWarning(interfaceapp, "Running on insufficient OpenGL version: %s.", glVersion.toStdString().c_str());
            } else {
                qCWarning(interfaceapp, "Early exit due to OpenGL version.");
                return 0;
            }
        }
    }
#endif

    // Debug option to demonstrate that the client's local time does not
    // need to be in sync with any other network node. This forces clock
    // skew for the individual client
    if (parser.isSet(clockSkewOption)) {
        const char* clockSkewValue = parser.value(clockSkewOption).toStdString().c_str();
        qint64 clockSkew = atoll(clockSkewValue);
        usecTimestampNowForceClockSkew(clockSkew);
        qCDebug(interfaceapp) << "clockSkewOption=" << clockSkewValue << "clockSkew=" << clockSkew;
    }

    // Oculus initialization MUST PRECEDE OpenGL context creation.
    // The nature of the Application constructor means this has to be either here,
    // or in the main window ctor, before GL startup.
    //app.configurePlugins(parser);

#ifdef Q_OS_WIN
    // If we're running in steam mode, we need to do an explicit check to ensure we're up to the required min spec
    if (parser.isSet(checkMinSpecOption)) {
        QString appPath;
        {
            char filename[MAX_PATH];
            GetModuleFileName(NULL, filename, MAX_PATH);
            QFileInfo appInfo(filename);
            appPath = appInfo.absolutePath();
        }
        QString openvrDllPath = appPath + "/plugins/openvr.dll";
        HMODULE openvrDll;
        CHECKMINSPECPROC checkMinSpecPtr;
        if ((openvrDll = LoadLibrary(openvrDllPath.toLocal8Bit().data())) &&
            (checkMinSpecPtr = (CHECKMINSPECPROC)GetProcAddress(openvrDll, "CheckMinSpec"))) {
            if (!checkMinSpecPtr()) {
                return -1;
            }
        }
    }
#endif

    int exitCode;
    {
        RunningMarker runningMarker("Interface.running");
        bool runningMarkerExisted = runningMarker.fileExists();
        qInfo() << "Running marker existed";
        runningMarker.writeRunningMarkerFile();

        bool noUpdater = parser.isSet(noUpdaterOption);
        bool runServer = parser.isSet(runServerOption);
        bool serverContentPathOptionIsSet = parser.isSet(serverContentPathOption);
        QString serverContentPath = serverContentPathOptionIsSet ? parser.value(serverContentPathOption) : QString();
        if (runServer) {
            SandboxUtils::runLocalSandbox(serverContentPath, true, noUpdater);
        }

        PROFILE_SYNC_END(startup, "main startup", "");
        PROFILE_SYNC_BEGIN(startup, "app full ctor", "");
        app.setPreviousSessionCrashed(runningMarkerExisted);
        app.initialize(parser);
        PROFILE_SYNC_END(startup, "app full ctor", "");

#if defined(Q_OS_LINUX)
        app.setWindowIcon(QIcon(PathUtils::resourcesPath() + "images/brand-logo.svg"));
#endif
        ch.startMonitor(&app);


        QTimer exitTimer;
        if (traceDuration > 0.0f) {
            exitTimer.setSingleShot(true);
            QObject::connect(&exitTimer, &QTimer::timeout, &app, &Application::quit);
            exitTimer.start(int(1000 * traceDuration));
        }

#if 0
        // If we failed the OpenGLVersion check, log it.
        if (overrideGLcheck) {
            auto accountManager = DependencyManager::get<AccountManager>();
            if (accountManager->isLoggedIn()) {
                UserActivityLogger::getInstance().insufficientGLVersion(glData);
            } else {
                QObject::connect(accountManager.data(), &AccountManager::loginComplete, [glData](){
                    static bool loggedInsufficientGL = false;
                    if (!loggedInsufficientGL) {
                        UserActivityLogger::getInstance().insufficientGLVersion(glData);
                        loggedInsufficientGL = true;
                    }
                });
            }
        }
#endif

        // Setup local server
        QLocalServer server{ &app };

        // We failed to connect to a local server, so we remove any existing servers.
        server.removeServer(applicationName);
        server.listen(applicationName);

        QObject::connect(&server, &QLocalServer::newConnection,
                         &app, &Application::handleLocalServerConnection, Qt::DirectConnection);

        printSystemInformation();

        auto appPointer = dynamic_cast<Application*>(&app);
        if (appPointer) {
            if (parser.isSet(urlOption)) {
                appPointer->overrideEntry();
            }
            if (parser.isSet(displayNameOption)) {
                QString displayName = QString(parser.value(displayNameOption));
                appPointer->forceDisplayName(displayName);
            }
            if (!launcherPath.isEmpty()) {
                appPointer->setConfigFileURL(configFileName);
            }
            if (parser.isSet(responseTokensOption)) {
                QString tokens = QString(parser.value(responseTokensOption));
                appPointer->forceLoginWithTokens(tokens);
            }
        }

        QTranslator translator;
        translator.load("i18n/interface_en");
        app.installTranslator(&translator);
        qCDebug(interfaceapp, "Created QT Application.");
        if (parser.isSet("abortAfterInit")) {
            return 99;
        }
        exitCode = app.exec();
        server.close();

        if (traceFile != nullptr) {
            tracer->stopTracing();
            tracer->serialize(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + traceFile);
        }
    }

    Application::shutdownPlugins();

    qCDebug(interfaceapp, "Normal exit.");
#if !defined(DEBUG) && !defined(Q_OS_LINUX)
    // HACK: exit immediately (don't handle shutdown callbacks) for Release build
    _exit(exitCode);
#endif
    return exitCode;
}
