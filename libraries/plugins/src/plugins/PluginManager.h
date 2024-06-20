//
//  Created by Bradley Austin Davis on 2015/08/08
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once

#include <QObject>
#include <QtCore/QSharedPointer>

#include <DependencyManager.h>
#include <SettingHandle.h>
#include <QJsonDocument>
#include <QJsonObject>

#include "Forward.h"

class QPluginLoader;
using PluginManagerPointer = QSharedPointer<PluginManager>;

/**
 * @brief Manages loadable plugins
 *
 * The current implementation does initialization only once, as soon as it's needed.
 * Once things are initialized the configuration is made permanent.
 *
 * Both loadable and statically modules are supported. Static modules have to be provided
 * with setDisplayPluginProvider, setInputPluginProvider and setCodecPluginProvider.
 *
 * @warning Users of the PluginManager must take care to do any configuration very early
 * on, because changes become impossible once initialization is done. Plugins can't be
 * added or removed once that happens.
 *
 * Initialization is performed in the getDisplayPlugins, getInputPlugins and getCodecPlugins
 * functions.
 */
class PluginManager : public QObject, public Dependency {
    SINGLETON_DEPENDENCY
    Q_OBJECT

public:

    /**
     * @brief Information about known plugins
     *
     */
    struct PluginInfo {
        /**
         * @brief Plugin metadata
        */
        QJsonObject metaData;

        /**
         * @brief Filename
         *
         */
        QString name;

        /**
         * @brief Whether the plugin has been disabled
         *
         */
        bool disabled = false;

        /**
         * @brief Whether the plugin has been filtered out by a filter
         *
         */
        bool filteredOut = false;

        /**
         * @brief Whether the plugin has been not loaded because it's the wrong version
         *
         */
        bool wrongVersion = false;

        /**
         * @brief Whether the plugin has been loaded successfully
         *
         */
        bool loaded = false;
    };


    static PluginManagerPointer getInstance();

    /**
     * @brief Get the list of display plugins
     *
     * @note Calling this function will perform initialization and
     * connects events to all the known the plugins on the first call.
     *
     * @return const DisplayPluginList&
     */
    const DisplayPluginList& getDisplayPlugins();

    /**
     * @brief Get the list of input plugins
     *
     * @note Calling this function will perform initialization and
     * connects events to all the known the plugins on the first call.
     *
     * @return const InputPluginList&
     */
    const InputPluginList& getInputPlugins();

    /**
     * @brief Get the list of audio codec plugins
     *
     * @note Calling this function will perform initialization and
     * connects events to all the known the plugins on the first call.
     *
     * @return const CodecPluginList&
     */
    const CodecPluginList& getCodecPlugins();

    /**
     * @brief Get the pointer to the Steam client plugin
     *
     * This may return a null pointer if Steam support isn't built in.
     *
     * @return const SteamClientPluginPointer
     */
    const SteamClientPluginPointer getSteamClientPlugin();

    /**
     * @brief Get the pointer to the Oculus Platform Plugin
     *
     * This may return a null pointer if Oculus support isn't built in.
     *
     * @return const OculusPlatformPluginPointer
     */
    const OculusPlatformPluginPointer getOculusPlatformPlugin();

    /**
     * @brief Returns the list of preferred display plugins
     *
     * The preferred display plugins are set by setPreferredDisplayPlugins.
     *
     * @return DisplayPluginList
     */
    DisplayPluginList getPreferredDisplayPlugins();

    /**
     * @brief Sets the list of preferred display plugins
     *
     * @note This must be called early, before any call to getPreferredDisplayPlugins.
     *
     * @param displays
     */
    void setPreferredDisplayPlugins(const QStringList& displays);

    /**
     * @brief Disable a list of displays
     *
     * This adds the display to a list of displays not to be used.
     *
     * @param displays
     */
    void disableDisplays(const QStringList& displays);

    /**
     * @brief Disable a list of inputs
     *
     * This adds the input to a list of inputs not to be used.
     * @param inputs
     */
    void disableInputs(const QStringList& inputs);

    /**
     * @brief Save the settings
     *
     */
    void saveSettings();

    /**
     * @brief Set the container for plugins
     *
     * This will be passed to all active plugins on initialization.
     *
     * @param container
     */
    void setContainer(PluginContainer* container) { _container = container; }

