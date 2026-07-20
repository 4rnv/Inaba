#include "GlobalHotkeyWin.h"
#include <windows.h>
#include <QDebug>

GlobalHotkeyWin::GlobalHotkeyWin(QObject *parent)
    : QObject(parent)
{
}

GlobalHotkeyWin::~GlobalHotkeyWin()
{
    unregisterHotkey();
}

// modifiers: combination of MOD_CONTROL, MOD_SHIFT, MOD_ALT, MOD_WIN (Win32 constants)
// vk: virtual-key code, e.g. 'S'
bool GlobalHotkeyWin::registerHotkey(quint32 modifiers, quint32 vk)
{
    if (!RegisterHotKey(nullptr, m_hotkeyId, modifiers | MOD_NOREPEAT, vk)) {
        qWarning() << "Failed to register global hotkey, error:" << GetLastError();
        return false;
    }
    m_registered = true;
    return true;
}

void GlobalHotkeyWin::unregisterHotkey()
{
    if (m_registered) {
        UnregisterHotKey(nullptr, m_hotkeyId);
        m_registered = false;
    }
}

bool GlobalHotkeyWin::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
{
    Q_UNUSED(result);
    if (eventType != "windows_generic_MSG")
        return false;

    MSG *msg = static_cast<MSG *>(message);
    if (msg->message == WM_HOTKEY && msg->wParam == static_cast<WPARAM>(m_hotkeyId)) {
        emit triggered();
        return true;
    }
    return false;
}