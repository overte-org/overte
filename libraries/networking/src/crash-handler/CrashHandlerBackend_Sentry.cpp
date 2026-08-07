//
//  CrashHandler_Sentry.cpp
//  interface/src
//
//  Created by Vadim Troschinsky on 05/08/2026.
//  Copyright 2018 High Fidelity, Inc.
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#define HAS_SENTRY 1

#if HAS_SENTRY


#include "sentry.h"
#include "CrashHandler.h"

Q_LOGGING_CATEGORY(crash_handler, "overte.crash_handler")

#include <assert.h>

#include <vector>
#include <string>

#include <QtCore/QAtomicInteger>
#include <QtCore/QDeadlineTimer>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtCore/QString>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++14-extensions"
#endif


#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#include <BuildInfo.h>
#include "../FingerprintUtils.h"
#include "../UserActivityLogger.h"
#include <UUID.h>



/**
 * @brief Sentry implementation
 *
 * Sentry is a crash reporting and metrics system.
 *
 * Documentation for the API is at https://docs.sentry.io/platforms/native/usage/
 *
 * For reporting, we use the sentry DSN, not the minidump endpoint.
 *
 *
 */

static const std::string BACKTRACE_URL{ OVERTE_BACKTRACE_URL };
static const std::string BACKTRACE_TOKEN{ OVERTE_BACKTRACE_TOKEN };

std::string custom_backtrace_url;
std::string custom_backtrace_token;


// ------------------------------------------------------------------------------------------------
// SpinLock - a lock that can timeout attempting to lock a block of code, and is in a busy-wait cycle while trying to acquire
//   note that this code will malfunction if you attempt to grab a lock while already holding it

class SpinLock {
public:
    SpinLock();
    void lock();
    bool lock(int msecs);
    void unlock();

private:
    QAtomicInteger<int> _lock{ 0 };

    Q_DISABLE_COPY(SpinLock)
};

class SpinLockLocker {
public:
    SpinLockLocker(SpinLock& lock, int msecs = -1);
    ~SpinLockLocker();
    bool isLocked() const;
    void unlock();
    bool relock(int msecs = -1);

private:
    SpinLock* _lock;
    bool _isLocked;

    Q_DISABLE_COPY(SpinLockLocker)
};

SpinLock::SpinLock() {
}

void SpinLock::lock() {
    while (!_lock.testAndSetAcquire(0, 1))
        ;
}

bool SpinLock::lock(int msecs) {
    QDeadlineTimer deadline(msecs);
    for (;;) {
        if (_lock.testAndSetAcquire(0, 1)) {
            return true;
        }
        if (deadline.hasExpired()) {
            return false;
        }
    }
}

void SpinLock::unlock() {
    _lock.storeRelease(0);
}

SpinLockLocker::SpinLockLocker(SpinLock& lock, int msecs /* = -1 */ ) : _lock(&lock) {
    _isLocked = _lock->lock(msecs);
}

SpinLockLocker::~SpinLockLocker() {
    if (_isLocked) {
        _lock->unlock();
    }
}

bool SpinLockLocker::isLocked() const {
    return _isLocked;
}

void SpinLockLocker::unlock() {
    if (_isLocked) {
        _lock->unlock();
        _isLocked = false;
    }
}

bool SpinLockLocker::relock(int msecs /* = -1 */ ) {
    if (!_isLocked) {
        _isLocked = _lock->lock(msecs);
    }
    return _isLocked;
}

// ------------------------------------------------------------------------------------------------



#if defined(Q_OS_WIN)
static const QString CRASHPAD_HANDLER_NAME{ "crashpad_handler.exe" };
#else
static const QString CRASHPAD_HANDLER_NAME{ "crashpad_handler" };
#endif

#ifdef Q_OS_LINUX
#include <unistd.h>
#endif

#ifdef Q_OS_WIN
// ------------------------------------------------------------------------------------------------
// The area within this #ifdef is specific to the Microsoft C++ compiler

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QLogging.h>
#include <QtCore/QTimer>

#include <Windows.h>
#include <typeinfo>

static constexpr DWORD STATUS_MSVC_CPP_EXCEPTION = 0xE06D7363;
static constexpr ULONG_PTR MSVC_CPP_EXCEPTION_SIGNATURE = 0x19930520;
static constexpr int ANNOTATION_LOCK_WEAK_ATTEMPT = 5000;  // attempt to lock the annotations list, but give up if it takes more than 5 seconds

LPTOP_LEVEL_EXCEPTION_FILTER gl_crashpadUnhandledExceptionFilter = nullptr;
QTimer unhandledExceptionTimer;  // checks occasionally in case loading an external DLL reset the unhandled exception pointer

void fatalCxxException(PEXCEPTION_POINTERS pExceptionInfo);  // extracts type information from a thrown C++ exception
LONG WINAPI firstChanceExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo);  // called on any thrown exception (whether or not it's caught)
LONG WINAPI unhandledExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo);  // called on any exception without a corresponding catch

