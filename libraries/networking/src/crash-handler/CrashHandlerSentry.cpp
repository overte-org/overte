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
