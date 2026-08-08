

#ifndef hifi_CrashHandlerNone_h
#define hifi_CrashHandlerNone_h

#include "CrashHandler.h"

class CrashHandlerNone : public CrashHandler {
    Q_OBJECT

    public:


    protected:

    virtual bool startCrashHandler() override {
        qCWarning(crash_handler) << "No crash handler available.";
        return false;
    }

    virtual void setCrashReportingEnabled([[maybe_unused]] bool value) override {
        // This space intentionally left blank.
    }

    virtual void sendLogMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg) override {
        // This space intentionally left blank.
    }

    virtual void setTag(std::string name, std::string value) override {
        // This space intentionally left blank.
    }
};

#endif
