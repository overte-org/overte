//
//  Snapshot.h
//  interface/src/ui
//
//  Created by Stojce Slavkovski on 1/26/14.
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_Snapshot_h
#define hifi_Snapshot_h

#include <glm/glm.hpp>

#include <QString>
#include <QStandardPaths>
#include <QUrl>
#include <QTimer>
#include <QtGui/QImage>

#include <SettingHandle.h>
#include <DependencyManager.h>

class QFile;
class QTemporaryFile;

class SnapshotMetaData {
public:

    QUrl getURL() { return _URL; }
    void setURL(QUrl URL) { _URL = URL; }

private:
    QUrl _URL;
};


/*@jsdoc
 * The <code>Snapshot</code> API provides access to the path that snapshots are saved to. This path is that provided in 
 * Settings &gt; General &gt; Snapshots. Snapshots may be taken using <code>Window</code> API functions such as 
 * {@link Window.takeSnapshot}.
 *
 * @namespace Snapshot
 * 
 * @hifi-interface
 * @hifi-client-entity
 * @hifi-avatar
 */

class Snapshot : public QObject, public Dependency {
    Q_OBJECT
    SINGLETON_DEPENDENCY
public:
    Snapshot();
    QString saveSnapshot(QImage image, const QString& filename, const QString& pathname = QString());
    void save360Snapshot(const glm::vec3& cameraPosition,
                         const bool& cubemapOutputFormat,
                         const bool& notify,
                         const QString& filename);
    QTemporaryFile* saveTempSnapshot(QImage image);
    SnapshotMetaData* parseSnapshotData(QString snapshotPath);

    Setting::Handle<QString> _snapshotsLocation{ "snapshotsLocation" };
    Setting::Handle<bool> _snapshotNotifications{ "snapshotNotifications", true };
    Setting::Handle<QString> _snapshotFormat{ "snapshotFormat" };
    Setting::Handle<QString> _animatedSnapshotFormat{ "animatedSnapshotFormat" };
    void uploadSnapshot(const QString& filename, const QUrl& href = QUrl(""));

signals:

    /*@jsdoc
     * Triggered when the path that snapshots are saved to is changed.
     * @function Snapshot.snapshotLocationSet
     * @param {string} location - The new snapshots location.
     * @returns {Signal}
     * @example <caption>Report when the snapshots location is changed.</caption>
     * // Run this script then change the snapshots location in Settings > General > Snapshots.
     * Snapshot.snapshotLocationSet.connect(function (path) {
     *     print("New snapshot location: " + path);
     * });
     */
    void snapshotLocationSet(const QString& value);

public slots:

    /*@jsdoc
     * Gets the path that snapshots are saved to.
     * @function Snapshot.getSnapshotsLocation
     * @returns {string} The path to save snapshots to.
     */
    Q_INVOKABLE QString getSnapshotsLocation();

    /*@jsdoc
     * Sets the path that snapshots are saved to.
     * @function Snapshot.setSnapshotsLocation
     * @param {String} location - The path to save snapshots to.
     */
    Q_INVOKABLE void setSnapshotsLocation(const QString& location);
    
    /*@jsdoc
     * Gets the currently selected snapshot format.
     * @function Snapshot.getSnapshotFormat
     * @returns {string} Currently selected snapshot format.
     */
    Q_INVOKABLE QString getSnapshotFormat();

    /*@jsdoc
     * Sets the snapshot format.
     * @function Snapshot.setSnapshotFormat
     * @param {String} format - one of the format names returned by Snapshot.getAvailableSnapshotFormats().
     */
    Q_INVOKABLE void setSnapshotFormat(const QString& format);

    /*@jsdoc
     * Gets the currently selected animated snapshot format.
     * @function Snapshot.getAnimatedSnapshotFormat
     * @returns {Array.<string>} Currently selected snapshot format.
     */
    Q_INVOKABLE QString getAnimatedSnapshotFormat();

    /*@jsdoc
     * Sets the snapshot format.
     * @function Snapshot.setAnimatedSnapshotFormat
     * @param {String} format - one of the format names returned by Snapshot.getAvailableSnapshotFormats().
     */
    Q_INVOKABLE void setAnimatedSnapshotFormat(const QString& format);

    /*@jsdoc
     * Returns a list of supported snapshot formats.
     * @function Snapshot.getAvailableSnapshotFormats
     * @returns {Array.<string>} List of supported snapshot formats.
     */
    Q_INVOKABLE QStringList getAvailableSnapshotFormats();

    /*@jsdoc
     * Returns a list of supported snapshot formats with short descriptions.
     * @function Snapshot.getAvailableSnapshotFormatsWithDescriptions
     * @returns {Array.<string>} List of supported snapshot formats with short descriptions.
     */
    Q_INVOKABLE QStringList getAvailableSnapshotFormatsWithDescriptions();

    /*@jsdoc
     * Returns a list of supported animated snapshot formats.
     * @function Snapshot.getAvailableAnimatedSnapshotFormats
     * @returns {Array.<string>} List of supported animated snapshot formats.
     */
    Q_INVOKABLE QStringList getAvailableAnimatedSnapshotFormats();

    /*@jsdoc
     * Returns a list of supported animated snapshot formats with short descriptions.
     * @function Snapshot.getAvailableAnimatedSnapshotFormatsWithDescriptions
     * @returns {Array.<string>} List of supported animated snapshot formats with short descriptions.
     */
    Q_INVOKABLE QStringList getAvailableAnimatedSnapshotFormatsWithDescriptions();
    
    

private slots:
    void takeNextSnapshot();

private:
    QFile* savedFileForSnapshot(QImage& image,
                                       bool isTemporary,
                                       const QString& userSelectedFilename = QString(),
                                       const QString& userSelectedPathname = QString());
    QString _snapshotFilename;
    bool _notify360;
    bool _cubemapOutputFormat;
    QTimer _snapshotTimer;
    qint16 _snapshotIndex;
    bool _waitingOnSnapshot { false };
    bool _taking360Snapshot { false };
    bool _oldEnabled;
    QVariant _oldAttachedEntityId;
    QVariant _oldOrientation;
    QVariant _oldvFoV;
    QVariant _oldNearClipPlaneDistance;
    QVariant _oldFarClipPlaneDistance;
    QImage _imageArray[6];
    void convertToCubemap();
    void convertToEquirectangular();
};

#endif // hifi_Snapshot_h
