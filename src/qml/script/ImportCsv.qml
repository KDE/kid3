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
    var rows, names, files = {}
    var numRowsImported = 0

    function readCsvFile(filePath) {
      var filePathCol = -1
      var lines = ("" + script.readFile(filePath)).split("\n")
      for (var i = 0; i < lines.length; ++i) {
        var line = lines[i]
        if (line.length > 0) {
          var columns = line.split("\t")
          if (i > 0) {
            if (filePathCol >= 0 && filePathCol < columns.length) {
              files[columns[filePathCol]] = i - 1
            }
            rows.push(columns)
          } else {
            names = columns
            filePathCol = names.indexOf("File Path")
            rows = []
          }
        }
      }
    }

    function doWork() {
      var rowNr
      if (files) {
        var filePath = app.selectionInfo.filePath
        rowNr = files[filePath]
      } else {
        rowNr = numRowsImported
      }

      if (typeof rowNr !== "undefined" && rowNr >= 0 && rowNr < rows.length) {
        var row = rows[rowNr]
        for (var i = 0; i < row.length && i < names.length; ++i) {
          var frameName = names[i]
          var frameValue = row[i]
          if (frameName !== "File Path" && frameName !== "Duration" &&
              frameValue !== "") {
            frameValue = frameValue.replace(/\\n/g, "\n").replace(/\\r/g, "\r").
                                    replace(/\\t/g, "\t")
            if (frameName.substr(0, 2) === "v1") {
              frameName = frameName.substr(2)
              app.setFrame(tagv1, frameName, frameValue)
            } else {
              app.setFrame(tagv2, frameName, frameValue)
            }
          }
        }
        ++numRowsImported
      } else if (filePath) {
        console.log("No data for " + filePath)
      }
      if (!app.nextFile()) {
        console.log("Imported tags for %1 files".arg(numRowsImported))
        if (isStandalone()) {
          // Save the changes if the script is started stand-alone, not from Kid3.
//          app.saveDirectory()
        }
        Qt.quit()
      } else {
        setTimeout(doWork, 1)
      }
    }

    function startWork() {
      app.expandFileListFinished.disconnect(startWork)
      console.log("Setting tags")
      app.firstFile()
      doWork()
    }

    var importPath = getArguments()[0]
    if (!importPath) {
      importPath = script.tempPath() + "/export.csv"
    }
    readCsvFile(importPath)
    if (rows && rows.length > 0 && names && names.length > 1) {
      console.log("Read tags for %1 files from %2".
                  arg(rows.length).arg(importPath))
      if (Object.keys(files).length === 0) {
        console.log("No File Path column found, importing unconditionally.")
        files = undefined
      }

      if (!isStandalone()) {
        console.log("Expanding file list")
        app.expandFileListFinished.connect(startWork)
        app.requestExpandFileList()
      } else {
        startWork()
      }
    } else {
      console.log("No data found in %1".arg(importPath))
    }
  }
}
