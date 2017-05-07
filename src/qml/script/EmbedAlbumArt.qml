/**
 * \file EmbedAlbumArt.qml
 * Embed cover art found in image files into audio files in the same folder.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 08 Mar 2015
 *
 * Copyright (C) 2015-2017  Urs Fleisch
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

import Kid3 1.1

Kid3Script {
  onRun: {
    var lastDir, picName, picData
    var baseName = configs.fileConfig().defaultCoverFileName

    function doWork() {
      if (app.selectionInfo.tag(Frame.Tag_2).tagFormat ||
          app.selectionInfo.tag(Frame.Tag_1).tagFormat) {
        var fileName = app.selectionInfo.fileName
        var dirName = app.selectionInfo.filePath
        dirName = dirName.substring(0, dirName.length - fileName.length)
        if (dirName !== lastDir) {
          lastDir = dirName
          var existingImageFiles = script.listDir(dirName, ["*.png", "*.jpg"])
          picName = ""
          picData = null
          if (existingImageFiles.indexOf(baseName) !== -1) {
            picName = baseName
          } else if (existingImageFiles.length > 0) {
            picName = existingImageFiles[0]
          }
          if (picName) {
            picName = dirName + picName
            picData = script.readFile(picName)
            if (script.getDataSize(picData) === 0) {
              picData = null
            }
          }
        }
        if (picData) {
          if (script.getDataSize(app.getPictureData()) === 0) {
            app.setPictureData(picData)
            console.log("%1: Embedding picture from %2".
                        arg(fileName).arg(picName))
          } else {
            console.log(fileName + ": Already contains picture")
          }
        }
      }
      if (!nextFile()) {
        if (isStandalone()) {
          // Save the changes if the script is started stand-alone, not from Kid3.
          app.saveDirectory()
        }
        Qt.quit()
      } else {
        setTimeout(doWork, 1)
      }
    }

    firstFile()
    doWork()
  }
}
