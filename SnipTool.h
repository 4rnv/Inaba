#ifndef SNIPTOOL_H
#define SNIPTOOL_H

#include <QObject>
#include <QRect>
#include <QQuickWindow>
#include <QPixmap>

class SnipTool : public QObject {
    Q_OBJECT
    Q_PROPERTY(QRect selectionRect READ selectionRect WRITE setSelectionRect NOTIFY selectionRectChanged)
    Q_PROPERTY(bool waitingForResponse READ waitingForResponse NOTIFY waitingForResponseChanged)

public:
    explicit SnipTool(QObject *parent = nullptr);

    QRect selectionRect() const;
    Q_INVOKABLE void setSelectionRect(const QRect &rect);
    Q_INVOKABLE void startSnip(QQuickWindow *window);
    Q_INVOKABLE void capture(const QString &query, const QString &connectionURL, const QString &modelName);
    Q_INVOKABLE void cancelSnip();
    bool waitingForResponse() const { return m_waiting; }


signals:
    void selectionRectChanged();
    void waitingForResponseChanged();
    void snipCompleted(const QPixmap &screenshot, const QString &query);
    void responseReceived(const QString &text);
    void error(const QString &message);

private:
    QRect m_selectionRect;
    QQuickWindow *m_window = nullptr;
    bool m_waiting = false;

    void setWaiting(bool waiting);
    QPixmap grabScreenRegion(const QRect &globalRect);
    void sendToBackend(const QPixmap &pixmap, const QString &query, const QString &connectionURL, const QString &modelName);
};

#endif
