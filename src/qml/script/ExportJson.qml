/**
 * \file ExportJson.qml
 * Export all tags of all files to a JSON file.
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
    var obj = {data: []}

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
      if (app.selectionInfo.tag(Frame.Tag_3).tagFormat) {
        var tagsV3 = app.getAllFrames(Frame.TagV3)
        if (typeof tags === "undefined") {
          tags = {}
        }
        for (prop in tagsV3) {
          tags["v3" + prop] = tagsV3[prop]
        }
      }
      if (tags) {
        obj.data.push(tags)
        tags["File Path"] = app.selectionInfo.filePath
      }

      if (!app.nextFile()) {
        var txt = JSON.stringify(obj)
        var exportPath = getArguments()[0]
        if (!exportPath) {
          exportPath = app.selectFileName(
            "Export", app.dirName + "/export.json",
            "JSON files (*.json);;All files (*)", true)
          if (!exportPath) {
            Qt.quit()
            return
          }
        }
        if (script.writeFile(exportPath, txt)) {
          console.log("Exported tags of %1 files to %2".
                      arg(obj.data.length).arg(exportPath))
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

    if (!isStandalone() && app.hasGui()) {
      console.log("Expanding file list")
      app.expandFileListFinished.connect(startWork)
      app.requestExpandFileList()
    } else {
      startWork()
    }
  }
}
