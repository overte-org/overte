#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include "Context.h"

vks::Context& vks::Context::get() {
    static Context INSTANCE;
    return INSTANCE;
}
