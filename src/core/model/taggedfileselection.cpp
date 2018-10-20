/**
 * \file taggedfileselection.h
 * Information about selected tagged files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 19 Jun 2014
 *
 * Copyright (C) 2014  Urs Fleisch
 *
 * This file is part of Kid3.
 *
 * Kid3 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Kid3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "taggedfileselection.h"
#include "taggedfile.h"
#include "trackdata.h"
#include "frametablemodel.h"
#include "fileproxymodel.h"
#include "pictureframe.h"
#include "guiconfig.h"
#include "tagconfig.h"
#include "fileconfig.h"

/**
 * Constructor.
 * @param framesModel frame table models for all tags, Frame::Tag_NumValues
 * elements
 * @param parent parent object
 */
TaggedFileSelection::TaggedFileSelection(
    FrameTableModel* framesModel[], QObject* parent) : QObject(parent)
{
  FOR_ALL_TAGS(tagNr) {
    m_framesModel[tagNr] = framesModel[tagNr];
    m_tagContext[tagNr] = new TaggedFileSelectionTagContext(this, tagNr);
  }
  setObjectName(QLatin1String("TaggedFileSelection"));
}

/**
 * Start adding tagged files to selection.
 * Has to be called before adding the first file using addTaggedFile().
 */
void TaggedFileSelection::beginAddTaggedFiles()
{
  m_lastState = m_state;
  m_state.m_singleFile = nullptr;
  m_state.m_fileCount = 0;
  FOR_ALL_TAGS(tagNr) {
    m_state.m_tagSupportedCount[tagNr] = 0;
    m_state.m_hasTag[tagNr] = false;
  }
}

/**
 * End adding tagged files to selection.
 * Has to be called after adding the last file using addTaggedFile().
 */
void TaggedFileSelection::endAddTaggedFiles()
{
  FOR_ALL_TAGS(tagNr) {
    m_framesModel[tagNr]->setAllCheckStates(
          m_state.m_tagSupportedCount[tagNr] == 1);
  }
  if (GuiConfig::instance().autoHideTags()) {
    // If a tag is supposed to be absent, make sure that there is really no
    // unsaved data in the tag.
    FOR_ALL_TAGS(tagNr) {
      if (!m_state.m_hasTag[tagNr] &&
          (m_state.m_tagSupportedCount[tagNr] > 0 ||
           m_state.m_fileCount == 0)) {
        const FrameCollection& frames = m_framesModel[tagNr]->frames();
        for (auto it = frames.cbegin(); it != frames.cend(); ++it) {
          if (!(*it).getValue().isEmpty()) {
            m_state.m_hasTag[tagNr] = true;
            break;
          }
        }
      }
    }
  }
  FOR_ALL_TAGS(tagNr) {
    if (TagConfig::instance().markTruncations()) {
      m_framesModel[tagNr]->markRows(m_state.m_singleFile
                               ? m_state.m_singleFile->getTruncationFlags(tagNr)
                               : 0);
    }
    if (FileConfig::instance().markChanges()) {
      m_framesModel[tagNr]->markChangedFrames(m_state.m_singleFile
                               ? m_state.m_singleFile->getChangedFrames(tagNr)
                               : 0);
    }
    if (m_state.hasTag(tagNr) != m_lastState.hasTag(tagNr)) {
      emit m_tagContext[tagNr]->hasTagChanged(m_state.hasTag(tagNr));
    }
    if (m_state.isTagUsed(tagNr) != m_lastState.isTagUsed(tagNr)) {
      emit m_tagContext[tagNr]->tagUsedChanged(m_state.isTagUsed(tagNr));
    }
  }

  if (m_state.isEmpty() != m_lastState.isEmpty()) {
    emit emptyChanged(m_state.isEmpty());
  }
  if (m_state.isSingleFileSelected() != m_lastState.isSingleFileSelected()) {
    emit singleFileSelectedChanged(m_state.isSingleFileSelected());
  }
  if (m_state.isSingleFileSelected() || m_lastState.isSingleFileSelected()) {
    // The properties depending on the single file may have changed.
    emit singleFileChanged();
    FOR_ALL_TAGS(tagNr) {
      emit m_tagContext[tagNr]->tagFormatChanged();
    }
  }
}

/**
 * Add a tagged file to the selection.
 * @param taggedFile tagged file
 */
