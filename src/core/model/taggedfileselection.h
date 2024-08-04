/**
 * \file taggedfileselection.h
 * Information about selected tagged files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 19 Jun 2014
 *
 * Copyright (C) 2014-2024  Urs Fleisch
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

#pragma once

#include <QObject>
#include "frame.h"
#include "kid3api.h"

class FrameTableModel;
class PictureFrame;
class TaggedFile;
class TaggedFileSelectionTagContext;

/**
 * Information about selected tagged files.
 */
class KID3_CORE_EXPORT TaggedFileSelection : public QObject {
  Q_OBJECT
  /** true if no tagged file is selected. */
  Q_PROPERTY(bool empty READ isEmpty NOTIFY emptyChanged)
  /** true if exactly one file is selected. */
  Q_PROPERTY(bool singleFileSelected READ isSingleFileSelected
             NOTIFY singleFileSelectedChanged)
  /** true if single file selected and filename was changed. */
  Q_PROPERTY(bool fileNameChanged READ isFilenameChanged
             NOTIFY singleFileChanged)
  /** File name if single file selected, else null string. */
  Q_PROPERTY(QString fileName READ getFilename WRITE setFilename
             NOTIFY singleFileChanged)
  /** Absolute file path if single file selected, else null string. */
  Q_PROPERTY(QString filePath READ getFilePath NOTIFY singleFileChanged)
  /** Detail information if single file selected, else null string. */
  Q_PROPERTY(QString detailInfo READ getDetailInfo NOTIFY singleFileChanged)
  /**
   * Format of tag 1 if single file selected, else null string.
   * @deprecated For compatibility with old scripts,
   * use tag(Frame.Tag_1).tagFormat instead.
   */
  Q_PROPERTY(QString tagFormatV1 READ getTagFormatV1 NOTIFY singleFileChanged)
  /**
   * Format of tag 2 if single file selected, else null string.
   * @deprecated For compatibility with old scripts,
   * use tag(Frame.Tag_2).tagFormat instead.
   */
  Q_PROPERTY(QString tagFormatV2 READ getTagFormatV2 NOTIFY singleFileChanged)
  /** Picture data, empty if not available.*/
  Q_PROPERTY(QByteArray picture READ getPicture NOTIFY singleFileChanged)
public:
  /**
   * Constructor.
   * @param framesModel frame table models for all tags, Frame::Tag_NumValues
   * elements
   * @param parent parent object
   */
  TaggedFileSelection(FrameTableModel* framesModel[], QObject* parent = nullptr);

  /**
   * Destructor.
   */
  ~TaggedFileSelection() override = default;

  /**
   * Start adding tagged files to selection.
   * Has to be called before adding the first file using addTaggedFile().
   */
  void beginAddTaggedFiles();

  /**
   * End adding tagged files to selection.
   * Has to be called after adding the last file using addTaggedFile().
   */
  void endAddTaggedFiles();

  /**
   * Add a tagged file to the selection.
   * @param taggedFile tagged file
   */
  void addTaggedFile(TaggedFile* taggedFile);

  /**
   * Check if a single file is selected.
   * @return if a single file is selected, this tagged file, else 0.
   */
  TaggedFile* singleFile() const { return m_state.singleFile(); }

  /**
   * Check if selection is empty.
   * @return true if no tagged file is selected.
   */
  bool isEmpty() const { return m_state.isEmpty(); }

  /**
   * Check if any of the selected files has a tag.
   * @param tagNr tag number
   * @return true if any of the selected files has a tag.
   */
  bool hasTag(Frame::TagNumber tagNr) const { return m_state.hasTag(tagNr); }

  /**
   * Check if a single file is selected.
   * @return true if exactly one file is selected.
   */
  bool isSingleFileSelected() const { return m_state.isSingleFileSelected(); }

  /**
   * Check if tag is used.
   * @param tagNr tag number
   * @return true if any selected file supports tag.
   */
  bool isTagUsed(Frame::TagNumber tagNr) const { return m_state.isTagUsed(tagNr); }

  /**
   * Get file name.
   * @return file name if single file selected, else null string.
   */
  QString getFilename() const;

  /**
   * Set file name if single file selected.
   * @param fn file name
   */
  void setFilename(const QString& fn);

  /**
   * Get file path.
   * @return absolute file path if single file selected, else null string.
   */
  QString getFilePath() const;

  /**
   * Get string representation of detail information.
   * @return information summary as string if single file else null string.
   */
  QString getDetailInfo() const;

  /**
   * Get the format of tag.
   * @param tagNr tag number
   * @return string describing format of tag 2 if single file selected,
   * else null string.
   */
  QString getTagFormat(Frame::TagNumber tagNr) const;

