/**
 * \file tagsearcher.h
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

#ifndef TAGSEARCHER_H
#define TAGSEARCHER_H

#include <QObject>
#include <QString>
#include <QRegularExpression>
#include <QPersistentModelIndex>
#include "iabortable.h"
#include "frame.h"
#include "kid3api.h"

class FileProxyModel;
class BiDirFileProxyModelIterator;
class TaggedFile;

/**
 * Searcher for strings in tags.
 */
class KID3_CORE_EXPORT TagSearcher : public QObject, public IAbortable {
  Q_OBJECT
public:
  /**
   * Position of found string.
   */
  class KID3_CORE_EXPORT Position {
  public:
    /** Part of file where string was found. */
    enum Part {
      FileName, /**< Found in file name */
      Tag1,     /**< Found in tag 1 */
      Tag2,     /**< Found in tag 2 */
      Tag3      /**< Found in tag 3 */
    };

    /**
     * Constructor.
     */
    Position();

    /**
     * Clear to invalid position.
     */
    void clear();

    /**
     * Check if position is valid.
     * @return true if valid, false if not found.
     */
    bool isValid() const;

    /**
     * Get model index of tagged file.
     * @return model index.
     */
    QPersistentModelIndex getFileIndex() const { return m_fileIndex; }

    /**
     * Get part in file where string was found.
     * @return part.
     */
    Part getPart() const { return m_part; }

    /**
     * Get index of frame where string was found.
     * @return number of frame in frame list (tag 1 or tag 2),
     * -1 if string was not found in a tag.
     */
    int getFrameIndex() const { return m_frameIndex; }

    /**
     * Get name of frame where string was found.
     * @return name of frame, empty if string was not found in a tag.
     */
    QString getFrameName() const { return m_frameName; }

    /**
     * Get starting position of match.
     * @return starting position of match in tag frame value or file name,
     * -1 if not found.
     */
    int getMatchedPos() const { return m_matchedPos; }

    /**
     * Get length of match.
     * @return number of matched characters, -1 if not found.
     */
    int getMatchedLength() const { return m_matchedLength; }

    /**
     * Convert part in file where string was found to tag number.
     * @return tag number, Frame::Tag_NumValues if FileName.
     */
    static Frame::TagNumber partToTagNumber(Part part) {
      return part == FileName
          ? Frame::Tag_NumValues : static_cast<Frame::TagNumber>(part - 1);
    }

    /**
     * Convert tag number to part in file where string was found.
     * @return part, FileName if Frame::Tag_NumValues.
     */
    static Part tagNumberToPart(Frame::TagNumber tagNr) {
      return tagNr < Frame::Tag_NumValues
          ? static_cast<Part>(tagNr + 1) : FileName;
    }

  private:
    friend class TagSearcher;

    QString m_frameName;
    QPersistentModelIndex m_fileIndex;
    Part m_part;
    int m_frameIndex;
    int m_matchedPos;
    int m_matchedLength;
  };

  /** Flags controlling search */
  enum SearchFlag {
    CaseSensitive = 1 << 0, /**< is case sensitive */
    Backwards     = 1 << 1, /**< search backwards */
    RegExp        = 1 << 2, /**< use regular expressions */
    AllFrames     = 1 << 3  /**< search in all frames */
  };
  Q_DECLARE_FLAGS(SearchFlags, SearchFlag)

  /**
   * Search parameters.
   */
  class KID3_CORE_EXPORT Parameters {
  public:
    /**
     * Constructor.
     */
    Parameters() : m_frameMask(0), m_flags(AllFrames) {}

    /**
     * Get search text.
     * @return text.
     */
    QString getSearchText() const { return m_searchText; }

    /**
     * Set search text.
     * @param text search text
     */
    void setSearchText(const QString& text) { m_searchText = text; }

    /**
     * Get search text.
     * @return text.
     */
    QString getReplaceText() const { return m_replaceText; }

    /**
     * Set search text.
     * @param text search text
     */
    void setReplaceText(const QString& text) { m_replaceText = text; }

