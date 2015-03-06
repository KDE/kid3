/**
 * \file ResizeAlbumArt.qml
 * Resize embedded cover art images which are larger than 500x500 pixels.
 * The maximum size can be adapted by changing the maxPixels variable.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 28 Feb 2015
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
    var tempPath = script.tempPath()
    var picPath = tempPath + "/kid3img"
    var maxPixels = 500

    function doWork() {
      if (app.selectionInfo.tagFormatV2) {
        var frames = app.getAllFrames(tagv2)
        if ("Picture" in frames) {
          script.removeFile(picPath)
          var desc = app.getFrame(tagv2, "Picture:" + picPath)
          var img = script.loadImage(picPath)
          var imgProps = script.imageProperties(img)
          var width = imgProps.width, height = imgProps.height
          if (width > maxPixels || height > maxPixels) {
            if (width >= height) {
              width = maxPixels; height = -1
            } else {
              width = -1; height = maxPixels
            }
            img = script.scaleImage(img, width, height)
            imgProps = script.imageProperties(img)
            if (script.saveImage(img, picPath)) {
              app.setFrame(tagv2, "Picture:" + picPath, desc)
              console.log("Resized image to %1x%2 in %3".
                          arg(imgProps.width).arg(imgProps.height).
                          arg(app.selectionInfo.fileName))
            } else {
              console.log("Failed to save " + picPath + " for scaled image in "
                          + app.selectionInfo.fileName)
            }
          }
        }
      }
      if (!app.nextFile()) {
        script.removeFile(picPath)
        if (isStandalone()) {
          // Save the changes if the script is started stand-alone, not from Kid3.
          app.saveDirectory()
        }
        Qt.quit()
      } else {
        setTimeout(doWork, 1)
      }
    }

    app.firstFile()
    doWork()
  }
}