void TaggedFileSelection::addTaggedFile(TaggedFile* taggedFile)
{
  taggedFile = FileProxyModel::readTagsFromTaggedFile(taggedFile);

  FOR_ALL_TAGS(tagNr) {
    if (taggedFile->isTagSupported(tagNr)) {
      if (m_state.m_tagSupportedCount[tagNr] == 0) {
        FrameCollection frames;
        taggedFile->getAllFrames(tagNr, frames);
        m_framesModel[tagNr]->transferFrames(frames);
      } else {
        FrameCollection fileFrames;
        taggedFile->getAllFrames(tagNr, fileFrames);
        m_framesModel[tagNr]->filterDifferent(fileFrames);
      }
      ++m_state.m_tagSupportedCount[tagNr];
    }
  }
  m_state.m_singleFile = m_state.m_fileCount == 0 ? taggedFile : nullptr;
  ++m_state.m_fileCount;

  FOR_ALL_TAGS(tagNr) {
    m_state.m_hasTag[tagNr] =
        m_state.m_hasTag[tagNr] || taggedFile->hasTag(tagNr);
  }
}

/**
 * Get file name.
 * @return file name if single file selected, else null string.
 */
QString TaggedFileSelection::getFilename() const
{
  return m_state.m_singleFile ? m_state.m_singleFile->getFilename() : QString();
}

/**
 * Set file name if single file selected.
 * @param fn file name
 */
void TaggedFileSelection::setFilename(const QString& fn)
{
  if (m_state.m_singleFile && !fn.isEmpty() &&
      m_state.m_singleFile->getFilename() != fn) {
    m_state.m_singleFile->setFilename(fn);
    emit fileNameModified();
  }
}

/**
 * Get file path.
 * @return absolute file path if single file selected, else null string.
 */
QString TaggedFileSelection::getFilePath() const
{
  return m_state.m_singleFile
      ? m_state.m_singleFile->getAbsFilename() : QString();
}

/**
 * Get string representation of detail information.
 * @return information summary as string if single file else null string.
 */
QString TaggedFileSelection::getDetailInfo() const
{
  TaggedFile::DetailInfo info;
  if (m_state.m_singleFile) {
    m_state.m_singleFile->getDetailInfo(info);
  }
  return info.toString();
}

/**
 * Get the format of tag.
 * @param tagNr tag number
 * @return string describing format of tag 2 if single file selected,
 * else null string.
 */
QString TaggedFileSelection::getTagFormat(Frame::TagNumber tagNr) const
{
  return m_state.m_singleFile ? m_state.m_singleFile->getTagFormat(tagNr)
                              : QString();
}

/**
 * Get the format of tag 1.
 * @return string describing format of tag 1 if single file selected,
 * else null string.
 * @deprecated Use tag(Frame::Tag_1)->tagFormat() instead.
 */
QString TaggedFileSelection::getTagFormatV1() const
{
  return m_tagContext[Frame::Tag_1]->tagFormat();
}

/**
 * Get the format of tag 2.
 * @return string describing format of tag 2 if single file selected,
 * else null string.
 * @deprecated Use tag(Frame::Tag_2)->tagFormat() instead.
 */
QString TaggedFileSelection::getTagFormatV2() const
{
  return m_tagContext[Frame::Tag_2]->tagFormat();
}

/**
 * Check if filename is changed.
 * @return true if single file selected and filename was changed.
 */
bool TaggedFileSelection::isFilenameChanged() const
{
  return m_state.m_singleFile && m_state.m_singleFile->isFilenameChanged();
}

/**
 * Get data from a picture frame.
 * @return picture data, empty if not available.
 */
QByteArray TaggedFileSelection::getPicture() const
{
  QByteArray data;
  const FrameCollection& frames = m_framesModel[Frame::Tag_Picture]->frames();
  auto it = frames.find(
        Frame(Frame::FT_Picture, QLatin1String(""), QLatin1String(""), -1));
  if (it != frames.cend() && !it->isInactive()) {
    PictureFrame::getData(*it, data);
  }
  return data;
}

/**
 * Replace codes in format string with information from the tags.
 * @param tagVersion tag version
 * @param fmt format string
 * @return string with format codes replaced.
 */
QString TaggedFileSelection::formatString(Frame::TagVersion tagVersion,
                                          const QString& fmt)
{
  if (!m_state.m_singleFile)
    return fmt;

  TrackData trackData(*m_state.m_singleFile, tagVersion);
  return trackData.formatString(fmt);
}

/**
 * Select changed frames in the tables if multiple files are selected.
 */
void TaggedFileSelection::selectChangedFrames()
{
  if (m_state.m_fileCount > 1) {
    FOR_ALL_TAGS(tagNr) {
      m_framesModel[tagNr]->selectChangedFrames();
    }
  }
}

/**
 * Clear frame collection in frame models not used by current selection.
 */
void TaggedFileSelection::clearUnusedFrames()
{
  FOR_ALL_TAGS(tagNr) {
    if (m_state.m_tagSupportedCount[tagNr] == 0) {
      m_framesModel[tagNr]->clearFrames();
    }
  }
}
