

#ifndef hifi_CrashHandlerSentry_h
#define hifi_CrashHandlerSentry_h

#include "CrashHandler.h"

class CrashHandlerSentry : public CrashHandler {
    Q_OBJECT

    public:




    protected:

    virtual bool startCrashHandler() override;

    virtual void setCrashReportingEnabled([[maybe_unused]] bool value) override;

    virtual void sendLogMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg) override;

    virtual void setCrashAnnotation(std::string name, std::string value) override;
};


#endif
