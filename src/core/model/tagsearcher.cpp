/**
 * \file tagsearcher.cpp
 * Search for strings in tags.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 08 Feb 2014
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

#include "tagsearcher.h"
#include "trackdatamodel.h"
#include "fileproxymodel.h"
#include "bidirfileproxymodeliterator.h"

/**
 * Constructor.
 */
TagSearcher::Position::Position() :
  m_part(FileName), m_frameIndex(-1), m_matchedPos(-1), m_matchedLength(-1)
{
}

/**
 * Clear to invalid position.
 */
void TagSearcher::Position::clear()
{
  m_fileIndex = QPersistentModelIndex();
  m_frameName.clear();
  m_frameIndex = -1;
  m_matchedPos = -1;
  m_matchedLength = -1;
}

/**
 * Check if position is valid.
 * @return true if valid, false if not found.
 */
bool TagSearcher::Position::isValid() const
{
  return m_fileIndex.isValid() && m_matchedPos != -1;
}


/**
 * Constructor.
 * @param parent parent object
 */
TagSearcher::TagSearcher(QObject* parent) : QObject(parent),
  m_fileProxyModel(0), m_iterator(0), m_aborted(false), m_started(false)
{
}

/**
 * Destructor.
 */
TagSearcher::~TagSearcher()
{
}

/**
 * Clear abort flag.
 */
void TagSearcher::clearAborted()
{
  m_aborted = false;
}

/**
 * Check if dialog was aborted.
 * @return true if aborted.
 */
bool TagSearcher::isAborted() const
{
  return m_aborted;
}

/**
 * Set model of files to be searched.
 * @param model file proxy model
 */
void TagSearcher::setModel(FileProxyModel* model)
{
  if (m_iterator && m_fileProxyModel != model) {
    delete m_iterator;
    m_iterator = 0;
  }
  m_fileProxyModel = model;
  if (m_fileProxyModel && !m_iterator) {
    m_iterator = new BiDirFileProxyModelIterator(m_fileProxyModel, this);
    connect(m_iterator, SIGNAL(nextReady(QPersistentModelIndex)),
            this, SLOT(searchNextFile(QPersistentModelIndex)));
  }
}

/**
 * Set root index of directory to search.
 * @param index root index of directory
 */
void TagSearcher::setRootIndex(const QPersistentModelIndex& index)
{
  m_iterator->setRootIndex(index);
}

/**
 * Set index of file to start search.
 * @param index index of file where search is started
 */
void TagSearcher::setStartIndex(const QPersistentModelIndex& index)
{
  m_startIndex = index;
}

/**
 * Set abort flag.
 */
void TagSearcher::abort()
{
  m_aborted = true;
  m_started = false;
  if (m_iterator) {
    m_iterator->abort();
  }
}

/**
 * Find next occurrence of string.
 * @param params search parameters
 */
void TagSearcher::find(const Parameters &params)
{
  setParameters(params);
  findNext(1);
}

/**
 * Find next occurrence of same string.
 */
void TagSearcher::findNext(int advanceChars)
{
  m_aborted = false;
  if (m_iterator) {
    if (m_started) {
      continueSearch(advanceChars);
    } else {
      if (m_startIndex.isValid()) {
        m_iterator->setCurrentIndex(m_startIndex);
        m_startIndex = QPersistentModelIndex();
      }
      m_started = true;
      m_iterator->start();
    }
  }
}

/**
 * Search next file.
 * @param index index of file in file proxy model
 */
void TagSearcher::searchNextFile(const QPersistentModelIndex& index)
{
  if (index.isValid()) {
    if (TaggedFile* taggedFile = FileProxyModel::getTaggedFileOfIndex(index)) {
      emit progress(taggedFile->getFilename());
      taggedFile->readTags(false);
      taggedFile = FileProxyModel::readWithId3V24IfId3V24(taggedFile);

      Position pos;
      if (searchInFile(taggedFile, &pos, 1)) {
        pos.m_fileIndex = index;
        m_currentPosition = pos;
        if (m_iterator) {
          m_iterator->suspend();
        }
        emit progress(getLocationString(taggedFile));
        emit textFound();
      }
    }
  } else {
    m_started = false;
    m_currentPosition.clear();
    emit progress(tr("Search finished"));
    emit textFound();
  }
}

/**
 * Continue search in current file, if no other match is found, resume
 * file iteration.
 * @param advanceChars number of characters to advance from current position
 */
