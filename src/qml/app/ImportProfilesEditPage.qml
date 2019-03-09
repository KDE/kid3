/**
 * \file ImportProfilesEditPage.qml
 * Page to edit import profiles.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Mar 2019
 *
 * Copyright (C) 2019  Urs Fleisch
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import Kid3 1.1 as Kid3

StringListEditPage {
  id: page

  signal finished()

  title: qsTr("Profiles")

  StringListEditPage {
    id: editProfilePage

    property string name

    editDialog: profileEditDialog
    title: qsTr("Profile") + " " + name
    visible: false

    function setElements(sources) {
      model.clear()
      for (var i = 0; i < sources.length; i++) {
        var src = sources[i]
        model.append({name: sourceToString(src), sources: src})
      }
    }

    function getElements() {
      var sources = []
      for (var i = 0; i < model.count; i++) {
        sources.push(model.get(i).sources)
      }
      return sources
    }

    function sourceToString(source) {
      var parts = source.split(":")
      var server = parts[0] || ""
      var accuracy = parts[1] || ""
      var enabledStr = parts[2] || ""
      var result = server
      if (result) {
        result += ", " + qsTr("Accuracy") + " " + accuracy
        var enabled = []
        if (enabledStr.indexOf("S") !== -1) {
          enabled.push(qsTr("Standard Tags"))
        }
        if (enabledStr.indexOf("A") !== -1) {
          enabled.push(qsTr("Additional Tags"))
        }
        if (enabledStr.indexOf("C") !== -1) {
          enabled.push(qsTr("Cover Art"))
        }
        if (enabled.length > 0) {
          result += ", "
          result += enabled.join(", ")
        }
      }
      return result
    }

    Dialog {
      id: profileEditDialog

      signal completed(bool ok)

      function setElement(element) {
        var parts = element.sources ? element.sources.split(":") : [""]
        serverComboBox.currentIndex = serverComboBox.find(parts[0])
        accuracyLineEdit.text = parts[1] || ""
        var enabledStr = parts[2] || ""
        standardTagsCheckBox.checked = enabledStr.indexOf("S") !== -1
        additionalTagsCheckBox.checked = enabledStr.indexOf("A") !== -1
        coverArtCheckBox.checked = enabledStr.indexOf("C") !== -1
      }

      function getElement() {
        var sources = serverComboBox.currentText + ":" +
                      accuracyLineEdit.text + ":"
        if (standardTagsCheckBox.checked) {
          sources += "S"
        }
        if (additionalTagsCheckBox.checked) {
          sources += "A"
        }
        if (coverArtCheckBox.checked) {
          sources += "C"
        }
        return {name: editProfilePage.sourceToString(sources),
                sources: sources}
      }

      modal: true
      width: Math.min(root.width, constants.gu(70))
      x: (root.width - width) / 2
      y: 0
      standardButtons: Dialog.Ok | Dialog.Cancel

      GridLayout {
        columns: 2
        width: parent.width
        Label {
          text: qsTr("Server")
        }
        ComboBox {
          id: serverComboBox
          model: app.getServerImporterNames()
          Layout.fillWidth: true
        }
        Label {
          text: qsTr("Accuracy")
        }
        TextField {
          id: accuracyLineEdit
          validator: IntValidator{
            bottom: 0
            top: 100
          }
          selectByMouse: true
          Layout.fillWidth: true
        }
        CheckBox {
          id: standardTagsCheckBox
          text: qsTr("Standard Tags")
          Layout.columnSpan: 2
        }
        CheckBox {
          id: additionalTagsCheckBox
          text: qsTr("Additional Tags")
          Layout.columnSpan: 2
        }
        CheckBox {
          id: coverArtCheckBox
          text: qsTr("Cover Art")
          Layout.columnSpan: 2
        }
      }

      onAccepted: completed(true)
      onRejected: completed(false)
    }

    StackView.onActivated: {
      var idx = page.currentIndex
      var srcStr = ""
      if (idx >= 0 && idx < page.model.count) {
        var element = page.model.get(idx)
        name = element.name || ""
        srcStr = element.sources || ""
      } else {
        name = ""
        srcStr = ""
      }
      setElements(srcStr.split(";"))
    }
    StackView.onDeactivated: {
      var idx = page.currentIndex
      if (idx >= 0 && idx < page.model.count) {
        var element = page.model.get(idx)
        if (element.name === name) {
          element.sources = getElements().join(";")
          page.model.set(idx, element)
          page.saveModel()
        }
      }
    }
  }

  function setElements(namesSources) {
    var names = namesSources[0]
    var sources = namesSources[1]
    model.clear()
    for (var i = 0; i < names.length; i++) {
      model.append({name: names[i], sources: sources[i]})
    }
  }

  function getElements() {
    var names = []
    var sources = []
    for (var i = 0; i < model.count; i++) {
      names.push(model.get(i).name)
      sources.push(model.get(i).sources)
    }
    return [names, sources]
  }

  function saveModel() {
    var namesSources = getElements()
    configs.batchImportConfig().profileNames = namesSources[0]
    configs.batchImportConfig().profileSources = namesSources[1]
  }

  onEditClicked: function() {
    page.StackView.view.push(editProfilePage)
  }
  StackView.onActivated: {
    var idx = currentIndex
    setElements([configs.batchImportConfig().profileNames,
                 configs.batchImportConfig().profileSources])
    currentIndex = idx
  }
  StackView.onDeactivated: {
    saveModel()
    finished()
  }
}
