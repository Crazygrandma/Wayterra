import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Quickshell
import Quickshell.Io

ShellRoot {
    PanelWindow {
        id: welcomePanel
        implicitWidth: 600
        implicitHeight: 360
        anchors {
            left: true
            right: true
            top: true
            bottom: true
        }
        color: "transparent"
        property string currentPage: "main"
        property string selectedModeTitle: ""
        property string selectedModeDescription: ""
        property string configScriptPath: "/usr/lib/wayterra/apply-mode.sh"

        Process {
            id: configWriter
            workingDirectory: "/home/moviemakerhd/.config/wayterra"
        }

        MouseArea {
            anchors.fill: parent
            z: 0

            onClicked: Qt.quit()
        }

        Item {
            id: panelContent
            width: 600
            height: 360
            anchors.centerIn: parent
            z: 1

        FontLoader {
            id: terrariaFont
            source: "assets/fonts/andybold.ttf"
        }

        Rectangle {
            anchors.fill: panelContent
            radius: 8

            gradient: Gradient {
                GradientStop { position: 0.0; color: "#3c78c9" }
                GradientStop { position: 1.0; color: "#1d3f73" }
            }

            opacity: 0.85
            border.color: "#9bd2ff"
            border.width: 2
        }

Loader {
    id: pageLoader

    anchors.fill: panelContent
    anchors.margins: 30

    sourceComponent: welcomePanel.currentPage === "main"
        ? mainPage
        : (welcomePanel.currentPage === "difficulty" ? difficultyPage : modeGuidePage)

    opacity: 1
    y: 0

    ParallelAnimation {
        id: pageInAnimation
        NumberAnimation { target: pageLoader; property: "opacity"; from: 0; to: 1; duration: 220; easing.type: Easing.OutCubic }
        NumberAnimation { target: pageLoader; property: "y"; from: 10; to: 0; duration: 220; easing.type: Easing.OutCubic }
    }

    onSourceComponentChanged: {
        opacity = 0
        y = 10
        pageInAnimation.restart()
    }
}

        Component {
            id: mainPage

            ColumnLayout {
                spacing: 20
                anchors.fill: parent

                Text {
                    text: "Welcome Adventurer"
                    font.family: terrariaFont.name
                    font.pixelSize: 36
                    color: "white"
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                }

                Text {
                    text: "Configure your world"
                    font.family: terrariaFont.name
                    font.pixelSize: 20
                    color: "#d7ecff"
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                }

                Item { Layout.fillHeight: true }

                Button {
                    id: startSetupButton
                    text: "Start Setup"
                    font.family: terrariaFont.name
                    Layout.alignment: Qt.AlignHCenter
                    hoverEnabled: true

                    background: Rectangle {
                        radius: 6
                        color: startSetupButton.hovered ? "#bfe6ff" : "#a7d8ff"
                        border.color: "#e8f7ff"
                        border.width: 2
                    }

                    contentItem: Text {
                        text: startSetupButton.text
                        font.family: terrariaFont.name
                        font.pixelSize: 18
                        color: "#1d3f73"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: welcomePanel.currentPage = "difficulty"
                }
            }
        }

        Component {
            id: difficultyPage

            ColumnLayout {
                anchors.fill: parent
                spacing: 20

                Text {
                    text: "Choose Your Path"
                    font.family: terrariaFont.name
                    font.pixelSize: 32
                    color: "white"
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 20

                    DifficultyButton {
                        title: "Apprentice"
                        shortDescription: "Single-screen movement."
                        imageSource: "assets/images/apprentice.png"
                    }

                    DifficultyButton {
                        title: "Adventurer"
                        shortDescription: "Explore and build freely."
                        imageSource: "assets/images/adventurer.png"
                    }


                    DifficultyButton {
                        title: "Wizard"
                        shortDescription: "Full system control."
                        imageSource: "assets/images/wizard.png"
                    }
                }

                ColumnLayout {
            Layout.fillWidth: true
            spacing: 6

            Text {
                text: "You can find out more about the modes here:"
                font.family: terrariaFont.name
                font.pixelSize: 16
                color: "#d7ecff"
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
            }

            Text {
                text: "<a href='https://crazygrandma.github.io/Wayterra/'>Wayterra Wiki</a>"
                textFormat: Text.RichText
                font.family: terrariaFont.name
                font.pixelSize: 16
                color: "#9bd2ff"

                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true

                onLinkActivated: function(url) {
                    Qt.openUrlExternally(url)
                }
            }
        }


            }
        }

        Component {
            id: modeGuidePage

            ColumnLayout {
                anchors.fill: parent
                spacing: 16

                Text {
                    text: welcomePanel.selectedModeTitle
                    font.family: terrariaFont.name
                    font.pixelSize: 30
                    color: "white"
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                }

                Text {
                    text: welcomePanel.selectedModeDescription
                    font.family: terrariaFont.name
                    font.pixelSize: 18
                    color: "#d7ecff"
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                }

                Item { Layout.fillHeight: true }

                Button {
                    id: startTutorialButton
                    text: "Start Tutorial"
                    font.family: terrariaFont.name
                    Layout.alignment: Qt.AlignHCenter
                    hoverEnabled: true

                    background: Rectangle {
                        radius: 6
                        color: startTutorialButton.hovered ? "#bfe6ff" : "#a7d8ff"
                        border.color: "#e8f7ff"
                        border.width: 2
                    }

                    contentItem: Text {
                        text: startTutorialButton.text
                        font.family: terrariaFont.name
                        font.pixelSize: 18
                        color: "#1d3f73"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: console.log("Start tutorial for:", welcomePanel.selectedModeTitle)
                }
            }
        }

        component DifficultyButton : Rectangle {
            property string title
            property string imageSource
            property string shortDescription

            radius: 6
            color: "#2a5fa8"
            border.color: "#9bd2ff"
            border.width: 2

            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 6

                Text {
                    text: title
                    font.family: terrariaFont.name
                    font.pixelSize: 20
                    color: "white"
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                }

                Item { Layout.fillHeight: true }

                Image {
                    source: imageSource
                    width: 80
                    height: 80
                    fillMode: Image.PreserveAspectFit
                    Layout.alignment: Qt.AlignHCenter
                }

                Item { Layout.fillHeight: true }

                Text {
                    text: shortDescription
                    font.family: terrariaFont.name
                    font.pixelSize: 12
                    color: "#d7ecff"
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    Layout.fillWidth: true
                }
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true

                onEntered: parent.color = "#3b75c4"
                onExited: parent.color = "#2a5fa8"

onClicked: {
    console.log("Selected difficulty:", title)
    welcomePanel.selectedModeTitle = title
    if (title === "Apprentice") {
        welcomePanel.selectedModeDescription = "Your movement experience is limited to a single screen"
    } else if (title === "Adventurer") {
        welcomePanel.selectedModeDescription = "You can explore the world,build and place objects Configuration is hidden"
    } else if (title === "Wizard") {
        welcomePanel.selectedModeDescription = "You configure the system however your want."
    } else {
        welcomePanel.selectedModeDescription = ""
    }
    configWriter.command = [welcomePanel.configScriptPath, title]
    configWriter.running = true
    welcomePanel.currentPage = "guide"
}
            }
        }
        }
    }
}