static LONG WINAPI firstChanceExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo) {
    // we're catching these exceptions on first-chance as the system state is corrupted at this point and they may not survive the exception handling mechanism
    if (client && (pExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_HEAP_CORRUPTION ||
                   pExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_STACK_BUFFER_OVERRUN)) {
        client->DumpAndCrash(pExceptionInfo);
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

static LONG WINAPI unhandledExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo) {
    if (client && pExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_MSVC_CPP_EXCEPTION) {
        fatalCxxException(pExceptionInfo);
        client->DumpAndCrash(pExceptionInfo);
    }

    if (gl_crashpadUnhandledExceptionFilter != nullptr) {
        return gl_crashpadUnhandledExceptionFilter(pExceptionInfo);
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

// The following structures are modified versions of structs defined inplicitly by the Microsoft C++ compiler
// as described at http://www.geoffchappell.com/studies/msvc/language/predefined/
// They are redefined here as the definitions the compiler gives only work in 32-bit contexts and are out-of-sync
// with the internal structures when operating in a 64-bit environment
// as discovered and described here: https://stackoverflow.com/questions/39113168/c-rtti-in-a-windows-64-bit-vectoredexceptionhandler-ms-visual-studio-2015

#pragma pack(push, ehdata, 4)

struct PMD_internal {  // internal name: _PMD (no changes, so could in theory just use the original)
    int mdisp;
    int pdisp;
    int vdisp;
};

struct ThrowInfo_internal {  // internal name: _ThrowInfo (changed all pointers into __int32)
    __int32 attributes;
    __int32 pmfnUnwind;           // 32-bit RVA
    __int32 pForwardCompat;       // 32-bit RVA
    __int32 pCatchableTypeArray;  // 32-bit RVA
};

struct CatchableType_internal {  // internal name: _CatchableType (changed all pointers into __int32)
    __int32 properties;
    __int32 pType;               // 32-bit RVA
    PMD_internal thisDisplacement;
    __int32 sizeOrOffset;
    __int32 copyFunction;        // 32-bit RVA
};

#pragma warning(disable : 4200)
struct CatchableTypeArray_internal {  // internal name: _CatchableTypeArray (changed all pointers into __int32)
    int nCatchableTypes;
    __int32 arrayOfCatchableTypes[0];  // 32-bit RVA
};
#pragma warning(default : 4200)

#pragma pack(pop, ehdata)

// everything inside this function is extremely undocumented, attempting to extract
// the underlying C++ exception type (or at least its name) before throwing the whole
// mess at crashpad
// Some links describing how C++ exception handling works in an SEH context
// (since C++ exceptions are a figment of the Microsoft compiler):
//  - https://www.codeproject.com/Articles/175482/Compiler-Internals-How-Try-Catch-Throw-are-Interpr
//  - https://stackoverflow.com/questions/21888076/how-to-find-the-context-record-for-user-mode-exception-on-x64

static void fatalCxxException(PEXCEPTION_POINTERS pExceptionInfo) {
    SpinLockLocker guard(crashpadAnnotationsProtect, ANNOTATION_LOCK_WEAK_ATTEMPT);
    if (!guard.isLocked()) {
        return;
    }

    PEXCEPTION_RECORD ExceptionRecord = pExceptionInfo->ExceptionRecord;
    /*
    Exception arguments for Microsoft C++ exceptions:
    [0] signature  - magic number
    [1] void*      - variable that is being thrown
    [2] ThrowInfo* - description of the variable that was thrown
    [3] HMODULE    - (64-bit only) base address that all 32bit pointers are added to
    */

    if (ExceptionRecord->NumberParameters != 4 || ExceptionRecord->ExceptionInformation[0] != MSVC_CPP_EXCEPTION_SIGNATURE) {
        // doesn't match expected parameter counts or magic numbers
        return;
    }

    // get the ThrowInfo struct from the exception arguments
    ThrowInfo_internal* pThrowInfo = reinterpret_cast<ThrowInfo_internal*>(ExceptionRecord->ExceptionInformation[2]);
    ULONG_PTR moduleBase = ExceptionRecord->ExceptionInformation[3];
    if (moduleBase == 0 || pThrowInfo == NULL) {
        return;  // broken assumption
    }

    // get the CatchableTypeArray* struct from ThrowInfo
    if (pThrowInfo->pCatchableTypeArray == 0) {
        return;  // broken assumption
    }
    CatchableTypeArray_internal* pCatchableTypeArray =
        reinterpret_cast<CatchableTypeArray_internal*>(moduleBase + pThrowInfo->pCatchableTypeArray);
    if (pCatchableTypeArray->nCatchableTypes == 0 || pCatchableTypeArray->arrayOfCatchableTypes[0] == 0) {
        return;  // broken assumption
    }

    // get the CatchableType struct for the actual exception type from CatchableTypeArray
    CatchableType_internal* pCatchableType =
        reinterpret_cast<CatchableType_internal*>(moduleBase + pCatchableTypeArray->arrayOfCatchableTypes[0]);
    if (pCatchableType->pType == 0) {
        return;  // broken assumption
    }
    const std::type_info* type = reinterpret_cast<std::type_info*>(moduleBase + pCatchableType->pType);

    crashpadAnnotations->SetKeyValue("thrownObject", type->name());

    // After annotating the name of the actual object type, go through the other entries in CatcahleTypeArray and itemize the list of possible
    // catch() commands that could have caught this so we can find the list of its superclasses
    QString compatibleObjects;
    for (int catchTypeIdx = 1; catchTypeIdx < pCatchableTypeArray->nCatchableTypes; catchTypeIdx++) {
        CatchableType_internal* pCatchableSuperclassType =
            reinterpret_cast<CatchableType_internal*>(moduleBase + pCatchableTypeArray->arrayOfCatchableTypes[catchTypeIdx]);
        if (pCatchableSuperclassType->pType == 0) {
            return;  // broken assumption
        }
        const std::type_info* superclassType = reinterpret_cast<std::type_info*>(moduleBase + pCatchableSuperclassType->pType);

        if (!compatibleObjects.isEmpty()) {
            compatibleObjects += ", ";
        }
        compatibleObjects += superclassType->name();
    }
    crashpadAnnotations->SetKeyValue("thrownObjectLike", compatibleObjects.toStdString());
}

void checkUnhandledExceptionHook() {
    LPTOP_LEVEL_EXCEPTION_FILTER prevExceptionFilter = SetUnhandledExceptionFilter(unhandledExceptionHandler);
    if (prevExceptionFilter != unhandledExceptionHandler) {
        qWarning() << QString("Restored unhandled exception filter (which had been changed to %1)")
                          .arg(reinterpret_cast<ULONG_PTR>(prevExceptionFilter), 16, 16, QChar('0'));
    }
}

// End of code specific to the Microsoft C++ compiler
// ------------------------------------------------------------------------------------------------
#endif  // Q_OS_WIN

// Locate the full path to the binary's directory
static QString findBinaryDir() {
    // Normally we'd just use QCoreApplication::applicationDirPath(), but we can't.
    // That function needs the QApplication to be created first, and Crashpad is initialized as early as possible,
    // which is well before QApplication, so that function throws out a warning and returns ".".
    //
    // So we must do things the hard way here. In particular this is needed to correctly handle things in AppImage
    // on Linux. On Windows and MacOS falling back to argv[0] should be fine.

#ifdef Q_OS_LINUX
    // Find outselves by looking at /proc/<PID>/exe
    pid_t ourPid = getpid();
    QString exeLink = QString("/proc/%1/exe").arg(ourPid);
    qCDebug(crash_handler) << "Looking at" << exeLink;

    QFileInfo exeLinkInfo(exeLink);
    if (exeLinkInfo.isSymLink()) {
        QFileInfo exeInfo(exeLinkInfo.symLinkTarget());
        qCDebug(crash_handler) << "exe symlink points at" << exeInfo;
        return exeInfo.absoluteDir().absolutePath();
    } else {
        qCWarning(crash_handler) << exeLink << "isn't a symlink. /proc not mounted?";
    }

#endif

    return QString();
}

bool startCrashHandler(std::string appPath, std::string crashURL, std::string crashToken) {
    if (crashURL.empty()) {
        crashURL = BACKTRACE_URL;
    }

    if (crashToken.empty()) {
        crashToken = BACKTRACE_TOKEN;
    }

    if (crashURL.empty() || crashToken.empty()) {
        qCCritical(crash_handler) << "Backtrace URL or token not set, crash handler disabled.";
        return false;
    }

    sentry_options_t *options = sentry_options_new();
    sentry_options_set_dsn(options, crashURL.c_str());
    sentry_options_set_release(options, crashToken.c_str());
    sentry_options_set_debug(options, 1); // Debug for Sentry Native SDK itself
    sentry_options_set_enable_logs(options, 1);
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

void setCrashAnnotation(std::string name, std::string value) {

}

void startCrashHookMonitor(QCoreApplication* app) {
#ifdef Q_OS_WIN
    // create a timer that checks to see if our exception handler has been reset.  This may occur when a new CRT
    // is initialized, which could happen any time a DLL not compiled with the same compiler is loaded.
    // It would be nice if this were replaced with a more intelligent response; this fires once a minute which
    // may be too much (extra code running) and too little (leaving up to a 1min gap after the hook is broken)
    checkUnhandledExceptionHook();

    unhandledExceptionTimer.moveToThread(app->thread());
    QObject::connect(&unhandledExceptionTimer, &QTimer::timeout, checkUnhandledExceptionHook);
    unhandledExceptionTimer.start(60000);
#endif  // Q_OS_WIN
}


void CrashHandler::logMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg) {

    // Some of the logging is very spammy and Sentry seems to have a problem with shutting down
    // in that case.
    {
        std::lock_guard<std::mutex> lock(_logMutex);

        if (msg == _previousMessage) {
            _repeatCount++;
            return;
        }

        if (_repeatCount > 0) {
            sentry_log_warn("Previous log message repeated %i times", _repeatCount);
        }

        _previousMessage = msg;
        _repeatCount = 0;
    }


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

#endif  // HAS_SENTRY
