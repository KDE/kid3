/**
 * \file ImportJson.qml
 * Import all tags of all files from a JSON file.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Oct 2021
 *
 * Copyright (C) 2021  Urs Fleisch
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
    var obj, files = {}
    var numRowsImported = 0
    var round = 0

    function doWork() {
      var rowNr
      if (files) {
        var filePath = app.selectionInfo.filePath
        rowNr = files[filePath]
      } else {
        rowNr = numRowsImported
      }

      if (typeof rowNr !== "undefined" && rowNr >= 0 && rowNr < obj.data.length) {
        var row = obj.data[rowNr]
        for (var frameName in row) {
          var frameValue = row[frameName]
          if (frameName !== "File Path" && frameName !== "Duration" &&
              frameValue !== "") {
            if (frameName.substr(0, 2) === "v1") {
              frameName = frameName.substr(2)
              app.setFrame(tagv1, frameName, frameValue)
            } else if (frameName.substr(0, 2) === "v3") {
              frameName = frameName.substr(2)
              app.setFrame(Frame.TagV3, frameName, frameValue)
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
        "Import", app.dirName, "JSON files (*.json);;All files (*)", false)
      if (!importPath) {
        Qt.quit()
        return
      }
    }
    var txt = "" + script.readFile(importPath)
    obj = JSON.parse(txt);
    if (obj && obj.data && obj.data.length > 0) {
      console.log("Read tags for %1 files from %2".
                  arg(obj.data.length).arg(importPath))
      for (var i = 0; i < obj.data.length; ++i) {
        var filePath = obj.data[i].filePath
        if (filePath) {
          files[filePath] = i
        }
      }
      if (Object.keys(files).length === 0) {
        console.log("No File Path column found, importing unconditionally.")
        files = undefined
      }

      if (!isStandalone() && app.hasGui()) {
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
