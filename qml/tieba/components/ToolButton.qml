import QtQuick 1.1
import com.nokia.meego 1.0

// Drop-in replacement for ToolIcon using the app's own themed SVG set.
// Usage: ToolButton { icon: "back"; onClicked: ... }
ToolIcon {
    property string icon: ""
    iconSource: icon === "" ? "" :
        Qt.resolvedUrl("../icons/" + (appTheme.dark ? "dark" : "light") + "/" + icon + ".svg")
}
