#pragma once
#include <QCoreApplication>
class ApiToolApp : public QCoreApplication {
    Q_OBJECT
public:
    ApiToolApp(int argc, char* argv[]);
    ~ApiToolApp();

private:
};