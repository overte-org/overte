//
//  ArchiveDownloadInterface.h
//  libraries/script-engine/src
//
//  Created by Elisa Lupin-Jimenez on 6/28/16.
//  Copyright 2016 High Fidelity, Inc.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

/// @addtogroup Interface
/// @{

#ifndef hifi_ArchiveDownloadInterface_h
#define hifi_ArchiveDownloadInterface_h

#include <QtCore/QObject>
#include <QtCore/QUrl>
#include <QFileInfo>
#include <QString>

/**
 * @brief The ArchiveDownloadInterface API provides some facilities for working with the file system.
 */
class ArchiveDownloadInterface : public QObject {
    Q_OBJECT

public:
    ArchiveDownloadInterface(QObject* parent);

public slots:

    /**
     * @brief Extracts a filename from a URL, where the filename is specified in the query part of the URL as filename=.

     * @param url - The URL to extract the filename from.
     * @return The filename specified in the URL; an empty string if no filename is specified.
     */
    QString convertUrlToPath(QUrl url);

    /**
     * @brief Unzips a file in the local file system to a new, unique temporary directory.
     * @param path - The path of the zip file in the local file system. May have a leading "file:///".
     *     Need not have a ".zip" extension if it is in a temporary directory (as created by
     *     getTempDir).
     * @param url - Not used.
     * @param autoAdd - Not used by user scripts. The value is simply passed through to the
     *     unzipResult signal.
     * @param isZip - Set to true if path has a ".zip" extension,
     *     false if it doesn't (but should still be treated as a zip file).
     * @example (Old example from JS, needs to be converted)
     * Select and unzip a file.
     * File.unzipResult.connect(function (zipFile, unzipFiles, autoAdd, isZip) {
     *     print("File.unzipResult()");
     *     print("- zipFile: " + zipFile);
     *     print("- unzipFiles(" + unzipFiles.length + "): " + unzipFiles);
     *     print("- autoAdd: " + autoAdd);
     *     print("- isZip: " + isZip);
     * });
     * 
     * var zipFile = Window.browse("Select a Zip File", "", "*.zip");
     * if (zipFile) {
     *     File.runUnzip(zipFile, "", false, true);
     * } else {
     *     print("Zip file not selected.");
     * }
     */
    void runUnzip(QString path, QUrl url, bool autoAdd, bool isZip);

    /**
     * Creates a new, unique directory for temporary use.
     * @return The path of the newly created temporary directory.
     */
    QString getTempDir();

signals:

    /**
     * Triggered when runUnzip completes.
     * @param zipFile - The file that was unzipped.
     * @param unzipFiles - The paths of the unzipped files in a newly created temporary directory. Includes entries
     *     for any subdirectories created. An empty array if the zipFile could not be unzipped.
     * @param autoAdd - The value that runUnzip was called with.
     * @param isZip - true if runUnzip was called with isZip == true,
     *     unless there is no FBX or OBJ file in the unzipped file(s) in which case the value is false.
     */
    void unzipResult(QString zipFile, QStringList unzipFile, bool autoAdd, bool isZip);

private:
    bool isTempDir(QString tempDir);
    bool hasModel(QStringList fileList);
    QStringList unzipFile(QString path, QString tempDir);
    void recursiveFileScan(QFileInfo file, QString* dirName);
    void downloadZip(QString path, const QString link);

};

#endif // hifi_ArchiveDownloadInterface_h

/// @}