  /**
   * Get context for tag.
   * @param tagNr tag number
   * @return tag context.
   */
  Q_INVOKABLE TaggedFileSelectionTagContext* tag(Frame::TagNumber tagNr) const {
    return m_tagContext[tagNr];
  }

  /**
   * Check if filename is changed.
   * @return true if single file selected and filename was changed.
   */
  bool isFilenameChanged() const;

  /**
   * Get picture frames.
   * @return pictures, empty if not available.
   */
  QList<PictureFrame> getPictures() const;

  /**
   * Get data from a picture frame.
   * @return picture data, empty if not available.
   */
  QByteArray getPicture() const;

  /**
   * Replace codes in format string with information from the tags.
   * @param tagVersion tag version
   * @param fmt format string
   * @return string with format codes replaced.
   */
  Q_INVOKABLE QString formatString(Frame::TagVersion tagVersion,
                                   const QString& fmt) const;

  /**
   * Select changed frames in the tables if multiple files are selected.
   */
  void selectChangedFrames();

  /**
   * Clear frame collection in frame models not used by current selection.
   */
  void clearUnusedFrames();

signals:
  /**
   * Emitted when empty changed.
   */
  void emptyChanged(bool empty);

  /**
   * Emitted when singleFileSelected changed.
   */
  void singleFileSelectedChanged(bool single);

  /**
   * Emitted when the single file may have changed.
   * This is not accurate, because the file itself may change without notice.
   * Therefore this signal is emitted at the end of endAddTaggedFiles() unless
   * no single file exists in the current and the last state.
   */
  void singleFileChanged();

  /**
   * Emitted when the file name is modified.
   */
  void fileNameModified();

private:
  struct State {
    State() : m_singleFile(nullptr), m_fileCount(0) {
      FOR_ALL_TAGS(tagNr) {
        m_tagSupportedCount[tagNr] = 0;
        m_hasTag[tagNr] = false;
      }
    }

    TaggedFile* singleFile() const { return m_singleFile; }
    bool isEmpty() const { return m_fileCount == 0; }
    bool hasTag(Frame::TagNumber tagNr) const { return m_hasTag[tagNr]; }
    bool isSingleFileSelected() const { return m_singleFile != nullptr; }
    bool isTagUsed(Frame::TagNumber tagNr) const { return m_tagSupportedCount[tagNr] > 0; }

    /** If a single file is selected, this tagged file, else 0 */
    TaggedFile* m_singleFile;
    /** Number of selected files */
    int m_fileCount;
    /** Number of selected files which support tag 1 */
    int m_tagSupportedCount[Frame::Tag_NumValues];
    /** true if any of the selected files has a tag */
    bool m_hasTag[Frame::Tag_NumValues];
  };

  QString getTagFormatV1() const;
  QString getTagFormatV2() const;

  FrameTableModel* m_framesModel[Frame::Tag_NumValues];
  TaggedFileSelectionTagContext* m_tagContext[Frame::Tag_NumValues];
  State m_state;
  State m_lastState;
};

/**
 * Facade to have a uniform interface for different tags.
 */
class KID3_CORE_EXPORT TaggedFileSelectionTagContext : public QObject {
  Q_OBJECT
  /** true if any of the selected files has a tag. */
  Q_PROPERTY(bool hasTag READ hasTag NOTIFY hasTagChanged)
  /** true if any selected file supports the tag. */
  Q_PROPERTY(bool tagUsed READ isTagUsed NOTIFY tagUsedChanged)
  /** Format of tag if single file selected, else null string. */
  Q_PROPERTY(QString tagFormat READ tagFormat NOTIFY tagFormatChanged)
public:
  /**
   * Constructor.
   * @param selection tagged file selection
   * @param tagNr tag number
   */
  TaggedFileSelectionTagContext(TaggedFileSelection* selection,
                                Frame::TagNumber tagNr)
    : QObject(selection), m_selection(selection), m_tagNr(tagNr),
      m_tagVersion(Frame::tagVersionFromNumber(tagNr)) {
  }

signals:
  /**
   * Emitted when hasTag changed.
   */
  void hasTagChanged(bool hasTag);

  /**
   * Emitted when tagUsed changed.
   */
  void tagUsedChanged(bool used);

  /**
   * Emitted when tagFormat may have changed.
   */
  void tagFormatChanged();

private:
  friend class TaggedFileSelection;

  bool hasTag() const { return m_selection->hasTag(m_tagNr); }
  bool isTagUsed() const { return m_selection->isTagUsed(m_tagNr); }
  QString tagFormat() const { return m_selection->getTagFormat(m_tagNr); }

  TaggedFileSelection* m_selection;
  const Frame::TagNumber m_tagNr;
  const Frame::TagVersion m_tagVersion;
};
