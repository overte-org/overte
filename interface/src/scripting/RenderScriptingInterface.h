//
//  Created by Sam Gondelman on 5/16/19
//  Copyright 2013-2019 High Fidelity, Inc.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef hifi_RenderScriptingInterface_h
#define hifi_RenderScriptingInterface_h

#include <QtCore/QObject>
#include "Application.h"

#include "RenderForward.h"
#include "AntialiasingEffect.h"
#include <QScreen>

/*@jsdoc
 * The <code>Render</code> API enables you to configure the graphics engine.
 *
 * @namespace Render
 *
 * @hifi-interface
 * @hifi-client-entity
 * @hifi-avatar
 *
 * @property {Render.RenderMethod} renderMethod - The render method being used.
 * @property {boolean} shadowsEnabled - <code>true</code> if shadows are enabled, <code>false</code> if they're disabled.
 * @property {boolean} ambientOcclusionEnabled - <code>true</code> if ambient occlusion is enabled, <code>false</code> if it's
 *     disabled.
 * @property {integer} antialiasingMode - The active anti-aliasing mode.
 * @property {number} viewportResolutionScale - The view port resolution scale, <code>&gt; 0.0</code>.
 */
class RenderScriptingInterface : public QObject {
    Q_OBJECT
    Q_PROPERTY(RenderMethod renderMethod READ getRenderMethod WRITE setRenderMethod NOTIFY settingsChanged)
    Q_PROPERTY(bool shadowsEnabled READ getShadowsEnabled WRITE setShadowsEnabled NOTIFY settingsChanged)
    Q_PROPERTY(bool ambientOcclusionEnabled READ getAmbientOcclusionEnabled WRITE setAmbientOcclusionEnabled NOTIFY settingsChanged)
    Q_PROPERTY(AntialiasingConfig::Mode antialiasingMode READ getAntialiasingMode WRITE setAntialiasingMode NOTIFY settingsChanged)
    Q_PROPERTY(float viewportResolutionScale READ getViewportResolutionScale WRITE setViewportResolutionScale NOTIFY settingsChanged)
    Q_PROPERTY(float verticalFieldOfView READ getVerticalFieldOfView WRITE setVerticalFieldOfView NOTIFY settingsChanged)

public:
    RenderScriptingInterface();

    static RenderScriptingInterface* getInstance();

    /*@jsdoc
     * <p>The rendering method is specified by the following values:</p>
     * <table>
     *   <thead>
     *     <tr><th>Value</th><th>Name</th><th>Description</th>
     *   </thead>
     *   <tbody>
     *     <tr><td><code>0</code></td><td>DEFERRED</td><td>More complex rendering pipeline where lighting is applied to the
     *       scene as a whole after all objects have been rendered.</td></tr>
     *     <tr><td><code>1</code></td><td>FORWARD</td><td>Simpler rendering pipeline where each object in the scene, in turn,
     *       is rendered and has lighting applied.</td></tr>
     *   </tbody>
     * </table>
     * @typedef {number} Render.RenderMethod
     */
    // RenderMethod enum type
    enum class RenderMethod {
        DEFERRED = render::Args::RenderMethod::DEFERRED,
        FORWARD = render::Args::RenderMethod::FORWARD,
    };
    Q_ENUM(RenderMethod)
    static bool isValidRenderMethod(RenderMethod value) { return (value >= RenderMethod::DEFERRED && value <= RenderMethod::FORWARD); }


    // Load Settings
    // Synchronize the runtime value to the actual setting
    // Need to be called on start up to re-initialize the runtime to the saved setting states
    void loadSettings();


public slots:
    /*@jsdoc
     * Gets the configuration for a rendering job by name.
     * <p><strong>Warning:</strong> For internal, debugging purposes. Subject to change.</p>
     * @function Render.getConfig
     * @param {string} name - The name of the rendering job.
     * @returns {object} The configuration for the rendering job.
     */
    QObject* getConfig(const QString& name) { return qApp->getRenderEngine()->getConfiguration()->getConfig(name); }


    /*@jsdoc
     * Gets the render method being used.
     * @function Render.getRenderMethod
     * @returns {Render.RenderMethod} The render method being used.
     * @example <caption>Report the current render method.</caption>
     * var renderMethod = Render.getRenderMethod();
     * print("Current render method: " + Render.getRenderMethodNames()[renderMethod]);
     */
    RenderMethod getRenderMethod() const;

    /*@jsdoc
     * Sets the render method to use.
     * @function Render.setRenderMethod
     * @param {Render.RenderMethod} renderMethod - The render method to use.
     */
    void setRenderMethod(RenderMethod renderMethod);

    /*@jsdoc
     * Gets the names of the possible render methods, per {@link Render.RenderMethod}.
     * @function Render.getRenderMethodNames
     * @returns {string[]} The names of the possible render methods.
     * @example <caption>Report the names of the possible render methods.</caption>
     * var renderMethods = Render.getRenderMethodNames();
     * print("Render methods:");
     * for (var i = 0; i < renderMethods.length; i++) {
     *     print("- " + renderMethods[i]);
     * }
     */
    QStringList getRenderMethodNames() const;


    /*@jsdoc
     * Gets whether or not shadows are enabled.
     * @function Render.getShadowsEnabled
     * @returns {boolean} <code>true</code> if shadows are enabled, <code>false</code> if they're disabled.
     */
    bool getShadowsEnabled() const;

