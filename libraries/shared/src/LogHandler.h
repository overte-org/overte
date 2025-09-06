//
//  LogHandler.cpp
//  libraries/shared/src
//
//  Created by Stephen Birarda on 2014-10-28.
//  Migrated from Logging.cpp created on 6/11/13
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_LogHandler_h
#define hifi_LogHandler_h

#include <QObject>
#include <QString>
#include <QtCore5Compat/QRegExp>
#include <QRecursiveMutex>
#include <vector>
#include <memory>

const int VERBOSE_LOG_INTERVAL_SECONDS = 5;

enum LogMsgType {
    LogInfo = QtInfoMsg,
    LogDebug = QtDebugMsg,
    LogWarning = QtWarningMsg,
    LogCritical = QtCriticalMsg,
    LogFatal = QtFatalMsg,
    LogSuppressed = 100
};

///

/**
 * @brief Handles custom message handling and sending of stats/logs to Logstash instance
 *
 */
class LogHandler : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Returns the one instance of the LogHandler object
     *
     * @return LogHandler&
     */
    static LogHandler& getInstance();

    /**
     * @brief Parse logging options
     *
     * This parses the logging settings in the environment variable, or from the commandline
     *
     * @param options Option list
     * @param paramName Name of the log option, for error reporting.
     * @return true Option list was parsed successfully
     * @return false There was an error
     */
    bool parseOptions(const QString& options, const QString &paramName);

    /**
     * @brief Set the name of the component that's producing log output
     *
     * For instance, "assignment-client", "audio-mixer", etc.
     * Called once before logging begins
     *
     * @param targetName the desired target name to output in logs
     */
    void setTargetName(const QString& targetName);

    /**
     * @brief Set whether to output the process ID
     *
     * @note This has no effect when logging with journald, the PID is always logged
     *
     * @param shouldOutputProcessID Whether to output the PID
     */
    void setShouldOutputProcessID(bool shouldOutputProcessID);

    /**
     * @brief Set whether to output the thread ID
     *
     * @param shouldOutputThreadID
     */
    void setShouldOutputThreadID(bool shouldOutputThreadID);

    /**
     * @brief Set whether to display timestamps with milliseconds
     *
     * @param shouldDisplayMilliseconds
     */
    void setShouldDisplayMilliseconds(bool shouldDisplayMilliseconds);

    /**
     * @brief Set whether to use Journald, if it's available
     *
     * @param shouldUseJournald Whether to use journald
     */
    void setShouldUseJournald(bool shouldUseJournald);

    /**
     * @brief Whether Journald is available on this version/system.
     *
     * Support is available depending on compile options and only on Linux.
     *
     * @return true Journald is available
     * @return false Journald is not available
     */
    bool isJournaldAvailable() const;

    /**
     * @brief Process a log message
     *
     * This writes it to a file, logs it to the console, or sends it to journald.
     *
     * @param type  Log message type
     * @param context Context of the log message (source file, line, function)
     * @param message Log message
     * @return QString The log message's text with added severity and timestamp
     */
    QString printMessage(LogMsgType type, const QMessageLogContext& context, const QString &message);

    /**
     * @brief A qtMessageHandler that can be hooked up to a target that links to Qt
     *
     * Prints various process, message type, and time information
     *
     * @param type  Log message type
     * @param context Context of the log message (source file, line, function)
     * @param message Log message
     */
    static void verboseMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString &message);

    int newRepeatedMessageID();
    void printRepeatedMessage(int messageID, LogMsgType type, const QMessageLogContext& context, const QString &message);

    void setupRepeatedMessageFlusher();

    /**
     * @brief Break when a message that contains the specified string is logged
     *
     * This is a function intended to be invoked from inside a debugger. It should be of help when it's hard to put a breakpoint
     * on the generating line because it comes from inlined code, or the interesting text is generated at runtime.
     *
     * Example usage:
     *
     * @code {.cpp}
     * LogHandler::breakOnMessage("No instance available for");
     * @endcode
     *
     * Then the debugger should be triggered as soon as a message containing that string is logged. Backtracking
     * through the call stack should lead back to the source.
     *
     * @note Support for creating a breakpoint in software is compiler and OS specific. If there's no support for
     * creating a breakpoint on the current compiler/OS, then an abort will be triggered instead.
     *
     * @param str Text to match
     */
    static void breakOnMessage(const char *str);
private:
    LogHandler();
    ~LogHandler() = default;

    void flushRepeatedMessages();

    QString _targetName;
    bool _shouldOutputProcessID { false };
    bool _shouldOutputThreadID { false };
    bool _shouldDisplayMilliseconds { false };
    bool _useColor { false };
    bool _keepRepeats { false };
    bool _useJournald { false };

    QString _previousMessage;
    int _repeatCount { 0 };


    int _currentMessageID { 0 };
    struct RepeatedMessageRecord {
        int repeatCount;
        QString repeatString;
    };
    std::vector<RepeatedMessageRecord> _repeatedMessageRecords;

    QStringList _breakMessages;
    static QRecursiveMutex _mutex;
};

#define HIFI_FCDEBUG(category, message) \
    do { \
        if (category.isDebugEnabled()) { \
            static int repeatedMessageID_ = LogHandler::getInstance().newRepeatedMessageID(); \
            QString logString_; \
            QDebug debugStringReceiver_(&logString_); \
            debugStringReceiver_ << message; \
            LogHandler::getInstance().printRepeatedMessage(repeatedMessageID_, LogDebug, QMessageLogContext(__FILE__, \
                __LINE__, __func__, category().categoryName()), logString_); \
        } \
    } while (false)

#define HIFI_FDEBUG(message) HIFI_FCDEBUG((*QLoggingCategory::defaultCategory()), message)

#define HIFI_FCDEBUG_ID(category, messageID, message) \
    do { \
        if (category.isDebugEnabled()) { \
            QString logString_; \
            QDebug debugStringReceiver_(&logString_); \
            debugStringReceiver_ << message; \
            LogHandler::getInstance().printRepeatedMessage(messageID, LogDebug, QMessageLogContext(__FILE__, \
                __LINE__, __func__, category().categoryName()), logString_); \
        } \
    } while (false)

#define HIFI_FDEBUG_ID(messageID, message) HIFI_FCDEBUG_ID((*QLoggingCategory::defaultCategory()), messageID, message)

#endif // hifi_LogHandler_h
