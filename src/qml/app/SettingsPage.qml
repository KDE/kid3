/**
 * \file SettingsPage.qml
 * Settings page.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Feb 2015
 *
 * Copyright (C) 2015-2019  Urs Fleisch
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
import QtQuick.Controls 2.2

AbstractSettingsPage {
  id: settingsPage

  title: qsTr("Settings")
  model: [
    SettingsElement { name: qsTr("Tags") },
    SettingsElement { name: qsTr("Files") },
    SettingsElement { name: qsTr("Plugins") },
    SettingsElement { name: qsTr("Appearance") }
  ]
  onClicked: pageStack.push(
               [ tagsPage, filesPage, pluginsPage, appearancePage ][index])
  Component {
    id: tagsPage
    AbstractSettingsPage {
      property QtObject tagCfg: configs.tagConfig()
      property QtObject fmtCfg: configs.tagFormatConfig()
      title: qsTr("Tags")
      visible: false
      model: [
        SettingsElement {
          name: qsTr("Mark truncated fields")
          onActivated: function() { value = tagCfg.markTruncations; }
          onDeactivated: function() { tagCfg.markTruncations = value; }
        },
        SettingsElement {
          name: qsTr("ID3v1 text encoding")
          dropDownModel: configs.tagConfig().getTextCodecNames()
          onActivated: function() { value = tagCfg.textEncodingV1Index; }
          onDeactivated: function() { tagCfg.textEncodingV1Index = value; }
        },
        SettingsElement {
          name: qsTr("ID3v2 text encoding")
          dropDownModel: configs.tagConfig().getTextEncodingNames()
          onActivated: function() { value = tagCfg.textEncoding; }
          onDeactivated: function() { tagCfg.textEncoding = value; }
        },
        SettingsElement {
          name: qsTr("Use track/total number of tracks format")
          onActivated: function() { value = tagCfg.enableTotalNumberOfTracks; }
          onDeactivated: function() { tagCfg.enableTotalNumberOfTracks = value; }
        },
        SettingsElement {
          name: qsTr("Genre as text instead of numeric string")
          onActivated: function() { value = tagCfg.genreNotNumeric; }
          onDeactivated: function() { tagCfg.genreNotNumeric = value; }
        },
        SettingsElement {
          name: qsTr("WAV files with lowercase id3 chunk")
          onActivated: function() { value = tagCfg.lowercaseId3RiffChunk; }
          onDeactivated: function() { tagCfg.lowercaseId3RiffChunk = value; }
        },
        SettingsElement {
          name: qsTr("Version used for new ID3v2 tags")
          dropDownModel: configs.tagConfig().getId3v2VersionNames()
          onActivated: function() { value = tagCfg.id3v2Version; }
          onDeactivated: function() { tagCfg.id3v2Version = value; }
        },
        SettingsElement {
          name: qsTr("Track number digits")
          onActivated: function() { value = tagCfg.trackNumberDigits; }
          onDeactivated: function() { tagCfg.trackNumberDigits = value; }
        },
        SettingsElement {
          name: qsTr("Ogg/Vorbis comment field name")
          dropDownModel: configs.tagConfig().getCommentNames()
          onActivated: function() {
            value = dropDownModel.indexOf(tagCfg.commentName)
          }
          onDeactivated: function() {
            tagCfg.commentName = dropDownModel[value]
          }
        },
        SettingsElement {
          name: qsTr("Ogg/Vorbis picture field name")
          dropDownModel: configs.tagConfig().getPictureNames()
          onActivated: function() { value = tagCfg.pictureNameIndex; }
          onDeactivated: function() { tagCfg.pictureNameIndex = value; }
        },
        SettingsElement {
          name: qsTr("RIFF track number field name")
          dropDownModel: configs.tagConfig().getRiffTrackNames()
          onActivated: function() {
            value = dropDownModel.indexOf(tagCfg.riffTrackName)
          }
          onDeactivated: function() {
            tagCfg.riffTrackName = dropDownModel[value]
          }
        },
        SettingsElement {
          name: qsTr("Mark if picture larger than maxium size")
          onActivated: function() { value = tagCfg.markOversizedPictures; }
          onDeactivated: function() { tagCfg.markOversizedPictures = value; }
        },
        SettingsElement {
          name: qsTr("Picture maximum size (bytes)")
          onActivated: function() { value = tagCfg.maximumPictureSize; }
          onDeactivated: function() { tagCfg.maximumPictureSize = value; }
        },
        SettingsElement {
          name: qsTr("Show only custom genres")
          onActivated: function() { value = tagCfg.onlyCustomGenres; }
          onDeactivated: function() { tagCfg.onlyCustomGenres = value; }
          onEdit: function() {
            stringListEditPage.title = qsTr("Custom Genres")
            stringListEditPage.onActivated = function() {
              stringListEditPage.setElements(tagCfg.customGenres)
            }
            stringListEditPage.onDeactivated = function() {
              tagCfg.customGenres = stringListEditPage.getElements()
            }
            page.StackView.view.push(stringListEditPage)
          }
        },
        SettingsElement {
          name: qsTr("Case conversion")
          dropDownModel: configs.tagFormatConfig().getCaseConversionNames()
          onActivated: function() { value = fmtCfg.caseConversion; }
          onDeactivated: function() { fmtCfg.caseConversion = value; }
        },
        SettingsElement {
          name: qsTr("Locale")
          dropDownModel: configs.tagFormatConfig().getLocaleNames()
          onActivated: function() {
            var idx = dropDownModel.indexOf(fmtCfg.localeName)
            value = idx === -1 ? 0 : idx
          }
          onDeactivated: function() {
            fmtCfg.localeName = value > 0 ? dropDownModel[value] : ""
          }
        },
        SettingsElement {
          name: qsTr("String replacement")
          onActivated: function() { value = fmtCfg.strRepEnabled; }
          onDeactivated: function() { fmtCfg.strRepEnabled = value; }
          onEdit: function() {
            mapEditPage.title = qsTr("String Replacement")
            mapEditPage.onActivated = function() {
              mapEditPage.setElements(fmtCfg.strRepMap)
            }
            mapEditPage.onDeactivated = function() {
              fmtCfg.strRepMap = mapEditPage.getElements()
            }
            page.StackView.view.push(mapEditPage)
          }
        }
      ]
      StackView.onActivated: activateAll()
      StackView.onDeactivated: deactivateAll()
    }
  }
  Component {
    id: filesPage
    AbstractSettingsPage {
      property QtObject fileCfg: configs.fileConfig()
      property QtObject fmtCfg: configs.filenameFormatConfig()
      title: qsTr("Files")
      visible: false
      model: [
        SettingsElement {
          name: qsTr("Load last-opened files")
          onActivated: function() { value = fileCfg.loadLastOpenedFile; }
          onDeactivated: function() { fileCfg.loadLastOpenedFile = value; }
        },
        SettingsElement {
          name: qsTr("Preserve file timestamp")
          onActivated: function() { value = fileCfg.preserveTime; }
          onDeactivated: function() { fileCfg.preserveTime = value; }
        },
        SettingsElement {
          name: qsTr("Mark changes")
          onActivated: function() { value = fileCfg.markChanges; }
          onDeactivated: function() { fileCfg.markChanges = value; }
        },
        SettingsElement {
          name: qsTr("Automatically apply format")
          onActivated: function() { value = fmtCfg.formatWhileEditing; }
          onDeactivated: function() { fmtCfg.formatWhileEditing = value; }
        },
        SettingsElement {
          name: qsTr("Use maximum length")
          onActivated: function() { value = fmtCfg.enableMaximumLength; }
          onDeactivated: function() { fmtCfg.enableMaximumLength = value; }
        },
        SettingsElement {
          name: qsTr("Maximum length")
          onActivated: function() { value = fmtCfg.maximumLength; }
          onDeactivated: function() { fmtCfg.maximumLength = value; }
        },
        SettingsElement {
          name: qsTr("Case conversion")
          dropDownModel: configs.filenameFormatConfig().getCaseConversionNames()
          onActivated: function() { value = fmtCfg.caseConversion; }
          onDeactivated: function() { fmtCfg.caseConversion = value; }
        },
        SettingsElement {
          name: qsTr("Locale")
          dropDownModel: configs.filenameFormatConfig().getLocaleNames()
          onActivated: function() {
            var idx = dropDownModel.indexOf(fmtCfg.localeName)
            value = idx === -1 ? 0 : idx
          }
          onDeactivated: function() {
            fmtCfg.localeName = value > 0 ? dropDownModel[value] : ""
          }
        },
        SettingsElement {
          name: qsTr("String replacement")
          onActivated: function() { value = fmtCfg.strRepEnabled; }
          onDeactivated: function() { fmtCfg.strRepEnabled = value; }
          onEdit: function() {
            mapEditPage.title = qsTr("String Replacement")
            mapEditPage.onActivated = function() {
              mapEditPage.setElements(fmtCfg.strRepMap)
            }
            mapEditPage.onDeactivated = function() {
              fmtCfg.strRepMap = mapEditPage.getElements()
            }
            page.StackView.view.push(mapEditPage)
          }
        },
        SettingsElement {
          name: qsTr("Filename for cover")
          onActivated: function() { value = fileCfg.defaultCoverFileName; }
          onDeactivated: function() { fileCfg.defaultCoverFileName = value; }
        },
        SettingsElement {
          name: qsTr("Playlist text encoding")
          dropDownModel: configs.fileConfig().getTextCodecNames()
          onActivated: function() { value = fileCfg.textEncodingIndex; }
          onDeactivated: function() { fileCfg.textEncodingIndex = value; }
        },
        SettingsElement {
          name: qsTr("To filename format")
          dropDownModel: configs.fileConfig().toFilenameFormats
          width: constants.gu(51)
          onActivated: function() {
            value = dropDownModel.indexOf(fileCfg.toFilenameFormat)
          }
          onDeactivated: function() {
            fileCfg.toFilenameFormat = dropDownModel[value]
          }
          onEdit: function() {
            stringListEditPage.title = qsTr("Filename from Tag")
            stringListEditPage.onActivated = function() {
              stringListEditPage.setElements(dropDownModel)
              stringListEditPage.currentIndex = value
            }
            stringListEditPage.onDeactivated = function() {
              var lst = stringListEditPage.getElements()
              configs.fileConfig().toFilenameFormats = lst
              configs.fileConfig().toFilenameFormat =
                  lst[stringListEditPage.currentIndex]
            }
            page.StackView.view.push(stringListEditPage)
          }
        },
        SettingsElement {
          name: qsTr("From filename format")
          dropDownModel: configs.fileConfig().fromFilenameFormats
          width: constants.gu(51)
          onActivated: function() {
            value = dropDownModel.indexOf(fileCfg.fromFilenameFormat)
          }
          onDeactivated: function() {
            fileCfg.fromFilenameFormat = dropDownModel[value]
          }
          onEdit: function() {
            stringListEditPage.title = qsTr("Tag from Filename")
            stringListEditPage.onActivated = function() {
              stringListEditPage.setElements(dropDownModel)
              stringListEditPage.currentIndex = value
            }
            stringListEditPage.onDeactivated = function() {
              var lst = stringListEditPage.getElements()
              configs.fileConfig().fromFilenameFormats = lst
              configs.fileConfig().fromFilenameFormat =
                  lst[stringListEditPage.currentIndex]
            }
            page.StackView.view.push(stringListEditPage)
          }
        }
      ]
      StackView.onActivated: activateAll()
      StackView.onDeactivated: deactivateAll()
    }
  }
  Component {
    id: pluginsPage
    AbstractSettingsPage {
      title: qsTr("Plugins")
      visible: false
      StackView.onActivated: {
        var tagCfg = configs.tagConfig()
        var importCfg = configs.importConfig()
        var disabledTagPlugins, disabledImportPlugins, i, name
        disabledTagPlugins = tagCfg.disabledPlugins
        disabledImportPlugins = importCfg.disabledPlugins
        for (i = 0; i < model.length; ++i) {
          name = model[i].name
          model[i].value =
              disabledTagPlugins.indexOf(name) === -1 &&
              disabledImportPlugins.indexOf(name) === -1
        }
      }
      StackView.onDeactivated: {
        var tagCfg = configs.tagConfig()
        var importCfg = configs.importConfig()
        var disabledTagPlugins, disabledImportPlugins, i, name
        var availableTagPlugins = tagCfg.availablePlugins
        var availableImportPlugins = importCfg.availablePlugins
        disabledTagPlugins = []
        disabledImportPlugins = []
        for (i = 0; i < model.length; ++i) {
          if (model[i].value === false) {
            name = model[i].name
            if (availableTagPlugins.indexOf(name) !== -1) {
              disabledTagPlugins.push(name)
            } else if (availableImportPlugins.indexOf(name) !== -1) {
              disabledImportPlugins.push(name)
            }
          }
        }
        tagCfg.disabledPlugins = disabledTagPlugins
        importCfg.disabledPlugins = disabledImportPlugins
      }
      Component.onCompleted: {
        // A deep copy is necessary in QtQuick 2 because of QTBUG-33149
        // (concat does not work with QStringList) and to avoid modification of
        // the original list in the config.
        var availablePlugins = configs.tagConfig().availablePlugins.slice()
        availablePlugins = availablePlugins.concat(
               configs.importConfig().availablePlugins)
        var settingsModel = []
        var elementComponent = Qt.createComponent("SettingsElement.qml")
        for (var i = 0; i < availablePlugins.length; ++i) {
          var elementObj = elementComponent.createObject(null)
          elementObj.name = availablePlugins[i]
          settingsModel.push(elementObj)
        }
        model = settingsModel
      }
    }
  }
  Component {
    id: appearancePage
    AbstractSettingsPage {
      property QtObject mainWindowCfg: configs.mainWindowConfig()
      title: qsTr("Appearance")
      visible: false
      model: [
        SettingsElement {
          name: qsTr("Theme")
          dropDownModel: configs.mainWindowConfig().getQtQuickStyleNames()
          onActivated: function() {
            value = dropDownModel.indexOf(mainWindowCfg.qtQuickStyle)
          }
          onDeactivated: function() {
            mainWindowCfg.qtQuickStyle = dropDownModel[value]
          }
        }
      ]
      StackView.onActivated: activateAll()
      StackView.onDeactivated: deactivateAll()
    }
  }

  StringListEditPage {
    id: stringListEditPage
    property var onActivated
    property var onDeactivated
    visible: false
    StackView.onActivated: onActivated()
    StackView.onDeactivated: onDeactivated()
  }
  MapEditPage {
    id: mapEditPage
    property var onActivated
    property var onDeactivated
    visible: false
    StackView.onActivated: onActivated()
    StackView.onDeactivated: onDeactivated()
  }
}
