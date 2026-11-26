#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>

#include "ApiToolApp.h"
#include <BuildInfo.h>
#include "SharedUtil.h"

int main(int argc, char* argv[]) {
    setupHifiApplication(BuildInfo::API_TOOL_NAME);

    ApiToolApp app(argc, argv);
    return app.exec();
}