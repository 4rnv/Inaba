#include "SnipTool.h"
#include <QGuiApplication>
#include <QScreen>
#include <QBuffer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

SnipTool::SnipTool(QObject* parent) : QObject(parent) {
}

QRect SnipTool::selectionRect() const { return m_selectionRect; }

void SnipTool::setSelectionRect(const QRect &rect) {
    if (m_selectionRect != rect) {
        m_selectionRect = rect;
        emit selectionRectChanged();
    }
}

void SnipTool::setWaiting(bool waiting) {
    if (m_waiting == waiting)
        return;
    m_waiting = waiting;
    emit waitingForResponseChanged();
}

void SnipTool::startSnip(QQuickWindow* window) {
    m_window = window;
    m_selectionRect = QRect();
    if (m_window) m_window->showFullScreen();
}

QPixmap SnipTool::grabScreenRegion(const QRect &globalRect) {
    QScreen* screen = QGuiApplication::screenAt(globalRect.topLeft());
    if (!screen) screen = QGuiApplication::primaryScreen();
    if (!screen) return QPixmap();
    
    return screen->grabWindow(0,
                              globalRect.x() - screen->geometry().x(),
                              globalRect.y() - screen->geometry().y(),
                              globalRect.width(), globalRect.height());
}

void SnipTool::capture(const QString &query, const QString &connectionURL, const QString &modelName) {
    if (m_selectionRect.isEmpty()) {
        emit error("No selection made");
        return;
    }
    
    QPixmap screenshot = grabScreenRegion(m_selectionRect);
    if (screenshot.isNull()) {
        emit error("Failed to capture screenshot");
        return;
    }
    screenshot.save("capture.png");
    emit snipCompleted(screenshot, query);
    
    m_selectionRect = QRect();
    emit selectionRectChanged();
    sendToBackend(screenshot, query, connectionURL, modelName);
}

void SnipTool::cancelSnip() {
    m_selectionRect = QRect();
    if (m_window) m_window->hide();
}

void SnipTool::sendToBackend(const QPixmap &pixmap, const QString &query, const QString &connectionURL, const QString &modelName) {
    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");
    
    const QString base64Image = QString::fromLatin1(imageData.toBase64());
    
    QJsonObject message;
    message["role"] = "user";
    message["content"] = query.isEmpty() ? "Describe this image in detail." : query;
    message["images"] = QJsonArray{base64Image};
    qDebug() << "received image and query with params: " << connectionURL << modelName;
    QJsonObject root;
    root["model"] = modelName;
    root["messages"] = QJsonArray{message};
    root["stream"] = false;
    
    const QJsonDocument doc(root);
    const QByteArray jsonData = doc.toJson();
    
    QNetworkRequest request{QUrl(connectionURL)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    auto* manager = new QNetworkAccessManager(this);
    QNetworkReply* reply = manager->post(request, jsonData);
    setWaiting(true);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply, manager]() {
        setWaiting(false);
        if (reply->error() == QNetworkReply::NoError) {
            const QByteArray raw = reply->readAll();
            const QJsonDocument responseDoc = QJsonDocument::fromJson(raw);
            QString text;
            if (responseDoc.isObject()) {
                // Should be OpenAI API format
                const QJsonObject obj = responseDoc.object();
                text = obj.value("message").toObject().value("content").toString();
                qDebug() << "Response: " << text;
            }
            if (text.isEmpty())
                text = QString::fromUtf8(raw); // fall back to raw body if shape differs
            emit responseReceived(text);
        } else {
            emit error("AI request failed: " + reply->errorString());
        }
        reply->deleteLater();
        manager->deleteLater();
    });
}
