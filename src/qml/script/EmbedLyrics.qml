/**
 * \file EmbedLyrics.qml
 * Fetch unsynchronised lyrics from web service.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Mar 2015
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
    var lyricFetchers = {
      "makeitpersonal": {
        getUrl: function(artist, title) {
          return "https://makeitpersonal.co/lyrics?artist=%1&title=%2".
                   arg(artist).arg(title)
        },
        parseResponse: function(resp) {
          return (resp && resp.substring(0, 5) !== "Sorry") ? resp : ""
        }
      },
      "lyrics.wikia.com": {
        getUrl: function(artist, title) {
          return "https://lyrics.wikia.com/wiki/" + encodeURIComponent(artist) +
                   ":" + encodeURIComponent(title)
        },
        parseResponse: function(resp) {
          function replaceHtmlEntities(str) {
            str = str.replace(/<br[ \/]*>/gmi, "\n");
            str = str.replace(/<\/?\w(?:[^"'>]|"[^"]*"|'[^']*')*>/gmi, "");
            str = str.replace(/&#\d+;/gm, function(s) {
              return String.fromCharCode(parseInt(s.substring(
                2, s.length - 1)))
            })
            return str
          }

          var begin = resp.indexOf("<div class='lyricbox'>")
          if (begin !== -1) {
            begin += 22
            var end = resp.indexOf("<!--", begin)
            var txt = resp.substring(begin, end)
            if (txt.substring(0, 2) === "&#") {
              txt = replaceHtmlEntities(txt)
              return txt
            }
          }
          return ""
        }
      }
    }
    var usedFetchers = ["makeitpersonal", "lyrics.wikia.com"]
    var currentFetcherIdx = 0

    function toNextFile() {
      if (!nextFile()) {
        if (isStandalone()) {
          // Save the changes if the script is started stand-alone, not from Kid3.
          app.saveDirectory()
        }
        Qt.quit()
      } else {
        currentFetcherIdx = 0
        setTimeout(doWork, 1)
      }
    }

    function toNextFetcher() {
      if (++currentFetcherIdx < usedFetchers.length) {
        setTimeout(doWork, 1)
      } else {
        toNextFile()
      }
    }

    function doWork() {
      if (app.selectionInfo.tag(Frame.Tag_2).tagFormat ||
          app.selectionInfo.tag(Frame.Tag_1).tagFormat) {
        var artist = app.getFrame(tagv2, "artist") || app.getFrame(tagv1, "artist")
        var title = app.getFrame(tagv2, "title") || app.getFrame(tagv1, "title")
        var doc = new XMLHttpRequest();
        var name = usedFetchers[currentFetcherIdx]
        var fetcher = lyricFetchers[name]
        doc.onreadystatechange = function() {
          if (doc.readyState === XMLHttpRequest.DONE) {
            if (doc.status === 200) {
              var txt = doc.responseText.trim()
              txt = fetcher.parseResponse(txt)
              if (txt) {
                app.setFrame(tagv2, "lyrics", txt)
                console.log("Set lyrics for %1 - %2 from %3".
                            arg(artist).arg(title).arg(name))
                toNextFile()
              } else {
                console.log("No lyrics for %1 - %2 from %3".
                            arg(artist).arg(title).arg(name))
                toNextFetcher()
              }
            } else {
              console.log("Request failed for %1 - %2 from %3".
                          arg(artist).arg(title).arg(name))
              toNextFetcher()
            }
          }
        }

        doc.open("GET", fetcher.getUrl(artist, title))
        doc.send()
      } else {
        toNextFile()
      }
    }

    firstFile()
    doWork()
  }
}
