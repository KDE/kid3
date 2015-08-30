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

#ifndef TAGGEDFILESELECTION_H
#define TAGGEDFILESELECTION_H

#include <QObject>
#include "frame.h"
#include "kid3api.h"

class FrameTableModel;
class TaggedFile;

/**
 * Information about selected tagged files.
 */
class KID3_CORE_EXPORT TaggedFileSelection : public QObject {
  Q_OBJECT
  /** true if no tagged file is selected. */
  Q_PROPERTY(bool empty READ isEmpty NOTIFY emptyChanged)
  /** true if any of the selected files has a tag 1. */
  Q_PROPERTY(bool hasTagV1 READ hasTagV1 NOTIFY hasTagV1Changed)
  /** true if any of the selected files has a tag 2. */
  Q_PROPERTY(bool hasTagV2 READ hasTagV2 NOTIFY hasTagV2Changed)
  /** true if exactly one file is selected. */
  Q_PROPERTY(bool singleFileSelected READ isSingleFileSelected
             NOTIFY singleFileSelectedChanged)
  /** true if any selected file supports tag 1. */
  Q_PROPERTY(bool tag1Used READ isTag1Used NOTIFY tag1UsedChanged)
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
  /** Format of tag 1 if single file selected, else null string. */
  Q_PROPERTY(QString tagFormatV1 READ getTagFormatV1 NOTIFY singleFileChanged)
  /** Format of tag 2 if single file selected, else null string. */
  Q_PROPERTY(QString tagFormatV2 READ getTagFormatV2 NOTIFY singleFileChanged)
  /** Picture data, empty if not available.*/
  Q_PROPERTY(QByteArray picture READ getPicture NOTIFY singleFileChanged)
public:
  /**
   * Constructor.
   * @param framesV1Model frame table model for tag 1
   * @param framesV2Model frame table model for tag 1
   * @param parent parent object
   */
  TaggedFileSelection(FrameTableModel* framesV1Model, FrameTableModel* framesV2Model,
                      QObject* parent = 0);

  /**
   * Destructor.
   */
  virtual ~TaggedFileSelection();

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
   * Check if any of the selected files has a tag 1.
   * @return true if any of the selected files has a tag 1.
   */
  bool hasTagV1() const { return m_state.hasTagV1(); }

  /**
   * Check if any of the selected files has a tag 2.
   * @return true if any of the selected files has a tag 2.
   */
  bool hasTagV2() const { return m_state.hasTagV2(); }

  /**
   * Check if a single file is selected.
   * @return true if exactly one file is selected.
   */
  bool isSingleFileSelected() const { return m_state.isSingleFileSelected(); }

  /**
   * Check if tag 1 is used.
   * @return true if any selected file supports tag 1.
   */
  bool isTag1Used() const { return m_state.isTag1Used(); }

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
   * Get the format of tag 1.
   * @return string describing format of tag 1 if single file selected,
   * else null string.
   */
  QString getTagFormatV1() const;

  /**
   * Get the format of tag 2.
   * @return string describing format of tag 2 if single file selected,
   * else null string.
   */
  QString getTagFormatV2() const;

  /**
   * Check if filename is changed.
   * @return true if single file selected and filename was changed.
   */
  bool isFilenameChanged() const;

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
                                   const QString& fmt);

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
   * Emitted when hasTagV1 changed.
   */
  void hasTagV1Changed(bool hasTag);

  /**
   * Emitted when hasTagV2 changed.
   */
  void hasTagV2Changed(bool hasTag);

  /**
   * Emitted when singleFileSelected changed.
   */
  void singleFileSelectedChanged(bool single);

  /**
   * Emitted when tag1Used changed.
   */
  void tag1UsedChanged(bool used);

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
    State() : m_singleFile(0), m_tagV1SupportedCount(0), m_fileCount(0),
      m_hasTagV1(false), m_hasTagV2(false) {}

    TaggedFile* singleFile() const { return m_singleFile; }
    bool isEmpty() const { return m_fileCount == 0; }
    bool hasTagV1() const { return m_hasTagV1; }
    bool hasTagV2() const { return m_hasTagV2; }
    bool isSingleFileSelected() const { return m_singleFile != 0; }
    bool isTag1Used() const { return m_tagV1SupportedCount > 0; }

    /** If a single file is selected, this tagged file, else 0 */
    TaggedFile* m_singleFile;
    /** Number of selected files which support tag 1 */
    int m_tagV1SupportedCount;
    /** Number of selected files */
    int m_fileCount;
    /** true if any of the selected files has a tag 1 */
    bool m_hasTagV1;
    /** true if any of the selected files has a tag 2 */
    bool m_hasTagV2;
  };

  FrameTableModel* const m_framesV1Model;
  FrameTableModel* const m_framesV2Model;
  State m_state;
  State m_lastState;
};

#endif // TAGGEDFILESELECTION_H
