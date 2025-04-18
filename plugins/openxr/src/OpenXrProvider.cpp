//
// Overte OpenXR Plugin
//
// Copyright 2024 Lubosz Sarnecki
// Copyright 2024 Overte e.V.
//
// SPDX-License-Identifier: Apache-2.0
//

#include "plugins/RuntimePlugin.h"
#include "OpenXrDisplayPlugin.h"
#include "OpenXrInputPlugin.h"

class OpenXrProvider : public QObject, public DisplayProvider, InputProvider {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DisplayProvider_iid FILE "plugin.json")
    Q_INTERFACES(DisplayProvider)
    Q_PLUGIN_METADATA(IID InputProvider_iid FILE "plugin.json")
    Q_INTERFACES(InputProvider)

public:
    OpenXrProvider(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~OpenXrProvider() {}
    std::shared_ptr<OpenXrContext> context = std::make_shared<OpenXrContext>();

    virtual DisplayPluginList getDisplayPlugins() override {
        if (!context->_isSupported) {
            return _displayPlugins;
        }

        static std::once_flag once;
        std::call_once(once, [&] {
            DisplayPluginPointer plugin(std::make_shared<OpenXrDisplayPlugin>(context));
            if (plugin->isSupported()) {
                _displayPlugins.push_back(plugin);
            }
        });

        return _displayPlugins;
    }

    virtual InputPluginList getInputPlugins() override {
        if (!context->_isSupported) {
            return _inputPlugins;
        }

        static std::once_flag once;

        std::call_once(once, [&] {
            InputPluginPointer plugin(std::make_shared<OpenXrInputPlugin>(context));
            if (plugin->isSupported()) {
                _inputPlugins.push_back(plugin);
            }
        });

        return _inputPlugins;
    }

    virtual void destroyInputPlugins() override { _inputPlugins.clear(); }

    virtual void destroyDisplayPlugins() override { _displayPlugins.clear(); }

private:
    DisplayPluginList _displayPlugins;
    InputPluginList _inputPlugins;
};

#include "OpenXrProvider.moc"
