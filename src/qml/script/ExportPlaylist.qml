/**
 * \file ExportPlaylistFolder.qml
 * Export files found in playlist to a new folder.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 07 Sep 2018
 *
 * Copyright (C) 2018  Urs Fleisch
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
    var playlistItems, exportItems, exportPlaylistPath, itemNr

    function getPlaylistPath() {
      var paths = isStandalone()
          ? getArguments().slice(1) : app.getSelectedFilePaths(false)
      for (var i = 0, len = paths.length; i < len; i++) {
        var path = paths[i]
        var dotPos = path.lastIndexOf(".")
        if (dotPos !== -1) {
          var ext = path.substring(dotPos)
          if ([".m3u", ".pls", ".xspf"].indexOf(ext) !== -1) {
            return path
          }
        }
      }
      return null
    }

    function getFileName(path) {
      var slashPos = path.lastIndexOf("/")
      return slashPos !== -1 ? path.substring(slashPos + 1) : path
    }

    function createPathLists() {
      var playlistPath = getPlaylistPath()
      if (!playlistPath) {
        console.error("No playlist file selected")
        return false
      }
      console.log("Reading playlist", playlistPath)
      playlistItems = app.getPlaylistItems(playlistPath)
      var numPlaylistItems = playlistItems.length
      if (numPlaylistItems <= 0) {
        return false
      }
      var exportPath = getArguments()[0]
      if (!exportPath) {
        exportPath = app.selectDirName(
          "Select Destination Folder", app.dirName)
      }
      if (!exportPath) {
        return false
      }
      if (exportPath[exportPath.length - 1] !== "/") {
        exportPath += "/"
      }

      // Create path to playlist in export directory.
      exportPlaylistPath = exportPath + getFileName(playlistPath)
      if (script.fileExists(exportPlaylistPath)) {
        console.error("File already exists:", exportPlaylistPath)
        return false
      }

      // Create the paths of the files in the exported playlist.
      exportItems = []
      var numDigits = Math.max(
          2, Math.floor(Math.log(exportItems.length) / Math.log(10)) + 1)
      var zeros = ""
      for (var i = 0; i < numDigits; i++) {
        zeros += "0"
      }
      for (i = 0; i < numPlaylistItems; i++) {
        // Make sure that the file name starts with the number
        // of the position in the playlist.
        var nr = (zeros + (i + 1)).slice(-numDigits)
        var fileName = getFileName(playlistItems[i])
        fileName = fileName.replace(/^\d+/, nr)
        if (fileName.substring(0, numDigits) !== nr) {
          fileName = nr + " " + fileName
        }

        var filePath = exportPath + fileName
        if (script.fileExists(filePath)) {
          console.error("File already exists:", filePath)
          return false
        }

        exportItems.push(filePath)
      }
      return true
    }

    function doWork() {
      if (itemNr < playlistItems.length) {
        // Copy the next file.
        var src = playlistItems[itemNr]
        var dst = exportItems[itemNr]
        console.log("Copy " + src + " to " + dst)
        if (!script.copyFile(src, dst)) {
          console.error("Failed to copy")
          Qt.quit()
        } else {
          ++itemNr
          setTimeout(doWork, 1)
        }
      } else {
        // Finally create a playlist in the export folder.
        app.setPlaylistItems(exportPlaylistPath, exportItems)
        Qt.quit()
      }
    }

    if (createPathLists()) {
      itemNr = 0
      doWork()
    } else {
      Qt.quit()
    }
  }
}
