#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include "SnipTool.h"
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQuickWindow::setDefaultAlphaBuffer(true);
    QQmlApplicationEngine engine;
    app.setOrganizationName("NSTC");
    app.setOrganizationDomain("NSTC");
    app.setApplicationName("Inaba");
    SnipTool* snipTool = new SnipTool();
    engine.rootContext()->setContextProperty("snipTool", snipTool); // for wherever snipTool is mentioned in the application (on QML side)
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("Inaba", "Main");
    if (engine.rootObjects().isEmpty())
        return -1;

    // Don't want this, the application should start and  when the user enters the keyboard shortcut it should open the snip (startSnip apparently)
    auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().first());
    if (window)
        snipTool->startSnip(window); // captures nothing, just shows overlay on start

    return QGuiApplication::exec();
}
