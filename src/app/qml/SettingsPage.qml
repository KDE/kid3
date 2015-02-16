/**
 * \file SettingsPage.qml
 * Settings page.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Feb 2015
 *
 * Copyright (C) 2015  Urs Fleisch
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

import QtQuick 2.2
import "ComponentsQtQuick" //@!Ubuntu
//import Ubuntu.Components 1.1 //@Ubuntu
//import Ubuntu.Components.Popups 1.0 //@Ubuntu
//import Ubuntu.Components.ListItems 1.0 //@Ubuntu
import Kid3App 1.0

AbstractSettingsPage {
  id: page

  title: qsTr("Settings")
  model: [
    SettingsElement { name: qsTr("Tags") },
    SettingsElement { name: qsTr("Files") },
    SettingsElement { name: qsTr("Plugins") }
  ]
  onClicked: pageStack.push(
               [ tagsPage, filesPage, pluginsPage ][index])
  Component {
    id: tagsPage
    AbstractSettingsPage {
      title: qsTr("Tags")
      visible: false
      model: [
        SettingsElement {
          name: qsTr("Mark truncated fields")
        },
        SettingsElement {
          name: qsTr("ID3v1 text encoding")
          dropDownModel: configs.tagConfig().getTextEncodingV1Names()
        },
        SettingsElement {
          name: qsTr("ID3v2 text encoding")
          dropDownModel: configs.tagConfig().getTextEncodingNames()
        },
        SettingsElement {
          name: qsTr("Use track/total number of tracks format")
        },
        SettingsElement {
          name: qsTr("Genre as text instead of numeric string")
        },
        SettingsElement {
          name: qsTr("Version used for new ID3v2 tags")
          dropDownModel: configs.tagConfig().getId3v2VersionNames()
        },
        SettingsElement {
          name: qsTr("Ogg/Vorbis comment field name")
          dropDownModel: configs.tagConfig().getCommentNames()
        },
        SettingsElement {
          name: qsTr("Ogg/Vorbis picture field name")
          dropDownModel: configs.tagConfig().getPictureNames()
        },
        SettingsElement {
          name: qsTr("Mark if picture larger than maxium size")
        },
        SettingsElement {
          name: qsTr("Picture maximum size (bytes)")
        },
        SettingsElement {
          name: qsTr("Show only custom genres")
        },
        SettingsElement {
          name: qsTr("Case conversion")
          dropDownModel: configs.tagFormatConfig().getCaseConversionNames()
        },
        SettingsElement {
          name: qsTr("Locale")
          dropDownModel: configs.tagFormatConfig().getLocaleNames()
        },
        SettingsElement {
          name: qsTr("String replacement")
        }
      ]
      onActiveChanged: {
        var tagCfg = configs.tagConfig()
        var fmtCfg = configs.tagFormatConfig()
        if (active) {
          model[0].value = tagCfg.markTruncations
          model[1].value = tagCfg.textEncodingV1Index
          model[2].value = tagCfg.textEncoding
          model[3].value = tagCfg.enableTotalNumberOfTracks
          model[4].value = tagCfg.genreNotNumeric
          model[5].value = tagCfg.id3v2Version
          model[6].value = tagCfg.commentName
          model[7].value = tagCfg.pictureNameIndex
          model[8].value = tagCfg.markOversizedPictures
          model[9].value = tagCfg.maximumPictureSize
          model[10].value = tagCfg.onlyCustomGenres
          model[11].value = fmtCfg.caseConversion
          model[12].value =
              model[12].dropDownModel.indexOf(fmtCfg.localeName) === -1
              ? model[12].dropDownModel[0] : fmtCfg.localeName
          model[13].value = fmtCfg.strRepEnabled
        } else {
          tagCfg.markTruncations = model[0].value
          tagCfg.textEncodingV1Index = model[1].value
          tagCfg.textEncoding = model[2].value
          tagCfg.enableTotalNumberOfTracks = model[3].value
          tagCfg.genreNotNumeric = model[4].value
          tagCfg.id3v2Version = model[5].value
          tagCfg.commentName = model[6].value
          tagCfg.pictureNameIndex = model[7].value
          tagCfg.markOversizedPictures = model[8].value
          tagCfg.maximumPictureSize = model[9].value
          tagCfg.onlyCustomGenres = model[10].value
          fmtCfg.caseConversion = model[11].value
          fmtCfg.localeName =
              model[12].dropDownModel.indexOf(model[12].value) > 0
              ? model[12].value : ""
          fmtCfg.strRepEnabled = model[13].value
        }
      }
    }
  }
  Component {
    id: filesPage
    AbstractSettingsPage {
      title: qsTr("Files")
      visible: false
      model: [
        SettingsElement {
          name: qsTr("Load last-opened files")
        },
        SettingsElement {
          name: qsTr("Preserve file timestamp")
        },
        SettingsElement {
          name: qsTr("Mark changes")
        },
        SettingsElement {
          name: qsTr("Format while editing")
        },
        SettingsElement {
          name: qsTr("Case conversion")
          dropDownModel: configs.filenameFormatConfig().getCaseConversionNames()
        },
        SettingsElement {
          name: qsTr("Locale")
          dropDownModel: configs.filenameFormatConfig().getLocaleNames()
        },
        SettingsElement {
          name: qsTr("String replacement")
        },
        SettingsElement {
          name: qsTr("Filename for cover")
        },
        SettingsElement {
          name: qsTr("To filename format")
          dropDownModel: configs.fileConfig().toFilenameFormats
        },
        SettingsElement {
          name: qsTr("From filename format")
          dropDownModel: configs.fileConfig().fromFilenameFormats
        }
      ]
      onActiveChanged: {
        var fileCfg = configs.fileConfig()
        var fmtCfg = configs.filenameFormatConfig()
        if (active) {
          model[0].value = fileCfg.loadLastOpenedFile
          model[1].value = fileCfg.preserveTime
          model[2].value = fileCfg.markChanges
          model[3].value = fmtCfg.formatWhileEditing
          model[4].value = fmtCfg.caseConversion
          model[5].value =
              model[5].dropDownModel.indexOf(fmtCfg.localeName) === -1
              ? model[5].dropDownModel[0] : fmtCfg.localeName
          model[6].value = fmtCfg.strRepEnabled
          model[7].value = fileCfg.defaultCoverFileName
          model[8].value = fileCfg.toFilenameFormat
          model[9].value = fileCfg.fromFilenameFormat
        } else {
          fileCfg.loadLastOpenedFile = model[0].value
          fileCfg.preserveTime = model[1].value
          fileCfg.markChanges = model[2].value
          fmtCfg.formatWhileEditing = model[3].value
          fmtCfg.caseConversion = model[4].value
          fmtCfg.localeName =
              model[5].dropDownModel.indexOf(model[5].value) > 0
              ? model[5].value : ""
          fmtCfg.strRepEnabled = model[6].value
          fileCfg.defaultCoverFileName = model[7].value
          fileCfg.toFilenameFormat = model[8].value
          fileCfg.fromFilenameFormat = model[9].value
        }
      }
    }
  }
  Component {
    id: pluginsPage
    AbstractSettingsPage {
      title: qsTr("Plugins")
      visible: false
      onActiveChanged: {
        var tagCfg = configs.tagConfig()
        var importCfg = configs.importConfig()
        if (active) {
          var disabledTagPlugins = tagCfg.disabledPlugins
          var disabledImportPlugins = importCfg.disabledPlugins
          for (var i = 0; i < model.length; ++i) {
            var name = model[i].name
            model[i].value =
                disabledTagPlugins.indexOf(name) === -1 &&
                disabledImportPlugins.indexOf(name) === -1
          }
        } else {
          var availableTagPlugins = tagCfg.availablePlugins
          var availableImportPlugins = importCfg.availablePlugins
          var disabledTagPlugins = []
          var disabledImportPlugins = []
          for (var i = 0; i < model.length; ++i) {
            if (model[i].value === false) {
              var name = model[i].name
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
}
