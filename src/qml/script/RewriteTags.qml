/**
 * \file RewriteTags.qml
 * Rewrite all tags in the selected files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 30 Jan 2021
 *
 * Copyright (C) 2021  Urs Fleisch
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

    function doWork() {
      for (var ti = 0; ti < consideredTags.length; ++ti) {
        var tagNr = consideredTags[ti]
        var tagMask = script.toTagVersion(1 << tagNr)
        if (app.selectionInfo.tag(tagNr).tagFormat) {
          // The files are saved in order to remove padding and use the
          // configured ID3v2 version used for new tags.
          app.copyTags(tagMask)
          app.removeTags(tagMask)
          app.saveDirectory()
          app.pasteTags(tagMask)
          app.saveDirectory()
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
