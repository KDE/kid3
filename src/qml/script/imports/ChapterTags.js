/**
 * \file ChapterTags.js
 * Select or deselect chapter tag frames to include them in or exclude them
 * from operations (e.g. copying or deletion).
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

// In the case that a frame occurs multiple times, we have to select/
// deselect every instance of it individually.
function setSelectedForAllFramesWithKey(key, selected) {
  for (let i = 0; app.getFrame(tagv2, `${key}[${i}]`); i++) {
    app.setFrame(tagv2, `${key}[${i}].selected`, selected);
  }
}

function setSelected_id3ChapterTags(selected) {
  app.setFrame(tagv2, 'Chapters.selected', selected);
  setSelectedForAllFramesWithKey('CTOC', selected);
  setSelectedForAllFramesWithKey('CHAP', selected);
}

function setSelected_mp4ChapterTags(selected) {
  // Only relevant with the MP4v2 plugin enabled, at the time of writing.
  app.setFrame(tagv2, 'Chapters.selected', selected);
}

function setSelected_vorbisCommentChapterTags(selected) {
  // VorbisComment doesn't actually use Chapters as of the time of writing,
  // but this future-proofs this script for the potential change.
  app.setFrame(tagv2, 'Chapters.selected', selected);

  for (const key of Object.keys(app.getAllFrames(tagv2))) {
    if (/^CHAPTER\d{3}(NAME|URL)?$/i.test(key)) {
      setSelectedForAllFramesWithKey(key, selected);
    }
  }
}

function setSelected_allChapterTags(selected) {
  let tagFormat = app.selectionInfo.tag(Frame.Tag_2).tagFormat;

  switch (true) {
    case tagFormat.startsWith('ID3v2.'):
      setSelected_id3ChapterTags(selected);
      break;
    case tagFormat === 'MP4':
      setSelected_mp4ChapterTags(selected);
      break;
    case tagFormat === 'Vorbis':
      setSelected_vorbisCommentChapterTags(selected);
      break;
  }
}
