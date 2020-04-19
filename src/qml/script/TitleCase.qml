/**
 * \file TitleCase.qml
 * Use English title case in certain tag frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 12 Apr 2020
 *
 * Copyright (C) 2020  Urs Fleisch
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
    var consideredTags = [Frame.Tag_1, Frame.Tag_2, Frame.Tag_3]
    var consideredFrames = ["title", "album"]

    var small = "(a|an|and|as|at|but|by|en|for|if|in|of|on|or|the|to|v[.]?|via|vs[.]?)";
    var punct = "([!\"#$%&'()*+,./:;<=>?@[\\\\\\]^_`{|}~-]*)";
    // This array only contains words which cannot be handled by the all lower
    // and upper case handling at the end of the toTitleCase() function.
    var unmodifiableWords = ["Pi-hsien"];

    /*
     * Title Caps
     *
     * Adapted for audio tag use case by Urs Fleisch - 12 Apr 2020
     * Ported to JavaScript By John Resig - http://ejohn.org/ - 21 May 2008
     * Original by John Gruber - https://daringfireball.net/ - 10 May 2008
     * License: https://www.opensource.org/licenses/mit-license.php
     */
    function toTitleCase(title) {
      var parts = [], split = /[:.;?!] |(?: |^)["\xab]/g, index = 0;

      while (true) {
        var m = split.exec(title);
        var part = title.substring(index, m ? m.index : title.length);

        parts.push(unmodifiableWords.indexOf(part) !== -1 ? part : part
          .replace(/\b([A-Za-z][a-z.'`()\u2019]*)\b/g,
                   function(all) {
                     return /[A-Za-z]\.[A-Za-z]/.test(all) ? all : capitalize(all);
                   })
          .replace(new RegExp("\\b" + small + "\\b", "ig"),
                   function(word) {
                     return word.toLowerCase();
                   })
          .replace(new RegExp("^" + punct + small + "\\b", "ig"),
                   function(all, punct, word) {
                     return punct + capitalize(word);
                   })
          .replace(new RegExp("([-\\u2013\\u2014]\\s+)" + punct + small + "\\b", "ig"),
                   function(all, dash, punct, word) {
                     return dash + punct + capitalize(word);
                   })
          .replace(new RegExp("\\b" + small + punct + "$", "ig"), capitalize));

        index = split.lastIndex;

        if (m) {
          parts.push(m[0]);
        } else {
          break;
        }
      }

      return parts.join("")
        .replace(/ V(s?)\. /ig, " v$1. ")
        .replace(/(['`\u2019])S\b/ig, "$1s")
        .replace(/\b(de|von|van|feat|n|http:\/\/)\b/ig, function(all) {
          return all.toLowerCase();
        })
        .replace(/\b(AT&T|Q&A|OYF’N)\b/ig, function(all) {
          return all.toUpperCase();
        });
      ;
    };

    function capitalize(word) {
      return word.substr(0, 1).toUpperCase() + word.substr(1);
    }

    function doWork() {
      for (var ti = 0; ti < consideredTags.length; ++ti) {
        var tagNr = consideredTags[ti]
        var tagMask = script.toTagVersion(1 << tagNr);
        if (app.selectionInfo.tag(tagNr).tagFormat) {
          for (var fi = 0; fi < consideredFrames.length; ++fi) {
            var name = consideredFrames[fi];
            var oldTxt = app.getFrame(tagMask, name)
            var newTxt = toTitleCase(oldTxt)
            if (newTxt !== oldTxt) {
              app.setFrame(tagMask, name, newTxt)
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
