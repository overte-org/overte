

#ifndef hifi_CrashHandlerNone_h
#define hifi_CrashHandlerNone_h

#include "CrashHandler.h"


class CrashHandlerNone : public CrashHandler {
    Q_OBJECT

    public:


    virtual void setTag([[maybe_unused]] std::string name, [[maybe_unused]] std::string value) override {
        // This space intentionally left blank.
    }

    virtual void setContext([[maybe_unused]] const QString &sectionName, [[maybe_unused]] const QString &key, [[maybe_unused]] const QVariant &value) override {
        // This space intentionally left blank.
    }

    virtual void attachFile(const QString &file) override {
        // This space intentionally left blank.
    }

    virtual void clearAttachments() override {
        // This space intentionally left blank.
    }

    virtual void setCounter([[maybe_unused]] const QString &name, [[maybe_unused]] qint64 value, [[maybe_unused]] MetricsUnit unit = MetricsUnit::None) override {
        // This space intentionally left blank.
    }

    virtual void setGauge([[maybe_unused]] const QString &name, [[maybe_unused]] double value, [[maybe_unused]] MetricsUnit unit = MetricsUnit::None) override {
        // This space intentionally left blank.
    }

    protected:

    virtual bool startCrashHandler() override {
        qCWarning(crash_handler) << "No crash handler available.";
        return false;
    }

    virtual void setCrashReportingEnabled([[maybe_unused]] bool value) override {
        // This space intentionally left blank.
    }

    virtual void sendLogMessage([[maybe_unused]] QtMsgType type, [[maybe_unused]] const QMessageLogContext &context, [[maybe_unused]] const QString &msg) override {
        // This space intentionally left blank.
    }

};

#endif
