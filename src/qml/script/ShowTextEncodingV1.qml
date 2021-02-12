/**
 * \file ShowTextEncodingV1.qml
 * Helps to find the encoding of ID3v1 tags by showing the tags of the
 * current file in all available character encodings.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 06 Mar 2015
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
    // Apply the encoding by changing the configuration, applying the changes,
    // and then force rereading the tags by deselecting and selecting the
    // current file.
    function applyEncoding(encNr) {
      tagCfg.textEncodingV1Index = encNr
      app.applyChangedConfiguration()
      app.selectCurrentFile(false)
      app.unloadAllTags()
      app.selectCurrentFile(true)
    }

    if (app.selectionInfo.tag(Frame.Tag_1).tagFormat) {
      var frameNames = ["title", "artist", "album", "comment"]
      var tagCfg = configs.tagConfig()
      var textEncodings = tagCfg.getTextCodecNames()
      var textEncodingV1Index = tagCfg.textEncodingV1Index
      var maxEncLen = Math.max.apply(null, textEncodings.map(
                                       function(s) { return s.length; }))
      var i
      for (var encNr = 0; encNr < textEncodings.length; ++encNr) {
        applyEncoding(encNr)
        var txt = textEncodings[encNr]
        var numSpaces = maxEncLen - txt.length
        txt += ":"
        for (i = 0; i < numSpaces; ++i) {
          txt += " "
        }
        for (i = 0; i < frameNames.length; ++i) {
          if (i > 0) {
            txt += " "
          }
          txt += app.getFrame(tagv1, frameNames[i])
        }
        console.log(txt)
      }
      applyEncoding(textEncodingV1Index)
      app.applyChangedConfiguration()
    }
    Qt.quit()
  }
}
