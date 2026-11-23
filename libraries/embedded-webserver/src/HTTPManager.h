//
//  HTTPManager.h
//  libraries/embedded-webserver/src
//
//  Created by Stephen Birarda on 1/16/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Heavily based on Andrzej Kapolka's original HTTPManager class
//  found from another one of his projects.
//  https://github.com/ey6es/witgap/tree/master/src/cpp/server/http
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_HTTPManager_h
#define hifi_HTTPManager_h

#include <QtNetwork/QTcpServer>
#include <QtCore/QTimer>

class HTTPConnection;
class HTTPSConnection;

class HTTPRequestHandler {
public:
    /// Handles an HTTP request.
    virtual bool handleHTTPRequest(HTTPConnection* connection, const QUrl& url, bool skipSubHandler = false) = 0;
};

/// Handles HTTP connections
class HTTPManager : public QTcpServer, public HTTPRequestHandler {
   Q_OBJECT
public:
    /**
     * Initializes the HTTP manager.
     *
     * @param listenAddress The address to listen on
     * @param port The port to listen on
     * @param documentRoot The root of the HTTP server's document tree
     * @param requestHandler The request handler to use for incoming requests
     *
     * @note If all the arguments are provided, the server will start immediately. Otherwise, the user must call start() manually.
     * This functionality exists to facilitate path management.
     */
    HTTPManager(const QHostAddress& listenAddress = QHostAddress::Null, quint16 port = 0, const QString& documentRoot = QString(), HTTPRequestHandler* requestHandler = nullptr);

    bool handleHTTPRequest(HTTPConnection* connection, const QUrl& url, bool skipSubHandler = false) override;

    /**
     * @brief Start the HTTP server and begin listening for incoming connections.
     *
     * This will start listening, if all the required parameters are set.
     *
     * @return true We're listening
     * @return false Something is still missing, there will be output on the console.
     */
    bool start();

    /**
     * @brief Where the root of the HTTP server's document tree is located.
     *
     * @param documentRoot Path to the root of the HTTP server's content directory
     */
    void setDocumentRoot(const QString &documentRoot) { _documentRoot = documentRoot; }


    /**
     * @brief Get the document root of the HTTP server.
     *
     * @return QString Path to the root of the HTTP server's content directory
     */
    QString documentRoot() const { return _documentRoot; }

    void setPort(quint16 port) { _port = port; }
    quint16 port() const { return _port; }

    void setListenAddress(const QHostAddress& listenAddress) { _listenAddress = listenAddress; }

    QHostAddress listenAddress() const { return _listenAddress; }

    void setRequestHandler(HTTPRequestHandler* requestHandler) { _requestHandler = requestHandler; }
    HTTPRequestHandler* requestHandler() const { return _requestHandler; }


private slots:
    void isTcpServerListening();
    void queuedExit(QString errorMessage);

private:
    bool bindSocket();
    bool canStart();

protected:
    /// Accepts all pending connections
    virtual void incomingConnection(qintptr socketDescriptor) override;
    virtual bool requestHandledByRequestHandler(HTTPConnection* connection, const QUrl& url);

    QHostAddress _listenAddress{QHostAddress::Null};
    QString _documentRoot;
    HTTPRequestHandler* _requestHandler{nullptr};
    QTimer* _isListeningTimer{nullptr};
    quint16 _port;
};

#endif // hifi_HTTPManager_h
