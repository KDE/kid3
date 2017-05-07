/**
 * \file ReplayGain2SoundCheck.qml
 * Create iTunNORM SoundCheck information from replay gain frames.
 * Replay gain tags can be generated e.g. using "mp3gain -s i".
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
    function doWork() {
      if (app.selectionInfo.tag(Frame.Tag_2).tagFormat) {
        var rgStr = app.getFrame(tagv2, "replaygain_track_gain")
        if (rgStr) {
          var rg = parseFloat(rgStr)
          if (!isNaN(rg)) {
            // Calculate SoundCheck value and insert a sequence of 10
            // hex zero-padded to 8 digits values into the iTunNORM frame.
            var scVal = parseInt(Math.pow(10, -rg / 10) * 1000).toString(16)
            var pad = "00000000"
            scVal = " " + (pad + scVal).slice(-pad.length)
            var sc = new Array(11).join(scVal)
            console.log("Set iTunNORM to %1 in %2".
                        arg(sc).arg(app.selectionInfo.fileName))
            app.setFrame(tagv2, "iTunNORM", sc)
          } else {
            console.log("Value %1 is not a float in %2".
                        arg(rgStr).arg(app.selectionInfo.fileName))
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
