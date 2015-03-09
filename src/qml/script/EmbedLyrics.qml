/**
 * \file EmbedLyrics.qml
 * Fetch unsynchronised lyrics from web service.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Mar 2015
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
    function toNextFile() {
      if (!app.nextFile()) {
        if (isStandalone()) {
          // Save the changes if the script is started stand-alone, not from Kid3.
          app.saveDirectory()
        }
        Qt.quit()
      } else {
        setTimeout(doWork, 1)
      }
    }

    function doWork() {
      if (app.selectionInfo.tagFormatV2 || app.selectionInfo.tagFormatV1) {
        var artist = app.getFrame(tagv2, "artist") || app.getFrame(tagv1, "artist")
        var title = app.getFrame(tagv2, "title") || app.getFrame(tagv1, "title")
        var doc = new XMLHttpRequest();
        doc.onreadystatechange = function() {
          if (doc.readyState === XMLHttpRequest.DONE) {
            if (doc.status === 200) {
              var txt = doc.responseText.trim()
              if (txt && txt.substring(0, 5) !== "Sorry") {
                app.setFrame(tagv2, "lyrics", txt)
                console.log("Set lyrics for %1 - %2".arg(artist).arg(title))
              } else {
                console.log("No lyrics for %1 - %2".arg(artist).arg(title))
              }
            } else {
              console.log("Request failed for %1 - %2".arg(artist).arg(title))
            }
            toNextFile()
          }
        }

        doc.open("GET", "http://makeitpersonal.co/lyrics?artist=%1&title=%2".
                 arg(artist).arg(title));
        doc.send();
      } else {
        toNextFile()
      }
    }

    app.firstFile()
    doWork()
  }
}
