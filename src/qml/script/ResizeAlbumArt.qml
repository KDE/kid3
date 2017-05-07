/**
 * \file ResizeAlbumArt.qml
 * Resize embedded cover art images which are larger than 500x500 pixels.
 * The maximum size can be adapted by changing the maxPixels variable.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 28 Feb 2015
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
    var maxPixels = 500

    function doWork() {
      if (app.selectionInfo.tag(Frame.Tag_2).tagFormat) {
        var data = app.getPictureData()
        if (script.getDataSize(data) !== 0) {
          var format = "JPG"
          var img = script.dataToImage(data, format)
          var imgProps = script.imageProperties(img)
          if (typeof imgProps.width === "undefined") {
            format = "PNG"
            img = script.dataToImage(data, format)
            imgProps = script.imageProperties(img)
          }
          var width = imgProps.width, height = imgProps.height
          if (width > maxPixels || height > maxPixels) {
            if (width >= height) {
              width = maxPixels; height = -1
            } else {
              width = -1; height = maxPixels
            }
            img = script.scaleImage(img, width, height)
            imgProps = script.imageProperties(img)
            data = script.dataFromImage(img, format)
            if (script.getDataSize(data) !== 0) {
              app.setPictureData(data)
              console.log("Resized image to %1x%2 in %3".
                          arg(imgProps.width).arg(imgProps.height).
                          arg(app.selectionInfo.fileName))
            }
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
