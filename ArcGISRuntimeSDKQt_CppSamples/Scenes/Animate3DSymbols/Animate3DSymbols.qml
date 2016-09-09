// [WriteFile Name=Animate3DSymbols, Category=Scenes]
// [Legal]
// Copyright 2016 Esri.

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// [Legal]

import QtQuick 2.6
import QtQuick.Controls 1.4
import QtQuick.Extras 1.4
import Esri.Samples 1.0
import Esri.ArcGISExtras 1.1

Animate3DSymbolsSample {
    id: rootRectangle
    clip: true

    width: 800
    height: 600

    property double scaleFactor: System.displayScaleFactor
    property url dataPath: System.userHomePath + "/ArcGIS/Runtime/Data/3D"

    SceneView {
        id: sceneView
        objectName: "sceneView"
        anchors.fill: parent
        z: 10

//        MouseArea {
//            anchors.fill: parent
//            onPressed: mouse.accepted = followButton.checked
//            onWheel: wheel.accepted = followButton.checked
//        }
    }

    ToggleButton{
        id: playButton
        anchors.top: sceneView.top
        anchors.left: sceneView.left
        width: 16 * scaleFactor
        height: 16 * scaleFactor
        checked: false
        enabled: missionReady
        z: 110
        text: checked ? "||" : ">"
    }

    ComboBox{
        id: missionList
        model: missionsModel()
        textRole: "display"
        anchors.top: playButton.bottom
        anchors.left: sceneView.left
        z: 110

        onCurrentTextChanged: changeMission(currentText)
    }

    CheckBox{
        id: followButton
        anchors.top: missionList.bottom
        anchors.left: sceneView.left
        checked: true
        enabled: missionReady
        z: 110
        text: "follow"
        onCheckedChanged: setFollowing(checked);
    }

    Slider{
        id: cameraDistance
        anchors.top: sceneView.top
        anchors.right: sceneView.right
        z: 110
        enabled: missionReady
        minimumValue: 10.
        maximumValue: 500.
        value: 200.

        Component.onCompleted: setZoom(value);
        onValueChanged: setZoom(value);
    }

    Slider{
        id: cameraAngle
        anchors.top: cameraDistance.bottom
        anchors.right: sceneView.right
        z: 110
        enabled: missionReady
        minimumValue: 0.
        maximumValue: 180.
        value: 75.

        Component.onCompleted: setAngle(value);
        onValueChanged: setAngle(value)
    }
    Slider{
        id: animationSpeed
        anchors.top: cameraAngle.bottom
        anchors.right: sceneView.right
        z: 110
        enabled: missionReady
        minimumValue: 10
        maximumValue: 100
        value: 20
    }

    Rectangle
    {
        anchors.left: sceneView.left
        anchors.bottom: sceneView.bottom
        width: sceneView.width * .2
        height: sceneView.height * .4
        color: "black"
        z: 100

        MapView{
            objectName: "mapView"
            anchors.fill: parent
            anchors.margins: 2

//            MouseArea {
//                anchors.fill: parent
//                onPressed: mouse.accepted = followButton.checked
//                onWheel: wheel.accepted = followButton.checked
//            }
        }
    }

    Timer{
        id: timer
        interval: 110 - animationSpeed.value; running: playButton.checked; repeat: true
        onTriggered: nextFrame();
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border {
            width: 0.5 * scaleFactor
            color: "black"
        }
    }
}
