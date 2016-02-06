/**
 * \file ExportHtmlPlayer.qml
 * Export HTML file to have a player.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Aug 2015
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
    function storeTags(src, dst) {
      for (var prop in src) {
        var val = src[prop]
        if (val) {
          var key = prop.toLowerCase()
          if (key === "track number") {
            key = "track"
          } else if (key === "date") {
            key = "year"
          }
          dst[key] = val
        }
      }
    }

    function splitFileName(fileName) {
      var dotPos = fileName.lastIndexOf(".")
      var ext = ""
      if (dotPos !== -1) {
        ext = fileName.substring(dotPos + 1)
        fileName = fileName.substring(0, dotPos)
      }
      return [fileName, ext]
    }

    var template =
'<!DOCTYPE html>\n' +
'<html>\n' +
'  <head>\n' +
'    <title class="title"></title>\n' +
'    <meta charset="utf-8" />\n' +
'  </head>\n' +
'  <body>\n' +
'    <div id="player">\n' +
'      <button class="arrow" id="prev-button">&#8676;</button>\n' +
'      <button class="arrow" id="next-button">&#8677;</button>\n' +
'      <audio class="current-src" preload="true" autoplay controls></audio>\n' +
'    </div>\n' +
'    <div id="track-data"></div>\n' +
'\n' +
'    <script type="text/javascript">\n' +
'var trackData = %{trackdata};\n' +
'\n' +
'var audio = document.getElementsByTagName("audio")[0];\n' +
'var prevButton = document.getElementById("prev-button");\n' +
'var nextButton = document.getElementById("next-button");\n' +
'var currentTrack = 0;\n' +
'\n' +
'function setCurrentTrackData() {\n' +
'  var data = trackData[currentTrack];\n' +
'  var elements = document.getElementsByClassName("title");\n' +
'  var i;\n' +
'  for (i = 0; i < elements.length; ++i) {\n' +
'    elements[i].innerHTML = data["title"] || "";\n' +
'  }\n' +
'  var currentFile = data["filename"];\n' +
'  elements = document.getElementsByClassName("current-src");\n' +
'  for (i = 0; i < elements.length; ++i) {\n' +
'    elements[i].setAttribute("src", currentFile);\n' +
'  }\n' +
'\n' +
'  var dataStr = "<table>";\n' +
'  for (var key in data) {\n' +
'    if (key !== "filename" && key !== "picture") {\n' +
'      dataStr += "<tr><td>";\n' +
'      dataStr += key;\n' +
'      dataStr += "</td><td><b>";\n' +
'      dataStr += data[key];\n' +
'      dataStr += "</b></td></tr>\\n";\n' +
'    }\n' +
'  }\n' +
'  dataStr += "</table>";\n' +
'  document.getElementById("track-data").innerHTML = dataStr;\n' +
'}\n' +
'\n' +
'function toPrevious() {\n' +
'  if (currentTrack > 0) {\n' +
'    --currentTrack;\n' +
'    setCurrentTrackData();\n' +
'  }\n' +
'}\n' +
'\n' +
'function toNext() {\n' +
'  if (currentTrack + 1 < trackData.length) {\n' +
'    ++currentTrack;\n' +
'    setCurrentTrackData();\n' +
'  }\n' +
'}\n' +
'\n' +
'function loadUrl(newLocation) {\n' +
'	window.location = newLocation;\n' +
'	return false;\n' +
'}\n' +
'\n' +
'audio.addEventListener("ended", function() {\n' +
'  audio.pause();\n' +
'  audio.currentTime=0;\n' +
'  toNext();\n' +
'});\n' +
'\n' +
'prevButton.addEventListener("click", toPrevious, false);\n' +
'nextButton.addEventListener("click", toNext, false);\n' +
'document.addEventListener("DOMContentLoaded", setCurrentTrackData, false);\n' +
'    </script>\n' +
'    <style type="text/css">\n' +
'      #player {\n' +
'        display: flex;\n' +
'      }\n' +
'      #player button {\n' +
'        margin: 0 5px 0 0;\n' +
'      }\n' +
'    </style>\n' +
'  </body>\n' +
'</html>\n'

    var trackData = []
    var dirName = "";
    
    function doWork() {
      var fileName = app.selectionInfo.fileName
      var baseNameExt = splitFileName(fileName)
      var baseName = baseNameExt[0]
      if (!dirName && fileName) {
        dirName = app.selectionInfo.filePath
        dirName = dirName.substring(0, dirName.length - fileName.length)
      }

      var tags = {}
      if (app.selectionInfo.tag(Frame.Tag_1).tagFormat) {
        storeTags(app.getAllFrames(tagv1), tags)
      }
      if (app.selectionInfo.tag(Frame.Tag_2).tagFormat) {
        storeTags(app.getAllFrames(tagv2), tags)
      }
      var hasTags = Object.keys(tags).length > 0
      if (hasTags) {
        tags["filename"] = fileName
        trackData.push(tags)
      }
      if (!app.nextFile()) {
        var contents = template.replace("%{trackdata}",
                                        JSON.stringify(trackData))
        script.writeFile(dirName + "index.html", contents)
        Qt.quit()
      } else {
        setTimeout(doWork, 1)
      }
    }

    app.firstFile()
    doWork()
  }
}
