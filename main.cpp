#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include "SnipTool.h"
#include "GlobalHotkeyWin.h"
#include <windows.h>
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

    auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().first());
    if (window)
        window->hide();

    GlobalHotkeyWin hotkey;
    app.installNativeEventFilter(&hotkey);
    QObject::connect(&hotkey,
                     &GlobalHotkeyWin::triggered,
                     [&]() {
                         snipTool->startSnip(window);
                     });
    hotkey.registerHotkey(MOD_CONTROL | MOD_SHIFT | MOD_ALT, 'S');

    return QGuiApplication::exec();
}
