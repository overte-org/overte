#include "Context.h"
#include <QtCore/QCoreApplication>

vks::Context& vks::Context::get() {
    static Context INSTANCE;
    return INSTANCE;
}
