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
#include "frame.h"
#include "frametablemodel.h"
#include "fileproxymodel.h"
#include "pictureframe.h"
#include "guiconfig.h"
#include "tagconfig.h"
#include "fileconfig.h"

/**
 * Constructor.
 * @param framesV1Model frame table model for tag 1
 * @param framesV2Model frame table model for tag 1
 * @param parent parent object
 */
TaggedFileSelection::TaggedFileSelection(
    FrameTableModel* framesV1Model, FrameTableModel* framesV2Model,
    QObject* parent) : QObject(parent),
  m_framesV1Model(framesV1Model), m_framesV2Model(framesV2Model)
{
  setObjectName(QLatin1String("TaggedFileSelection"));
}

/**
 * Destructor.
 */
TaggedFileSelection::~TaggedFileSelection()
{
}

/**
 * Start adding tagged files to selection.
 * Has to be called before adding the first file using addTaggedFile().
 */
void TaggedFileSelection::beginAddTaggedFiles()
{
  m_lastState = m_state;
  m_state.m_singleFile = 0;
  m_state.m_tagV1SupportedCount = 0;
  m_state.m_fileCount = 0;
  m_state.m_hasTagV1 = false;
  m_state.m_hasTagV2 = false;
}

/**
 * End adding tagged files to selection.
 * Has to be called after adding the last file using addTaggedFile().
 */
void TaggedFileSelection::endAddTaggedFiles()
{
  m_framesV1Model->setAllCheckStates(m_state.m_tagV1SupportedCount == 1);
  m_framesV2Model->setAllCheckStates(m_state.m_fileCount == 1);
  if (GuiConfig::instance().autoHideTags()) {
    // If a tag is supposed to be absent, make sure that there is really no
    // unsaved data in the tag.
    if (!m_state.m_hasTagV1 &&
        (m_state.m_tagV1SupportedCount > 0 || m_state.m_fileCount == 0)) {
      const FrameCollection& frames = m_framesV1Model->frames();
      for (FrameCollection::const_iterator it = frames.begin();
           it != frames.end();
           ++it) {
        if (!(*it).getValue().isEmpty()) {
          m_state.m_hasTagV1 = true;
          break;
        }
      }
    }
    if (!m_state.m_hasTagV2) {
      const FrameCollection& frames = m_framesV2Model->frames();
      for (FrameCollection::const_iterator it = frames.begin();
           it != frames.end();
           ++it) {
        if (!(*it).getValue().isEmpty()) {
          m_state.m_hasTagV2 = true;
          break;
        }
      }
    }
  }

  if (m_state.m_singleFile) {
    if (TagConfig::instance().markTruncations()) {
      m_framesV1Model->markRows(m_state.m_singleFile->getTruncationFlags());
    }
    if (FileConfig::instance().markChanges()) {
      m_framesV1Model->markChangedFrames(
        m_state.m_singleFile->getChangedFramesV1());
      m_framesV2Model->markChangedFrames(
        m_state.m_singleFile->getChangedFramesV2());
    }
  } else {
    if (TagConfig::instance().markTruncations()) {
      m_framesV1Model->markRows(0);
    }
    if (FileConfig::instance().markChanges()) {
      m_framesV1Model->markChangedFrames(0);
      m_framesV2Model->markChangedFrames(0);
    }
  }

  if (m_state.isEmpty() != m_lastState.isEmpty()) {
    emit emptyChanged(m_state.isEmpty());
  }
  if (m_state.hasTagV1() != m_lastState.hasTagV1()) {
    emit hasTagV1Changed(m_state.hasTagV1());
  }
  if (m_state.hasTagV2() != m_lastState.hasTagV2()) {
    emit hasTagV2Changed(m_state.hasTagV2());
  }
  if (m_state.isSingleFileSelected() != m_lastState.isSingleFileSelected()) {
    emit singleFileSelectedChanged(m_state.isSingleFileSelected());
  }
  if (m_state.isTag1Used() != m_lastState.isTag1Used()) {
    emit tag1UsedChanged(m_state.isTag1Used());
  }
  if (m_state.isSingleFileSelected() || m_lastState.isSingleFileSelected()) {
    // The properties depending on the single file may have changed.
    emit singleFileChanged();
  }
}

/**
 * Add a tagged file to the selection.
 * @param taggedFile tagged file
 */
void TaggedFileSelection::addTaggedFile(TaggedFile* taggedFile)
{
  taggedFile = FileProxyModel::readTagsFromTaggedFile(taggedFile);

  if (taggedFile->isTagV1Supported()) {
    if (m_state.m_tagV1SupportedCount == 0) {
      FrameCollection frames;
      taggedFile->getAllFramesV1(frames);
      m_framesV1Model->transferFrames(frames);
    } else {
      FrameCollection fileFrames;
      taggedFile->getAllFramesV1(fileFrames);
      m_framesV1Model->filterDifferent(fileFrames);
    }
    ++m_state.m_tagV1SupportedCount;
  }
  if (m_state.m_fileCount == 0) {
    FrameCollection frames;
    taggedFile->getAllFramesV2(frames);
    m_framesV2Model->transferFrames(frames);
    m_state.m_singleFile = taggedFile;
  } else {
    FrameCollection fileFrames;
    taggedFile->getAllFramesV2(fileFrames);
    m_framesV2Model->filterDifferent(fileFrames);
    m_state.m_singleFile = 0;
  }
  ++m_state.m_fileCount;

  m_state.m_hasTagV1 = m_state.m_hasTagV1 || taggedFile->hasTagV1();
  m_state.m_hasTagV2 = m_state.m_hasTagV2 || taggedFile->hasTagV2();
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
  if (m_state.m_singleFile && !fn.isEmpty()) {
    m_state.m_singleFile->setFilename(fn);
  }
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
 * Get the format of tag 1.
 * @return string describing format of tag 1 if single file selected,
 * else null string.
 */
QString TaggedFileSelection::getTagFormatV1() const
{
  return m_state.m_singleFile ? m_state.m_singleFile->getTagFormatV1()
                              : QString();
}

/**
 * Get the format of tag 1.
 * @return string describing format of tag 1 if single file selected,
 * else null string.
 */
QString TaggedFileSelection::getTagFormatV2() const
{
  return m_state.m_singleFile ? m_state.m_singleFile->getTagFormatV2()
                              : QString();
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
  const FrameCollection& frames = m_framesV2Model->frames();
  FrameCollection::const_iterator it = frames.find(
        Frame(Frame::FT_Picture, QLatin1String(""), QLatin1String(""), -1));
  if (it != frames.end() && !it->isInactive()) {
    PictureFrame::getData(*it, data);
  }
  return data;
}

/**
 * Select changed frames in the tables if multiple files are selected.
 */
void TaggedFileSelection::selectChangedFrames()
{
  if (m_state.m_fileCount > 1) {
    m_framesV1Model->selectChangedFrames();
    m_framesV2Model->selectChangedFrames();
  }
}

/**
 * Clear frame collection in frame models not used by current selection.
 */
void TaggedFileSelection::clearUnusedFrames()
{
  if (m_state.m_tagV1SupportedCount == 0) {
    m_framesV1Model->clearFrames();
  }
  if (m_state.m_fileCount == 0) {
    m_framesV2Model->clearFrames();
  }
}