    int instantiate();
    void shutdown();


    /**
     * @brief Provide a list of statically linked plugins.
     *
     * This is used to provide a list of statically linked plugins to the plugin manager.
     *
     * @note This must be called very early on, and only works once. Once the plugin manager
     * builds its internal list of plugins, the final list becomes set in stone.
     *
     * @param provider A std::function that returns a list of display plugins
     */
    void setDisplayPluginProvider(const DisplayPluginProvider& provider);

    /**
     * @brief Provide a list of statically linked plugins.
     *
     * This is used to provide a list of statically linked plugins to the plugin manager.
     *
     * @note This must be called very early on, and only works once. Once the plugin manager
     * builds its internal list of plugins, the final list becomes set in stone.
     *
     * @param provider A std::function that returns a list of input plugins
     */
    void setInputPluginProvider(const InputPluginProvider& provider);

    /**
     * @brief Provide a list of statically linked plugins.
     *
     * This is used to provide a list of statically linked plugins to the plugin manager.
     *
     * @note This must be called very early on, and only works once. Once the plugin manager
     * builds its internal list of plugins, the final list becomes set in stone.
     *
     * @param provider A std::function that returns a list of codec plugins
     */
    void setCodecPluginProvider(const CodecPluginProvider& provider);

    /**
     * @brief Set the input plugin persister
     *
     * @param persister A std::function that saves input plugin settings
     */
    void setInputPluginSettingsPersister(const InputPluginSettingsPersister& persister);

    /**
     * @brief Get the list of running input devices
     *
     * @return QStringList List of input devices in running state
     */
    QStringList getRunningInputDeviceNames() const;

    using PluginFilter = std::function<bool(const QJsonObject&)>;

    /**
     * @brief Set the plugin filter that determines whether a plugin will be used or not
     *
     * @note This must be called very early on. Once the plugin manager
     * builds its internal list of plugins, the final list becomes set in stone.
     *
     * As of writing, this is used in the audio mixer.
     *
     * @param pluginFilter
     */
    void setPluginFilter(PluginFilter pluginFilter) { _pluginFilter = pluginFilter; }

    /**
     * @brief Get a list of all the display plugins
     *
     * @return DisplayPluginList List of display plugins
     */
    Q_INVOKABLE DisplayPluginList getAllDisplayPlugins();

    bool getEnableOculusPluginSetting() { return _enableOculusPluginSetting.get(); }
    void setEnableOculusPluginSetting(bool value);

    /**
     * @brief Returns information about known plugins
     *
     * This is a function for informative/debugging purposes.
     *
     * @return std::vector<PluginInfo>
     */
    std::vector<PluginInfo> getPluginInfo() const;

signals:
    void inputDeviceRunningChanged(const QString& pluginName, bool isRunning, const QStringList& runningDevices);

private:
    PluginManager() = default;

    DisplayPluginProvider _displayPluginProvider { []()->DisplayPluginList { return {}; } };
    InputPluginProvider _inputPluginProvider { []()->InputPluginList { return {}; } };
    CodecPluginProvider _codecPluginProvider { []()->CodecPluginList { return {}; } };
    InputPluginSettingsPersister _inputSettingsPersister { [](const InputPluginList& list) {} };
    PluginContainer* _container { nullptr };
    DisplayPluginList _displayPlugins;
    InputPluginList _inputPlugins;
    PluginFilter _pluginFilter { [](const QJsonObject&) { return true; } };

    using Loader = QSharedPointer<QPluginLoader>;
    using LoaderList = QList<Loader>;

    const LoaderList& getLoadedPlugins() const;
    Setting::Handle<bool> _enableScriptingPlugins {
        "private/enableScriptingPlugins", (bool)qgetenv("enableScriptingPlugins").toInt()
    };

    Setting::Handle<bool> _enableOculusPluginSetting { "enableOculusPluginSetting", false };
};

// TODO: we should define this value in CMake, and then use CMake
// templating to generate the individual plugin.json files, so that we
// don't have to update every plugin.json file whenever we update this
// value.  The value should match "version" in
//   plugins/*/src/plugin.json
//   plugins/oculus/src/oculus.json
//   etc
static const int HIFI_PLUGIN_INTERFACE_VERSION = 1;
