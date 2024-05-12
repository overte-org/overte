#pragma once

#include <QtCore/QObject>
#include <QUuid>

class CryptographyScriptingInterface : public QObject {
    Q_OBJECT
public:
    Q_INVOKABLE static QUuid randomUUID();
};
