import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import QtQuick.Effects

Window {
    property point startPos: Qt.point(0, 0)
    property bool selecting: false
    id: window
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.SplashScreen
    visible: true
    minimumWidth: Screen.width
    minimumHeight: Screen.height
    Settings {
        id: appSettings
        property bool autoSend: false
        property int radius: 0
        property string modelName: "minicpm-v4.6:1b"
        property string connectionURL: "http://localhost:11434/api/chat"
        property color accent: "#f272a9"
        property string defaultPrompt: "Describe the image in detail."
    }

    Shortcut {
        sequence: "Escape"
        onActivated: {
            snipTool.setSelectionRect(Qt.rect(startPos.x, startPos.y, 0, 0))
            snipTool.cancelSnip()
            selecting = false
        }
    }

    Item {
        id: overlay
        anchors.fill: parent

        property bool hasSelection: snipTool.selectionRect.width > 0 && snipTool.selectionRect.height > 0
        property rect sel: snipTool.selectionRect

        // Initial screen dimming overlay
        Rectangle {
            anchors.fill: parent
            color: Qt.rgba(0,0,0,0.3)
            visible: !overlay.hasSelection
            border.width: 20
            border.color: Qt.rgba(appSettings.accent.r, appSettings.accent.g, appSettings.accent.b, 0.1)
        }

        // Overlays after selection
        Rectangle { // top strip
            visible: overlay.hasSelection
            color: Qt.rgba(0,0,0,0.3)
            x: 0; y: 0
            width: parent.width; height: overlay.sel.y
        }
        Rectangle { // bottom strip
            visible: overlay.hasSelection
            color: Qt.rgba(0,0,0,0.3)
            x: 0; y: overlay.sel.y + overlay.sel.height
            width: parent.width; height: parent.height - (overlay.sel.y + overlay.sel.height)
        }
        Rectangle { // left strip
            visible: overlay.hasSelection
            color: Qt.rgba(0,0,0,0.3)
            x: 0; y: overlay.sel.y
            width: overlay.sel.x; height: overlay.sel.height
        }
        Rectangle { // right strip
            visible: overlay.hasSelection
            color: Qt.rgba(0,0,0,0.3)
            x: overlay.sel.x + overlay.sel.width; y: overlay.sel.y
            width: parent.width - (overlay.sel.x + overlay.sel.width); height: overlay.sel.height
        }

        Rectangle {
            id: selectionRectVisual
            visible: overlay.hasSelection
            x: overlay.sel.x
            y: overlay.sel.y
            width: overlay.sel.width
            height: overlay.sel.height
            border.color: appSettings.accent
            border.width: 1
            color: "transparent"
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.CrossCursor

            onPressed: (mouse) => {
                startPos = Qt.point(mouse.x, mouse.y)
                selecting = true
                snipTool.setSelectionRect(Qt.rect(startPos.x, startPos.y, 0, 0))
            }

            onPositionChanged: (mouse) => {
                if (selecting) {
                    var x = Math.min(startPos.x, mouse.x)
                    var y = Math.min(startPos.y, mouse.y)
                    var w = Math.abs(mouse.x - startPos.x)
                    var h = Math.abs(mouse.y - startPos.y)
                    snipTool.setSelectionRect(Qt.rect(x, y, w, h))
                }
            }

            onReleased: (mouse) => {
                selecting = false
                if(appSettings.autoSend) {
                    snipTool.capture(appSettings.defaultPrompt, appSettings.connectionURL, appSettings.modelName)
                }
            }
        }
    }
    Rectangle {
        id: textareawrapper
        width: Screen.width/3
        height: Screen.height/6
        color: Qt.rgba(0.1, 0.1, 0.1, 0.85)
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 40
        anchors.horizontalCenter: parent.horizontalCenter
        border.width: 2
        border.color: textarea1.activeFocus ? appSettings.accent : Qt.rgba(1,1,1,0.15)
        radius: appSettings.radius
        Behavior on border.color { ColorAnimation { duration: 100 } } // 100ms

        TextArea {
            id: textarea1
            anchors.left: parent.left
            anchors.right: sendButton.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            anchors.topMargin: 8
            anchors.bottomMargin: 8
            font.pointSize: 14
            placeholderText: "Enter query… (Enter to send, Shift+Enter for newline)"
            placeholderTextColor: "#888888"
            color: "white"
            wrapMode: TextArea.Wrap
            selectByMouse: true
            Keys.onPressed: (event) => {
                if ((event.key === Qt.Key_Return || event.key === Qt.Key_Enter) // Return is the actual Enter key, Enter is the Numpad Enter
                        && !(event.modifiers & Qt.ShiftModifier)) {
                    event.accepted = true
                    if (textarea1.text.trim().length > 0) {
                        snipTool.capture(textarea1.text, appSettings.connectionURL, appSettings.modelName)
                        textarea1.text = ""
                    }
                }
            }
        }
        Button {
            id: sendButton
            width: parent.width/8
            height: parent.height
            icon.source: "icons/send.svg"
            icon.width: 48
            icon.height: 48
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            onClicked: {
                snipTool.capture(textarea1.text, appSettings.connectionURL, appSettings.modelName)
                textarea1.text = ""
            }
            background: Rectangle {
                radius: appSettings.radius
                width: parent.width
                height: parent.height
                color: sendButton.hovered ? Qt.rgba(appSettings.accent.r,appSettings.accent.g,appSettings.accent.b,0.7) : "transparent"
            }
        }
    }

    Button {
        id: settingsButton
        width: 48
        height: 48
        icon.source: "icons/settings.svg"
        icon.width: 48
        icon.height: 48
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 32
        onClicked: settingsPopup.open()
        background: Rectangle {
            radius: appSettings.radius / 2
            width: 48
            height: 48
            color: settingsButton.hovered ? Qt.rgba(appSettings.accent.r,appSettings.accent.g,appSettings.accent.b,0.7) : Qt.rgba(0,0,0,0.7)
        }
    }

    Popup {
        id: settingsPopup
        modal: true
        focus: true
        anchors.centerIn: parent
        width: Screen.width / 2
        padding: 20
        background: Rectangle {
            radius: appSettings.radius
            color: Qt.rgba(0,0,0,0.7)
        }

        ColumnLayout {
            width: parent.width
            spacing: 12

            Text { text: "Settings"; color: "white"; font.pointSize: 16; font.bold: true }

            Text { text: "Connection URL"; color: "#c0c0c0"; font.pointSize: 12 }
            TextField {
                Layout.fillWidth: true
                text: appSettings.connectionURL
                onEditingFinished: appSettings.connectionURL = text
            }

            Text { text: "Model name"; color: "#c0c0c0"; font.pointSize: 12 }
            TextField {
                Layout.fillWidth: true
                text: appSettings.modelName
                onEditingFinished: appSettings.modelName = text
            }

            Text { text: "Default Prompt"; color: "#c0c0c0"; font.pointSize: 12 }
            TextField {
                Layout.fillWidth: true
                text: appSettings.defaultPrompt
                onEditingFinished: appSettings.defaultPrompt = text
            }

            RowLayout {
                Layout.fillWidth: true
                Text { text: "Auto Send"; color: "#c0c0c0"; Layout.fillWidth: true; font.pointSize: 12 }
                Switch {
                    checked: appSettings.autoSend
                    onCheckedChanged: appSettings.autoSend = checked
                }
            }

            Text { text: "Radius"; color: "#c0c0c0"; font.pointSize: 12 }
            SpinBox {
                id: radiusSpinBox
                Layout.fillWidth: true
                from: 0
                to: 20
                value: appSettings.radius
                onValueModified: appSettings.radius = value
            }

            Text { text: "Accent color"; color: "#c0c0c0"; font.pointSize: 12 }
            Row {
                spacing: 12
                Repeater {
                    model: ["#f272a9", "#4f8fdc", "#d6a52c", "#9a6fe0", "#ba1c4b", "#88e843"]
                    delegate: Rectangle {
                        width: 32; height: 32; radius: appSettings.radius
                        color: modelData // modelData is the model field in the Repeater
                        border.width: appSettings.accent === Qt.color(modelData) ? 2 : 0
                        border.color: "white"
                        RadioButton {
                            anchors.fill: parent
                            opacity: 0
                            checked: appSettings.accent === modelData
                            onClicked: appSettings.accent = modelData
                        }
                    }
                }
            }

            Button {
                id: settingsCloseButton
                text: "Close"
                font.pointSize: 12
                Layout.alignment: Qt.AlignRight
                onClicked: settingsPopup.close()
                Rectangle {
                    id: settingCloseButtonRectangle
                    border.color: appSettings.accent
                    border.width: 1
                }
            }
        }
    }


    Button {
        id: chatButton
        width: 48
        height: 48
        icon.source: "icons/chat.svg"
        icon.width: 48
        icon.height: 48
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 32
        onClicked: chatPanel.open = true
        background: Rectangle {
            radius: appSettings.radius / 2
            width: 48
            height: 48
            color: chatButton.hovered ? Qt.rgba(appSettings.accent.r,appSettings.accent.g,appSettings.accent.b,0.7) : Qt.rgba(0,0,0,0.7)
        }
    }

    // Chat Panel
    ListModel { id: chatModel }

    Rectangle {
        id: chatPanel
        width: Math.min(300, parent.width / 4)
        height: parent.height
        anchors.top: parent.top
        x: parent.width
        color: Qt.rgba(0, 0, 0, 0.7)

        property bool open: false
        onOpenChanged: x = open ? parent.width - width : parent.width

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 10

            RowLayout {
                Layout.fillWidth: true
                Text { text: "Chat"; color: "white"; font.pointSize: 14; font.bold: true; Layout.fillWidth: true }
                Button {
                    flat: true
                    onClicked: chatPanel.open = false
                    contentItem: Text { text: "✕"; color: "white"; horizontalAlignment: Text.AlignHCenter }
                    background: Rectangle { color: "transparent" }
                }
            }

            ListView {
                id: chatList
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: chatModel
                spacing: 10
                clip: true
                onCountChanged: positionViewAtEnd()

                delegate: Rectangle {
                    width: chatList.width
                    radius: appSettings.radius / 2
                    color: role === "user" ? Qt.rgba(appSettings.accent.r, appSettings.accent.g, appSettings.accent.b, 0.4)
                                            : Qt.rgba(1,1,1, 0.2)
                    height: msgText.implicitHeight + 20
                    Text {
                        id: msgText
                        text: content
                        color: "white"
                        wrapMode: Text.Wrap
                        width: parent.width - 20
                        anchors.centerIn: parent
                    }
                }
            }

            BusyIndicator {
                id: replyIndicator
                Layout.alignment: Qt.AlignHCenter
                running: snipTool.waitingForResponse
                visible: running
            }
        }
    }

    Connections {
        target: snipTool
        function onSnipCompleted(screenshot, query) {
            if (query.trim().length > 0)
                chatModel.append({ role: "user", content: query })
            chatPanel.open = true
        }
        function onResponseReceived(text) {
            chatModel.append({ role: "assistant", content: text })
        }
        function onError(message) {
                toast.show(message)
        }
    }

    Rectangle {
        id: toast
        property string message
        width: Math.min(400, parent.width * 0.6)
        height: 60
        color: Qt.rgba(0.2, 0.2, 0.2, 0.85)
        radius: appSettings.radius
        anchors.top: parent.top
        anchors.topMargin: 50
        anchors.horizontalCenter: parent.horizontalCenter
        visible: false
        border.color: "red"
        border.width: 1
        Text {
            anchors.centerIn: parent
            text: toast.message
            color: "white"
            font.pointSize: 14
        }
        Timer { id: toastTimer; interval: 3000; onTriggered: toast.visible = false }
        function show(msg) {
            toast.message = msg
            visible = true
            toastTimer.restart()
        }
    }
}
