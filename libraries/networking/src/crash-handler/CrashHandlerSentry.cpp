#include "CrashHandlerSentry.h"

#include "sentry.h"
#include <BuildInfo.h>
#include "../FingerprintUtils.h"
#include "../UserActivityLogger.h"
#include <UUID.h>


bool CrashHandlerSentry::startCrashHandler() {
    std::string appPath = path().toStdString();
    std::string crashURL = url().toStdString();
    std::string crashToken = token().toStdString();


    if (crashURL.empty() || crashToken.empty()) {
        qCCritical(crash_handler) << "Backtrace URL or token not set, crash handler disabled.";
        return false;
    }

    sentry_options_t *options = sentry_options_new();
    sentry_options_set_dsn(options, crashURL.c_str());
    sentry_options_set_release(options, crashToken.c_str());
    sentry_options_set_debug(options, 1); // Debug for Sentry Native SDK itself
    sentry_options_set_enable_logs(options, 1);
    sentry_options_set_enable_metrics(options, 1);
    sentry_options_set_cache_keep(options, SENTRY_CACHE_KEEP_OFFLINE);
    sentry_options_set_http_retry(options, true);
    sentry_options_set_require_user_consent(options, 1);
    sentry_options_set_minidump_mode(options, SENTRY_MINIDUMP_MODE_FULL); // TODO: a way to choose SENTRY_MINIDUMP_MODE_SMART, which sends smaller reports
    sentry_options_set_crashpad_wait_for_upload(options, 1); // Wait for crashpad to upload before exiting
    sentry_options_set_logs_with_attributes(options, true);

    if (sentry_init(options) == 0) {
        qCInfo(crash_handler) << "Sentry init successful";
    } else {
        qCCritical(crash_handler) << "Sentry init failed";
        return false;
    }


    sentry_value_t release_info = sentry_value_new_object();
    sentry_value_set_by_key(release_info, "version", sentry_value_new_string(BuildInfo::VERSION.toStdString().c_str()));
    sentry_value_set_by_key(release_info, "build", sentry_value_new_string(BuildInfo::BUILD_NUMBER.toStdString().c_str()));
    sentry_value_set_by_key(release_info, "build_time", sentry_value_new_string(BuildInfo::BUILD_TIME.toStdString().c_str()));
    sentry_value_set_by_key(release_info, "build_type", sentry_value_new_string(BuildInfo::BUILD_TYPE_STRING.toStdString().c_str()));

    sentry_value_t machine_info = sentry_value_new_object();
    sentry_value_set_by_key(machine_info, "fingerprint", sentry_value_new_string(uuidStringWithoutCurlyBraces(FingerprintUtils::getMachineFingerprint()).toStdString().c_str()))    ;


    sentry_set_context("Release", release_info);
    sentry_set_context("Machine", machine_info);

    sentry_set_tag("machine_id", uuidStringWithoutCurlyBraces(FingerprintUtils::getMachineFingerprint()).toStdString().c_str());
    sentry_set_tag("support_tag", getSupportTag().toStdString().c_str());

    qCInfo(crash_handler) << "Sentry crash handler initialized. SDK version" << sentry_sdk_version() << ". Will send reports to " << crashURL.c_str() << " with token " << crashToken.c_str();
    return true;

    //qCInfo(crash_handler) << "Crashpad uploads " << (enabled ? QString("enabled") : QString("disabled"));
}

void CrashHandlerSentry::sendLogMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)  {
    sentry_level_t level = SENTRY_LEVEL_INFO;

    sentry_value_t attributes = sentry_value_new_object();

    switch(type) {
        case QtMsgType::QtFatalMsg:
            level = SENTRY_LEVEL_FATAL;
            break;
        case QtMsgType::QtCriticalMsg:
            level = SENTRY_LEVEL_ERROR;
            break;
        case QtMsgType::QtWarningMsg:
            level = SENTRY_LEVEL_WARNING;
            break;
        case QtMsgType::QtInfoMsg:
            level = SENTRY_LEVEL_INFO;
            break;
        case QtMsgType::QtDebugMsg:
            level = SENTRY_LEVEL_DEBUG;
            break;
        default:
            sentry_log_warn("Log message of unknown type %i", (int)type);
    }

    sentry_value_set_by_key(attributes, "file", sentry_value_new_string(context.file));
    sentry_value_set_by_key(attributes, "line", sentry_value_new_int32(context.line));
    sentry_value_set_by_key(attributes, "function", sentry_value_new_string(context.function));
    sentry_value_set_by_key(attributes, "category", sentry_value_new_string(context.category));

    sentry_log(level, msg.toStdString().c_str(), attributes);

}

void CrashHandlerSentry::setCrashReportingEnabled(bool value) {
    if (value) {
        qCInfo(crash_handler) << "Sentry crash reporting consent given. Crash reports will be sent to the server.";
        sentry_user_consent_give();
    } else {
        qCInfo(crash_handler) << "Sentry crash reporting consent revoked.";
        sentry_user_consent_revoke();
    }
}

void CrashHandlerSentry::setTag(std::string name, std::string value) {
    sentry_set_tag(name.c_str(), value.c_str());
}

void CrashHandlerSentry::setContext(const QString &sectionName, const QString &key, const QVariant &value) {
    // Newer Sentry Native SDKs have sentry_update_context(), but we don't. Got to do things manually.

    QMutexLocker lock(&_contextMutex);

    QVariantMap section = _context.value(sectionName).toMap();
    section.insert(key, value);
    _context.insert(sectionName, section);


    foreach(const QString &sectionName, _context.keys()) {
        sentry_value_t section_info = sentry_value_new_object();
        QVariantMap sectionMap = _context.value(sectionName).toMap();

        foreach(const QString &key, sectionMap.keys()) {
            sentry_value_t value;

            if (sectionMap.value(key).type() == QVariant::String) {
                value = sentry_value_new_string(sectionMap.value(key).toString().toStdString().c_str());
            } else if (sectionMap.value(key).type() == QVariant::Int) {
                value = sentry_value_new_int32(sectionMap.value(key).toInt());
            } else if (sectionMap.value(key).type() == QVariant::LongLong) {
                value = sentry_value_new_int64(sectionMap.value(key).toLongLong());
            } else if (sectionMap.value(key).type() == QVariant::Double) {
                value = sentry_value_new_double(sectionMap.value(key).toDouble());
            } else if (sectionMap.value(key).type() == QVariant::Bool) {
                value = sentry_value_new_bool(sectionMap.value(key).toBool());
            } else {
                qCWarning(crash_handler) << "Unsupported context value type" << sectionMap.value(key).typeName() << "for key" << key << "in section" << sectionName << ", adding as a string.";
                value = sentry_value_new_string(sectionMap.value(key).toString().toStdString().c_str());
            }

            sentry_value_set_by_key(section_info, key.toStdString().c_str(), value);
        }

        sentry_set_context(sectionName.toStdString().c_str(), section_info);
    }

}
