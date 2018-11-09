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
    var round = 0

    function unquoteColumns(columns) {
      for (var i = 0; i < columns.length; ++i) {
        var val = columns[i]
        if (val.length >= 2 &&
            val[0] === '"' && val[val.length - 1] === '"') {
          val = val.substr(1, val.length - 2).replace(/""/g, '"')
          columns[i] = val
        }
      }
      return columns
    }

    function readCsvFile(filePath) {
      var filePathCol = -1
      var lines = ("" + script.readFile(filePath)).split(/[\r\n]+/)
      for (var i = 0; i < lines.length; ++i) {
        var line = lines[i]
        if (line.length > 0) {
          var columns = unquoteColumns(line.split("\t"))
          if (i > 0) {
            if (columns.length < names.length && i + 1 < lines.length) {
              // The line does not contain all columns, check for continuation.
              var lastColumn = columns[columns.length - 1]
              if (lastColumn.length > 1 && lastColumn[0] === '"' &&
                  lastColumn[lastColumn.length - 1] !== '"') {
                for (var consumed = 0, extendedLine = line;
                     i + 1 + consumed < lines.length &&
                     columns.length < names.length;
                     ++consumed) {
                  extendedLine += "\n"
                  extendedLine += lines[i + 1 + consumed]
                  columns = unquoteColumns(extendedLine.split("\t"))
                }
                if (columns.length === names.length) {
                  // Continuation OK, apply changes.
                  lines[i] = extendedLine
                  lines.splice(i + 1, consumed)
                } else {
                  columns = unquoteColumns(line.split("\t"))
                }
              }
            }
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
          app.saveDirectory()
        } else if (numRowsImported === 0 && round === 0) {
          console.log("No files found, importing unconditionally.")
          files = undefined
          ++round;
          app.firstFile()
          doWork()
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
      importPath = app.selectFileName(
        "Import", app.dirName, "CSV files (*.csv);;All files (*)", false)
      if (!importPath) {
        Qt.quit()
        return
      }
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
