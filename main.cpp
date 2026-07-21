#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include "SnipTool.h"
#include "GlobalHotkeyWin.h"
#include <windows.h>
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
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
    QSystemTrayIcon tray;
    tray.setIcon(QIcon(":qt/qml/Inaba/icons/icon.png"));
    tray.setToolTip("Inaba");
    QMenu menu;

    QAction *showAction = menu.addAction("Show Overlay");
    QAction *quitAction = menu.addAction("Quit");

    tray.setContextMenu(&menu);

    QObject::connect(showAction, &QAction::triggered, [&]{
        snipTool->startSnip(window);
    });

    QObject::connect(quitAction, &QAction::triggered, [&]{
        app.quit();
    });

    QObject::connect(&tray, &QSystemTrayIcon::activated,
                     [&](QSystemTrayIcon::ActivationReason reason)
                     {
                         if (reason == QSystemTrayIcon::DoubleClick)
                             snipTool->startSnip(window);
                     });

    tray.show();
    return QApplication::exec();
}
