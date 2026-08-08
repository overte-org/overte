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

    sentry_options_set_require_user_consent(options, 1);
    sentry_options_set_minidump_mode(options, SENTRY_MINIDUMP_MODE_FULL); // TODO: a way to choose SENTRY_MINIDUMP_MODE_SMART, which sends smaller reports

    if (sentry_init(options) == 0) {
        qCInfo(crash_handler) << "Sentry init successful";
    } else {
        qCCritical(crash_handler) << "Sentry init failed";
        return false;
    }


    sentry_value_t release_info = sentry_value_new_object();
    sentry_value_set_by_key(release_info, "version", sentry_value_new_string(BuildInfo::VERSION.toStdString().c_str()));
    sentry_value_set_by_key(release_info, "build", sentry_value_new_string(BuildInfo::BUILD_NUMBER.toStdString().c_str()));

    sentry_value_t machine_info = sentry_value_new_object();
    sentry_value_set_by_key(machine_info, "fingerprint", sentry_value_new_string(uuidStringWithoutCurlyBraces(FingerprintUtils::getMachineFingerprint()).toStdString().c_str()))    ;


    sentry_set_context("Release", release_info);
    sentry_set_context("Machine", machine_info);


    qCInfo(crash_handler) << "Sentry crash handler initialized. SDK version" << sentry_sdk_version() << ". Will send reports to " << crashURL.c_str() << " with token " << crashToken.c_str();
    return true;

    //qCInfo(crash_handler) << "Crashpad uploads " << (enabled ? QString("enabled") : QString("disabled"));
}

void CrashHandlerSentry::sendLogMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)  {
    switch(type) {
        case QtMsgType::QtFatalMsg:
            sentry_log_fatal(msg.toStdString().c_str());
            break;
        case QtMsgType::QtCriticalMsg:
            sentry_log_error(msg.toStdString().c_str());
            break;
        case QtMsgType::QtWarningMsg:
            sentry_log_warn(msg.toStdString().c_str());
            break;
        case QtMsgType::QtInfoMsg:
            sentry_log_info(msg.toStdString().c_str());
            break;
        case QtMsgType::QtDebugMsg:
            sentry_log_debug(msg.toStdString().c_str());
            break;
        default:
            sentry_log_warn(msg.toStdString().c_str());
            sentry_log_warn("Previous log message was of unknown type %i", (int)type);
    }
}

void CrashHandlerSentry::setCrashReportingEnabled(bool value) {
    if (value) {
        sentry_user_consent_give();
    } else {
        sentry_user_consent_revoke();
    }
}

void CrashHandlerSentry::setTag(std::string name, std::string value) {
    sentry_set_tag(name.c_str(), value.c_str());
}
