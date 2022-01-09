#include <QtCore/QCoreApplication>
#include <QtCore/QString>
#include <QtCore/QFileInfo>

#include "Helpers.h"

#include <mutex>

#include <gl/Config.h>
#include <shared/FileUtils.h>

const QString& getPipelineCacheFile() {
    static const QString PIPELINE_CACHE_FOLDER{ "" };
    static const QString PIPELINE_CACHE_FILE_NAME{ "pipeline_cache.bin" };
    static const QString PIPELINE_CACHE_FILE = FileUtils::standardPath(PIPELINE_CACHE_FOLDER) + PIPELINE_CACHE_FILE_NAME;
    return PIPELINE_CACHE_FOLDER;
}

bool vks::util::loadPipelineCacheData(std::vector<uint8_t>& outCache) {
    outCache.clear();
    const QString& cacheFile = getPipelineCacheFile();
    if (QFileInfo(cacheFile).exists()) {
        QFile file(cacheFile);
        if (file.open(QFile::ReadOnly)) {
            QByteArray data = file.readAll();
            outCache.resize(data.size());
            memcpy(outCache.data(), data.data(), data.size());
            file.close();
            return true;
        }
    }
    return false;
}

void vks::util::savePipelineCacheData(const std::vector<uint8_t>& cache) {
    QString cacheFile = getPipelineCacheFile();
    QFile saveFile(cacheFile);
    saveFile.open(QFile::WriteOnly | QFile::Truncate);
    saveFile.write((const char*)cache.data(), cache.size());
    saveFile.close();
}

static std::set<std::string> getGLExtensions() {
    static std::set<std::string> result;
    static std::once_flag once;
    std::call_once(once, [&]{
        GLint count = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &count);
        for (GLint i = 0; i < count; ++i) {
            auto name = glGetStringi(GL_EXTENSIONS, i);
            result.insert((const char*)name);
        }
    });
    return result;
}

static bool hasExtension(const std::string& name) {
    const auto& extensions = getGLExtensions();
    return 0 != extensions.count(name);
}


vks::util::gl::UuidSet vks::util::gl::getUuids() {
    static vks::util::gl::UuidSet result;
    static std::once_flag once;
    QUuid driverUuid;
    using GLUUID = std::array<GLint, 16>;

    std::call_once(once, [&]{
        GLUUID value;
        glGetIntegerv(GL_DRIVER_UUID_EXT, value.data());

        GLint deviceIdCount = 0;
        glGetIntegerv(GL_NUM_DEVICE_UUIDS_EXT, &deviceIdCount);
        for (GLint i = 0; i < deviceIdCount; ++i) {
            result.insert(QUuid(QByteArray((const char*)value.data(), (int)value.size())));
        }
    });
    return result;
}

bool vks::util::gl::contextSupported(QOpenGLContext*) {
    return hasExtension("GL_EXT_memory_object") && hasExtension("GL_EXT_semaphore");
 }



