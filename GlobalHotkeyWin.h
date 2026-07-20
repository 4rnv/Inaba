#include <QAbstractNativeEventFilter>
#include <QObject>

// Registers a system-wide hotkey using the Win32 RegisterHotKey API, so the overlay can be summoned even while another app has focus. Emits triggered() when pressed.
class GlobalHotkeyWin : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit GlobalHotkeyWin(QObject *parent = nullptr);
    ~GlobalHotkeyWin() override;
    bool registerHotkey(quint32 modifiers, quint32 vk);
    void unregisterHotkey();

    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

signals:
    void triggered();

private:
    int m_hotkeyId = 1;
    bool m_registered = false;
};