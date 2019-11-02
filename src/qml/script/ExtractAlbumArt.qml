/**
 * \file ExtractAlbumArt.qml
 * Extract all embedded cover art pictures avoiding duplicates.
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
    var lastDir
    var md5Map = {}
    var baseName = configs.fileConfig().defaultCoverFileName
    var extPos = baseName.lastIndexOf(".")
    if (extPos !== -1) {
      baseName = baseName.substr(0, extPos)
    }

    function doWork() {
      if (app.selectionInfo.tag(Frame.Tag_2).tagFormat) {
        var data = app.getPictureData()
        if (script.getDataSize(data) !== 0) {
          var fileName = app.selectionInfo.fileName
          var dirName = app.selectionInfo.filePath
          dirName = dirName.substring(0, dirName.length - fileName.length)
          if (dirName !== lastDir) {
            lastDir = dirName
            var existingImageFiles = script.listDir(dirName, ["*.png", "*.jpg"])
            for (var i = 0; i < existingImageFiles.length; ++i) {
              var filePath = dirName + existingImageFiles[i]
              var fileData = script.readFile(filePath)
              if (script.getDataSize(fileData) !== 0) {
                md5Map[script.getDataMd5(fileData)] = filePath
              }
            }
          }
          var md5 = script.getDataMd5(data)
          if (md5 in md5Map) {
            console.log("Picture in %1 already exists in %2".
                        arg(fileName).arg(md5Map[md5]))
          } else {
            var format = "jpg"
            var img = script.dataToImage(data, format)
            var imgProps = script.imageProperties(img)
            if (!("width" in imgProps)) {
              format = "png"
              img = script.dataToImage(data, format)
              imgProps = script.imageProperties(img)
            }
            if ("width" in imgProps) {
              var fileBaseName = baseName
              if (fileBaseName.indexOf("%") !== -1) {
                fileBaseName = app.importFromTagsToSelection(
                      Frame.Tag_2, baseName, "%{__return}(.+)")
              }
              var picPath = dirName + fileBaseName + "." + format
              var picNr = 1
              while (script.fileExists(picPath)) {
                ++picNr
                picPath = dirName + fileBaseName + picNr + "." + format
              }
              if (script.writeFile(picPath, data)) {
                md5Map[md5] = picPath
                console.log("Picture in %1 stored to %2".
                            arg(fileName).arg(picPath))
              } else {
                console.log("Failed to write", picPath)
              }
            }
          }
        }
      }
      if (!nextFile()) {
        Qt.quit()
      } else {
        setTimeout(doWork, 1)
      }
    }

    firstFile()
    doWork()
  }
}