void TagSearcher::continueSearch(int advanceChars)
{
  if (m_currentPosition.isValid()) {
    if (TaggedFile* taggedFile =
       FileProxyModel::getTaggedFileOfIndex(m_currentPosition.getFileIndex())) {
      if (searchInFile(taggedFile, &m_currentPosition, advanceChars)) {
        emit progress(getLocationString(taggedFile));
        emit textFound();
        return;
      }
    }
  }
  if (m_iterator) {
    m_iterator->resume();
  }
}

/**
 * Search for next occurrence in a file.
 * @param taggedFile tagged file
 * @param pos position of last match in @a taggedFile, will be updated
 * with new position
 * @param advanceChars number of characters to advance from current position
 * @return true if found.
 */
bool TagSearcher::searchInFile(TaggedFile* taggedFile, Position* pos,
                               int advanceChars) const
{
  if (pos->getPart() <= Position::FileName &&
      ((m_params.getFlags() & AllFrames) ||
       (m_params.getFrameMask() & (1ULL << TrackDataModel::FT_FileName)))) {
    int idx = 0;
    if (pos->getPart() == Position::FileName) {
      idx = pos->m_matchedPos + advanceChars;
    }
    int len = findInString(taggedFile->getFilename(), idx);
    if (len != -1) {
      pos->m_part = Position::FileName;
      pos->m_matchedPos = idx;
      pos->m_matchedLength = len;
      return true;
    }
  }
  if (pos->getPart() <= Position::Tag1) {
    FrameCollection frames;
    taggedFile->getAllFramesV1(frames);
    if (searchInFrames(frames, Position::Tag1, pos, advanceChars)) {
      return true;
    }
  }
  if (pos->getPart() <= Position::Tag2) {
    FrameCollection frames;
    taggedFile->getAllFramesV2(frames);
    if (searchInFrames(frames, Position::Tag2, pos, advanceChars)) {
      return true;
    }
  }
  return false;
}

/**
 * Search for next occurrence in frames.
 * @param frames frames of tag
 * @param part tag 1 or tag 2
 * @param pos position of last match, will be updated with new position
 * @param advanceChars number of characters to advance from current position
 * @return true if found.
 */
bool TagSearcher::searchInFrames(const FrameCollection& frames,
                                 Position::Part part, Position* pos,
                                 int advanceChars) const
{
  int idx = 0;
  int frameNr = 0;
  FrameCollection::const_iterator begin = frames.begin();
  FrameCollection::const_iterator end = frames.end();
  if (pos->getPart() == part) {
    idx = pos->m_matchedPos + advanceChars;
    for (frameNr = 0;
         frameNr < pos->getFrameIndex() && begin != end; ++frameNr) {
      ++begin;
    }
  }
  int len = -1;
  QString frameName;
  for (FrameCollection::const_iterator it = begin; it != end; ++it, ++frameNr) {
    if ((m_params.getFlags() & AllFrames) ||
        (m_params.getFrameMask() & (1ULL << it->getType()))) {
      len = findInString(it->getValue(), idx);
      if (len != -1) {
        frameName = it->getExtendedType().getTranslatedName();
        break;
      }
    }
    idx = 0;
  }
  if (len != -1) {
    pos->m_part = part;
    pos->m_frameName = frameName;
    pos->m_frameIndex = frameNr;
    pos->m_matchedPos = idx;
    pos->m_matchedLength = len;
    return true;
  }
  return false;
}

/**
 * Replace found text.
 * @param params search parameters
 */
void TagSearcher::replace(const TagSearcher::Parameters& params)
{
  setParameters(params);
  replaceNext();
}

/**
 * Replace found text.
 */
