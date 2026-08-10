
#include <QObject>
#include "CrashHandler.h"


#ifndef overte_CrashHandlerScriptingInterface_h
#define overte_CrashHandlerScriptingInterface_h

/*@jsdoc
 * <p>The <code>CrashHandler</code> provides access to the Crash Handler's state.</p>
 * @namespace CrashHandler
  */

class CrashHandlerScriptingInterface : public QObject {
    Q_OBJECT
    public:

        Q_PROPERTY(QString supportTag READ getSupportTag CONSTANT);
        Q_PROPERTY(QString crashHandlerName READ getCrashHandlerName CONSTANT);
        Q_PROPERTY(bool crashReportingEnabled READ isCrashReportingEnabled CONSTANT);
        Q_PROPERTY(bool logStreamingEnabled READ isLogStreamingEnabled CONSTANT);
        Q_PROPERTY(bool statsStreamingEnabled READ isStatsStreamingEnabled CONSTANT);



        static CrashHandlerScriptingInterface* getInstance() {
            static CrashHandlerScriptingInterface instance;
            return &instance;
        }



        /*@jsdoc
         * Gets the support tag for the crash handler.
         *
         * The support tag is a randomly generated string that is used to identify crash reports.
         * It is randomly generated on every application start.
         * @function CrashHandler.getSupportTag
         * @returns {string} The support tag.
         */
        QString getSupportTag() {
            auto& crashHandler = CrashHandler::getInstance();
            return crashHandler.getSupportTag();
        }

        /*@jsdoc
         * Returns the name of the crash handler implementation.
         * @function CrashHandler.getCrashHandlerName
         * @returns {string} The name of the crash handler implementation.
         */
         QString getCrashHandlerName() {
            auto& crashHandler = CrashHandler::getInstance();
            return crashHandler.metaObject()->className();
         }


        /*@jsdoc
         * Whether crash reporting is enabled.
         * @function CrashHandler.isCrashReportingEnabled
         * @returns {boolean} <code>true</code> if crash reporting is enabled, <code>false</code> if it isn't.
         */
        bool isCrashReportingEnabled() {
            auto& crashHandler = CrashHandler::getInstance();
            return crashHandler.isEnabled();
        }

        /*@jsdoc
         * Whether log streaming is enabled.
         * @function CrashHandler.isLogStreamingEnabled
         * @returns {boolean} <code>true</code> if log streaming is enabled, <code>false</code> if it isn't.
         */
        bool isLogStreamingEnabled() {
            auto& crashHandler = CrashHandler::getInstance();
            return crashHandler.isLogStreamingEnabled();
        }

        /*@jsdoc
         * Whether stats streaming is enabled.
         * @function CrashHandler.isStatsStreamingEnabled
         * @returns {boolean} <code>true</code> if stats streaming is enabled, <code>false</code> if it isn't.
         */
        bool isStatsStreamingEnabled() {
            auto& crashHandler = CrashHandler::getInstance();
            return crashHandler.isStatsStreamingEnabled();
        }


        virtual ~CrashHandlerScriptingInterface() = default;

    private:

        CrashHandlerScriptingInterface(QObject* parent = nullptr) : QObject(parent) {

        }



};

#endif
