/**
 * \file ExportAll.qml
 * Export all tags of all files to a CSV file.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 06 Mar 2015
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

import Kid3 1.0

Kid3Script {
  onRun: {
    var columnSet = {}
    var rows = []

    function doWork() {
      var tags
      var prop
      if (app.selectionInfo.tag(Frame.Tag_2).tagFormat) {
        tags = app.getAllFrames(tagv2)
      }
      if (app.selectionInfo.tag(Frame.Tag_1).tagFormat) {
        var tagsV1 = app.getAllFrames(tagv1)
        if (typeof tags === "undefined") {
          tags = {}
        }
        for (prop in tagsV1) {
          tags["v1" + prop] = tagsV1[prop]
        }
      }
      if (tags) {
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
      app.expandFileListFinished.disconnect(startWork)
      console.log("Reading tags")
      app.firstFile()
      doWork()
    }

    if (!isStandalone()) {
      console.log("Expanding file list")
      app.expandFileListFinished.connect(startWork)
      app.requestExpandFileList()
    } else {
      startWork()
    }
  }
}