void TagSearcher::replaceNext()
{
  QString replaced;
  if (m_currentPosition.isValid()) {
    if (TaggedFile* taggedFile =
        FileProxyModel::getTaggedFileOfIndex(m_currentPosition.getFileIndex())) {
      switch (m_currentPosition.getPart()) {
      case Position::FileName:
      {
        QString str = taggedFile->getFilename();
        replaced = str.mid(m_currentPosition.getMatchedPos(),
                           m_currentPosition.getMatchedLength());
        replaceString(replaced);
        str.replace(m_currentPosition.getMatchedPos(),
                    m_currentPosition.getMatchedLength(), replaced);
        taggedFile->setFilename(str);
        break;
      }
      case Position::Tag1:
      case Position::Tag2:
      {
        FrameCollection frames;
        if (m_currentPosition.getPart() == Position::Tag1) {
          taggedFile->getAllFramesV1(frames);
        } else {
          taggedFile->getAllFramesV2(frames);
        }
        FrameCollection::iterator it = frames.begin();
        FrameCollection::iterator end = frames.end();
        for (int frameNr = 0;
             frameNr < m_currentPosition.getFrameIndex() && it != end;
             ++frameNr) {
          ++it;
        }
        if (it != end) {
          Frame& frame = const_cast<Frame&>(*it);
          QString str = frame.getValue();
          replaced = str.mid(m_currentPosition.getMatchedPos(),
                             m_currentPosition.getMatchedLength());
          replaceString(replaced);
          str.replace(m_currentPosition.getMatchedPos(),
                      m_currentPosition.getMatchedLength(), replaced);
          frame.setValueIfChanged(str);
          if (m_currentPosition.getPart() == Position::Tag1) {
            taggedFile->setFramesV1(frames);
          } else {
            taggedFile->setFramesV2(frames);
          }
        }
        break;
      }
      }
    }
  }
  if (!replaced.isNull()) {
    emit textReplaced();
    findNext(replaced.length());
  } else {
    findNext(1);
  }
}

/**
 * Replace all occurrences.
 * @param params search parameters
 */
void TagSearcher::replaceAll(const TagSearcher::Parameters& params)
{
  setParameters(params);
  disconnect(this, SIGNAL(textFound()), this, SLOT(replaceThenFindNext()));
  connect(this, SIGNAL(textFound()), this, SLOT(replaceThenFindNext()),
          Qt::QueuedConnection);
  replaceNext();
}

/**
 * If a text is found replace it and then search the next occurrence.
 */
void TagSearcher::replaceThenFindNext()
{
  if (!m_aborted && m_currentPosition.isValid()) {
    replaceNext();
  } else {
    disconnect(this, SIGNAL(textFound()), this, SLOT(replaceThenFindNext()));
  }
}

/**
 * Search string for text.
 * @param str string to be searched
 * @param idx start index of search, will be updated with index of found text
 * @return length of match if found, else -1.
 */
int TagSearcher::findInString(const QString& str, int& idx) const
{
  if (m_regExp.isEmpty()) {
    idx = str.indexOf(m_params.getSearchText(), idx,
                      m_params.getFlags() & CaseSensitive
                      ? Qt::CaseSensitive : Qt::CaseInsensitive);
    return idx != -1 ? m_params.getSearchText().length() : -1;
  } else {
    idx = m_regExp.indexIn(str, idx);
    return idx != -1 ? m_regExp.matchedLength() : -1;
  }
}

/**
 * Replace string.
 * @param str string which will be replaced
 */
void TagSearcher::replaceString(QString& str) const
{
  if (m_regExp.isEmpty()) {
    str.replace(m_params.getSearchText(), m_params.getReplaceText(),
                m_params.getFlags() & CaseSensitive
                ? Qt::CaseSensitive : Qt::CaseInsensitive);
  } else {
    str.replace(m_regExp, m_params.getReplaceText());
  }
}

/**
 * Set and preprocess search parameters.
 * @param params search parameters
 */
void TagSearcher::setParameters(Parameters params)
{
  m_params = params;
  SearchFlags flags = m_params.getFlags();
  if (m_iterator) {
    m_iterator->setDirectionBackwards(flags & Backwards);
  }
  if (flags & RegExp) {
    m_regExp = QRegExp(m_params.getSearchText(), flags & CaseSensitive
                       ? Qt::CaseSensitive : Qt::CaseInsensitive);
  } else {
    m_regExp = QRegExp();
  }
}

/**
 * Get a string describing where the text was found.
 * @param taggedFile tagged file
 * @return description of location.
 */
QString TagSearcher::getLocationString(TaggedFile* taggedFile) const
{
  QString location = taggedFile->getFilename();
  location += QLatin1String(": ");
  switch (m_currentPosition.getPart()) {
  case Position::FileName:
    location += tr("Filename");
    break;
  case Position::Tag1:
  case Position::Tag2:
    location += m_currentPosition.getPart() == Position::Tag1
        ? tr("Tag 1") : tr("Tag 2");
    location += QLatin1String(": ");
    location += m_currentPosition.getFrameName();
    break;
  }
  return location;
}
