#include "CryptographyScriptingInterface.h"

QUuid CryptographyScriptingInterface::randomUUID() {
    return QUuid::createUuid();
}
