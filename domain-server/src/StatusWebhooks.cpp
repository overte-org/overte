#include "StatusWebhooks.h"

void sendWebhookMessage(const QString& webookURL, QJsonObject& json) {
    // prep the webook url
    QUrl url(webookURL);

    // Convert the JSON object to a QByteArray
    QJsonDocument json_doc(json);
    QByteArray json_data = json_doc.toJson(QJsonDocument::Compact);

    // Create a QNetworkRequest with the URL
    QNetworkRequest request(url);

    // Set the HTTP headers
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkAccessManager& nam = NetworkAccessManager::getInstance();

    // Send the HTTP request using the QNetworkAccessManager instance
    QNetworkReply* reply = nam.post(request, json_data);

    // Connect the finished() signal of the QNetworkReply to a lambda function
    QObject::connect(reply, &QNetworkReply::finished, [=]() {
        // Handle the HTTP response
        if (reply->error() != QNetworkReply::NoError) {
            qCritical() << "Error sending message to webhook:" << reply->errorString();
        } else {
            qDebug() << "Message sent successfully!";
        }

        // Delete the QNetworkReply object
        reply->deleteLater();
    });
}

void sendDiscordMessage(QString& message) {
    // get the webhook from the secured variable
    QString hook = Setting::Handle<QString>("connectivity/discord_webhook").get();
    // Create a JSON object with the message content and other optional fields
    QJsonObject json;
    json.insert("content", message);

    // Call the sendWebhookMessage function
    sendWebhookMessage(hook, json);
}

// TODO: This function needs to be check below.

void sendMatrixMessage(QString& message, QString& username) {
    // get the webhook from the secured variable
    QString hook = Setting::Handle<QString>("connectivity/discord_webhook").get();
    // Create a JSON object with the message content and other optional fields
    QJsonObject json;
    json.insert("text", message);
    if (!username.isEmpty()) {
        json.insert("username", username);
    }

    // Call the sendWebhookMessage function
    sendWebhookMessage(hook, json);
}
