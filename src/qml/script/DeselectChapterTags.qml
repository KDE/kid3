/**
 * \file DeselectChapterTags.qml
 * Deselect chapter tag frames to exclude them from operations (e.g. copying
 * or deletion).
 *
 * \b Project: Kid3
 * \author Molly Messner
 * \date 7 Feb 2025
 *
 * Copyright (C) 2025  Molly Messner
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
import "imports/ChapterTags.js" as ChapterTags

Kid3Script {
  onRun: {
    ChapterTags.setSelected_allChapterTags(false);
    Qt.quit();
  }
}
