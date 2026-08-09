

#ifndef hifi_CrashHandlerSentry_h
#define hifi_CrashHandlerSentry_h

#include "CrashHandler.h"
#include <QMutex>

class CrashHandlerSentry : public CrashHandler {
    Q_OBJECT

    public:


    virtual void setTag(std::string name, std::string value) override;

    virtual void setContext(const QString &sectionName, const QString &key, const QVariant &value) override;

    protected:

    virtual bool startCrashHandler() override;

    virtual void setCrashReportingEnabled([[maybe_unused]] bool value) override;

    virtual void sendLogMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg) override;

    private:
        QMutex _contextMutex;
        QVariantMap _context;
};


#endif