    /*@jsdoc
     * Sets whether or not shadows are enabled.
     * @function Render.setShadowsEnabled
     * @param {boolean} enabled - <code>true</code> to enable shadows, <code>false</code> to disable.
     */
    void setShadowsEnabled(bool enabled);

    /*@jsdoc
     * Gets whether or not ambient occlusion is enabled.
     * @function Render.getAmbientOcclusionEnabled
     * @returns {boolean} <code>true</code> if ambient occlusion is enabled, <code>false</code> if it's disabled.
     */
    bool getAmbientOcclusionEnabled() const;

    /*@jsdoc
     * Sets whether or not ambient occlusion is enabled.
     * @function Render.setAmbientOcclusionEnabled
     * @param {boolean} enabled - <code>true</code> to enable ambient occlusion, <code>false</code> to disable.
     */
    void setAmbientOcclusionEnabled(bool enabled);

    /*@jsdoc
     * Gets the active anti-aliasing mode.
     * @function Render.getAntialiasingMode
     * @returns {AntialiasingMode} The active anti-aliasing mode.
     */
    AntialiasingConfig::Mode getAntialiasingMode() const;

    /*@jsdoc
     * Sets the active anti-aliasing mode.
     * @function Render.setAntialiasingMode
     * @param {AntialiasingMode} The active anti-aliasing mode.
     */
    void setAntialiasingMode(AntialiasingConfig::Mode mode);

    /*@jsdoc
     * Gets the view port resolution scale.
     * @function Render.getViewportResolutionScale
     * @returns {number} The view port resolution scale, <code>&gt; 0.0</code>.
     */
    float getViewportResolutionScale() const;

    /*@jsdoc
     * Sets the view port resolution scale.
     * @function Render.setViewportResolutionScale
     * @param {number} resolutionScale - The view port resolution scale to set, <code>&gt; 0.0</code>.
     */
    void setViewportResolutionScale(float resolutionScale);

    /*@jsdoc
     * Returns the list of screens
     * @function Render.getScreens
     * @returns {string[]} The names of the available screens.
     */
    QStringList getScreens() const;

    /*@jsdoc
     * Gets the screen used when switching to full screen mode.
     * @function Render.getFullScreenScreen
     * @returns {string} The name of the screen used for full screen mode.
     */
    QString getFullScreenScreen() const;

    /*@jsdoc
     * Sets the screen used when switching to full screen mode.
     * This function will only succeed if the name passed is one of the entries from Render.getScreens.
     * Otherwise, it will return False and have no effect.
     *
     * @function Render.setFullScreenScreen
     * @returns {bool} True if the setting was successful.
     */
    bool setFullScreenScreen(QString name);

    /*@jsdoc
     * Gets the vertical field of view in degrees.
     * @function Render.getVerticalFieldOfView
     * @returns {number} The vertical field of view in degrees.
     */
    float getVerticalFieldOfView() { return qApp->getFieldOfView(); }

    /*@jsdoc
     * Sets the vertical field of view in degrees.
     * @function Render.setVerticalFieldOfView
     * @param {number} fieldOfView - The vertical field of view in degrees to set.
     */
    void setVerticalFieldOfView( float fieldOfView );

signals:

    /*@jsdoc
     * Triggered when one of the <code>Render</code> API's properties changes.
     * @function Render.settingsChanged
     * @returns {Signal}
     * @example <caption>Report when a render setting changes.</caption>
     * Render.settingsChanged.connect(function () {
     *     print("Render setting changed");
     * });
     * // Toggle Developer > Render > Shadows or similar to trigger.
     */
    void settingsChanged();

private:
    // One lock to serialize and access safely all the settings
    mutable ReadWriteLockable _renderSettingLock;

    // Runtime value of each settings
    int  _renderMethod{ RENDER_FORWARD ? render::Args::RenderMethod::FORWARD : render::Args::RenderMethod::DEFERRED };
    bool _shadowsEnabled{ true };
    bool _ambientOcclusionEnabled{ false };
    AntialiasingConfig::Mode _antialiasingMode{ AntialiasingConfig::Mode::NONE };
    float _viewportResolutionScale{ 1.0f };
    QString _fullScreenScreen;


    // Actual settings saved on disk
    Setting::Handle<int> _renderMethodSetting { "renderMethod", RENDER_FORWARD ? render::Args::RenderMethod::FORWARD : render::Args::RenderMethod::DEFERRED };
    Setting::Handle<bool> _shadowsEnabledSetting { "shadowsEnabled", true };
    Setting::Handle<bool> _ambientOcclusionEnabledSetting { "ambientOcclusionEnabled", false };
    //Setting::Handle<AntialiasingConfig::Mode> _antialiasingModeSetting { "antialiasingMode", AntialiasingConfig::Mode::TAA };
    Setting::Handle<int> _antialiasingModeSetting { "antialiasingMode", AntialiasingConfig::Mode::NONE };
    Setting::Handle<float> _viewportResolutionScaleSetting { "viewportResolutionScale", 1.0f };
    Setting::Handle<QString> _fullScreenScreenSetting { "fullScreenScreen", "" };

    // Force assign both setting AND runtime value to the parameter value
    void forceRenderMethod(RenderMethod renderMethod);
    void forceShadowsEnabled(bool enabled);
    void forceAmbientOcclusionEnabled(bool enabled);
    void forceAntialiasingMode(AntialiasingConfig::Mode mode);
    void forceViewportResolutionScale(float scale);

    static std::once_flag registry_flag;
};

#endif  // hifi_RenderScriptingInterface_h