    /**
     * Get search flags.
     * @return flags.
     */
    SearchFlags getFlags() const { return m_flags; }

    /**
     * Set search flags.
     * @param flags flags
     */
    void setFlags(SearchFlags flags) { m_flags = flags; }

    /**
     * Get mask with bits set for frames to be searched.
     * @return mask containing bits corresponding to Frame::Type and
     * additionally TrackDataModel::FT_FileName.
     */
    quint64 getFrameMask() const { return m_frameMask; }

    /**
     * Set mask with bits set for frames to be searched.
     * @param frameMask mask with Frame::Type and TrackDataModel::FT_FileName
     * bits
     */
    void setFrameMask(quint64 frameMask) { m_frameMask = frameMask; }

    /**
     * Get parameters as variant list.
     * @return variant list containing search text, replace text, flags,
     * frameMask.
     */
    QVariantList toVariantList() const;

    /**
     * Set parameters from variant list.
     * @param lst variant list containing search text, replace text, flags,
     * frameMask
     */
    void fromVariantList(const QVariantList& lst);

  private:
    quint64 m_frameMask;
    QString m_searchText;
    QString m_replaceText;
    SearchFlags m_flags;
  };


  /**
   * Constructor.
   * @param parent parent object
   */
  explicit TagSearcher(QObject* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~TagSearcher() override = default;

  /**
   * Clear abort flag.
   */
  virtual void clearAborted() override;

  /**
   * Check if dialog was aborted.
   * @return true if aborted.
   */
  virtual bool isAborted() const override;

  /**
   * Set model of files to be searched.
   * @param model file proxy model
   */
  void setModel(FileProxyModel* model);

  /**
   * Set root index of directory to search.
   * @param index root index of directory
   */
  void setRootIndex(const QPersistentModelIndex& index);

  /**
   * Set index of file to start search.
   * @param index index of file where search is started
   */
  void setStartIndex(const QPersistentModelIndex& index);

  /**
   * Get position of current match.
   * @return position.
   */
  const Position& getPosition() const { return m_currentPosition; }

public slots:
  /**
   * Stop current search, so that the next call to findNext() will use the
   * index set with setStartIndex().
   */
  virtual void abort() override;

  /**
   * Find next occurrence of string.
   * @param params search parameters
   */
  void find(const Parameters& params);

  /**
   * Replace found text.
   * @param params search parameters
   * @return true if replaced.
   */
  void replace(const Parameters& params);

  /**
   * Replace all occurrences.
   * @param params search parameters
   */
  void replaceAll(const TagSearcher::Parameters& params);

signals:
  /**
   * Emitted when a match is found.
   * The position of the match is available via getPosition(), it is
   * invalid when the end is reached.
   */
  void textFound();

  /**
   * Emitted when a text is replaced.
   * The position of the replaced text is available via getPosition().
   */
  void textReplaced();

  /**
   * Progress message while searching.
   * @param msg message
   */
  void progress(const QString& msg);

private slots:
  void searchNextFile(const QPersistentModelIndex& index);
  void replaceThenFindNext();

private:
  void setParameters(const Parameters& params);
  void findNext(int advanceChars);
  void replaceNext();
  void continueSearch(int advanceChars);
  bool searchInFile(TaggedFile* taggedFile, Position* pos,
                    int advanceChars) const;
  bool searchInFrames(const FrameCollection& frames,
                      Position::Part part, Position* pos,
                      int advanceChars) const;
  int findInString(const QString& str, int& idx) const;
  void replaceString(QString& str) const;
  QString getLocationString(TaggedFile* taggedFile) const;

  FileProxyModel* m_fileProxyModel;
  BiDirFileProxyModelIterator* m_iterator;
  QPersistentModelIndex m_startIndex;
  Position m_currentPosition;
  Parameters m_params;
  QRegularExpression m_regExp;
  bool m_aborted;
  bool m_started;
};

#endif // TAGSEARCHER_H
