/**
 * \file ExportCsv.qml
 * Export all tags of all files to a CSV file.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 06 Mar 2015
 *
 * Copyright (C) 2015-2021  Urs Fleisch
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

import Kid3 1.0

Kid3Script {
  onRun: {
    var columnSet = {}
    var rows = []
    var selectedFramesV1 = null
    var selectedFramesV2 = null
    var selectedFramesV3 = null

    /**
     * Get list of frame names which are selected in frame table.
     * @param tagNr Frame.Tag_1, Frame.Tag_2, or Frame.Tag_3
     * @return selected frame names, null if all frames are selected.
     */
    function getSelectedFrames(tagNr) {
      var checked = []
      var frameModel = app.tag(tagNr).frameModel
      var numRows = frameModel.rowCount()
      for (var row = 0; row < numRows; ++row) {
        var name = script.getRoleData(frameModel, row, "name")
        if (script.getRoleData(frameModel, row, "checkState") === Qt.Checked) {
          checked.push(name)
        }
      }
      return checked.length < numRows ? checked : null
    }

    /**
     * Remove all frames from tags which are not included in @a selectedFrames.
     * @param tags object with frame names as keys
     * @param selectedFrames array with keys which will not be removed,
     * if null, nothing will be removed
     */
    function removeUnselectedFrames(tags, selectedFrames) {
      if (selectedFrames) {
        for (var name in tags) {
          if (tags.hasOwnProperty(name)) {
            if (!selectedFrames.includes(name)) {
              delete tags[name]
            }
          }
        }
      }
    }

    function doWork() {
      var tags
      var prop
      if (app.selectionInfo.tag(Frame.Tag_2).tagFormat) {
        tags = app.getAllFrames(tagv2)
        removeUnselectedFrames(tags, selectedFramesV2)
      }
      if (app.selectionInfo.tag(Frame.Tag_1).tagFormat) {
        var tagsV1 = app.getAllFrames(tagv1)
        removeUnselectedFrames(tagsV1, selectedFramesV1)
        if (typeof tags === "undefined") {
          tags = {}
        }
        for (prop in tagsV1) {
          tags["v1" + prop] = tagsV1[prop]
        }
      }
      if (app.selectionInfo.tag(Frame.Tag_3).tagFormat) {
        var tagsV3 = app.getAllFrames(Frame.TagV3)
        removeUnselectedFrames(tagsV3, selectedFramesV3)
        if (typeof tags === "undefined") {
          tags = {}
        }
        for (prop in tagsV3) {
          tags["v3" + prop] = tagsV3[prop]
        }
      }
      if (tags) {
        // Feel free to add additional columns, but you may have to exclude
        // them in ImportCsv.qml too.
        // tags["Duration"] = app.selectionInfo.formatString(Frame.Tag_2, "%{duration}")
        // tags["Bitrate"] = app.selectionInfo.formatString(Frame.Tag_2, "%{bitrate}")
        // tags["Mode"] = app.selectionInfo.formatString(Frame.Tag_2, "%{mode}")
        // tags["Codec"] = app.selectionInfo.formatString(Frame.Tag_2, "%{codec}")
        // tags["Directory"] = app.selectionInfo.formatString(Frame.Tag_2, "%{dirname}")
        // tags["File"] = app.selectionInfo.formatString(Frame.Tag_2, "%{file}")
        rows.push(tags)
        for (prop in tags) {
          columnSet[prop] = null
        }
        tags["File Path"] = app.selectionInfo.filePath
      }

      if (!app.nextFile()) {
        var columns = []
        for (prop in columnSet) {
          columns.push(prop)
        }
        columns.sort()
        columns.unshift("File Path")

        var numRows = rows.length
        var numColumns = columns.length
        var columnNr, rowNr
        var txt = ""
        for (columnNr = 0; columnNr < numColumns; ++columnNr) {
          if (columnNr > 0) {
            txt += "\t"
          }
          txt += columns[columnNr]
        }
        txt += "\n"
        for (rowNr = 0; rowNr < numRows; ++rowNr) {
          var row = rows[rowNr]
          for (columnNr = 0; columnNr < numColumns; ++columnNr) {
            var value = row[columns[columnNr]]
            if (typeof value === "undefined") {
              value = ""
            } else {
              if (value.indexOf("\n") !== -1) {
                value = '"' + value.replace(/"/g, '""').replace(/\r\n/g, "\n") +
                        '"'
              }
              value = value.replace(/\t/g, " ")
            }
            if (columnNr > 0) {
              txt += "\t"
            }
            txt += value
          }
          txt += "\n"
        }
        var exportPath = getArguments()[0]
        if (!exportPath) {
          exportPath = app.selectFileName(
            "Export", app.dirName + "/export.csv",
            "CSV files (*.csv);;All files (*)", true)
          if (!exportPath) {
            Qt.quit()
            return
          }
        }
        if (script.writeFile(exportPath, txt)) {
          console.log("Exported tags of %1 files to %2".
                      arg(numRows).arg(exportPath))
        } else {
          console.log("Failed to write", exportPath)
        }
        Qt.quit()
      } else {
        setTimeout(doWork, 1)
      }
    }

    function startWork() {
      selectedFramesV1 = getSelectedFrames(Frame.Tag_1)
      selectedFramesV2 = getSelectedFrames(Frame.Tag_2)
      selectedFramesV3 = getSelectedFrames(Frame.Tag_3)

      app.expandFileListFinished.disconnect(startWork)
      console.log("Reading tags")
      app.firstFile()
      doWork()
    }

    if (!isStandalone() && app.hasGui()) {
      console.log("Expanding file list")
      app.expandFileListFinished.connect(startWork)
      app.requestExpandFileList()
    } else {
      startWork()
    }
  }
}
