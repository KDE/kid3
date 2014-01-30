/**
 * \file dirrenamer.h
 * Directory renamer.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 23 Jul 2011
 *
 * Copyright (C) 2011  Urs Fleisch
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

#ifndef DIRRENAMER_H
#define DIRRENAMER_H

#include <QObject>
#include <QString>
#include "trackdata.h"
#include "iabortable.h"
#include "kid3api.h"

class TaggedFile;

/**
 * Directory renamer.
 */
class KID3_CORE_EXPORT DirRenamer : public QObject, public IAbortable {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param parent parent object
   */
  explicit DirRenamer(QObject* parent = 0);

  /**
   * Destructor.
   */
  virtual ~DirRenamer();

  /**
   * Check if operation is aborted.
   *
   * @return true if aborted.
   */
  virtual bool isAborted() const;

  /**
   * Clear state which is reported by isAborted().
   */
  virtual void clearAborted();

  /**
   * Abort operation.
   */
  virtual void abort();

  /**
   * Set version of tags used to get rename information.
   * @param tagVersion tag version
   */
  void setTagVersion(TrackData::TagVersion tagVersion) {
    m_tagVersion = tagVersion;
  }

  /**
   * Set action to be performed.
   * @param create true for create action
   */
  void setAction(bool create) { m_actionCreate = create; }

  /**
   * Set format to generate directory names.
   * @param format format
   */
  void setFormat(const QString& format) { m_format = format; }

  /**
   * Generate new directory name according to current settings.
   *
   * @param taggedFile file to get information from
   * @param olddir pointer to QString to place old directory name into,
   *               NULL if not used
   *
   * @return new directory name.
   */
  QString generateNewDirname(TaggedFile* taggedFile, QString* olddir);

  /**
   * Clear the rename actions.
   * This method has to be called before scheduling new actions.
   */
  void clearActions();

  /**
   * Schedule the actions necessary to rename the directory containing a file.
   *
   * @param taggedFile file in directory
   */
  void scheduleAction(TaggedFile* taggedFile);

  /**
   * Perform the scheduled rename actions.
   *
   * @param errorMsg if not 0 and an error occurred, a message is appended here,
   *                 otherwise it is not touched
   */
  void performActions(QString* errorMsg);

  /**
   * Set directory name.
   * This should be done before calling performActions(), so that the directory
   * name is changed when the application directory is renamed.
   *
   * @param dirName directory name
   */
  void setDirName(const QString& dirName) { m_dirName = dirName; }

  /**
   * Get directory name.
   * The directory name should be initialized with the value of
   * Kid3Application::getDirName() before performActions() is started and will
   * be updated if it is renamed while performing the actions. If it is
   * different from the application directory name after performActions(), the
   * new directory should be opened.
   *
   * @return directory.
   */
  QString getDirName() const { return m_dirName; }

signals:
  /**
   * Is emitted after an action has been scheduled.
   *
   * @param actionStrs description of action
   */
  void actionScheduled(const QStringList& actionStrs);

private:
  /**
   * An action performed while renaming a directory.
   */
  class RenameAction {
  public:
    /** Action type. */
    enum Type {
      CreateDirectory,
      RenameDirectory,
      RenameFile,
      ReportError,
      NumTypes
    };

    /**
     * Constructor.
     * @param type type of action
     * @param src  source file or directory name
     * @param dest destination file or directory name
     * @param index model index of item to rename
     */
    RenameAction(Type type, const QString& src, const QString& dest,
                 const QPersistentModelIndex& index) :
      m_type(type), m_src(src), m_dest(dest), m_index(index) {}

    /**
     * Constructor.
     */
    RenameAction() : m_type(ReportError) {}

    /**
     * Destructor.
     */
    ~RenameAction() {}

    /**
     * Test for equality.
     * @param rhs right hand side
     * @return true if equal.
     */
    bool operator==(const RenameAction& rhs) const {
      return m_type == rhs.m_type && m_src == rhs.m_src && m_dest == rhs.m_dest;
    }

    Type m_type;    /**< type of action */
    QString m_src;  /**< source file or directory name */
    QString m_dest; /**< destination file or directory name */
    QPersistentModelIndex m_index; /**< model index of item to rename */
  };

  /** List of rename actions. */
  typedef QList<RenameAction> RenameActionList;

  /**
   * Create a directory if it does not exist.
   *
   * @param dir      directory path
   * @param errorMsg if not NULL and an error occurred, a message is appended here,
   *                 otherwise it is not touched
   *
   * @return true if directory exists or was created successfully.
   */
  bool createDirectory(const QString& dir, QString* errorMsg) const;

  /**
   * Rename a directory.
   *
   * @param olddir   old directory name
   * @param newdir   new directory name
   * @param errorMsg if not NULL and an error occurred, a message is
   *                 appended here, otherwise it is not touched
   * @param index    model index of item to rename
   *
   * @return true if rename successful.
   */
  bool renameDirectory(
    const QString& olddir, const QString& newdir,
    const QPersistentModelIndex& index, QString *errorMsg) const;

  /**
   * Rename a file.
   *
   * @param oldfn    old file name
   * @param newfn    new file name
   * @param errorMsg if not NULL and an error occurred, a message is
   *                 appended here, otherwise it is not touched
   * @param index    model index of item to rename
   *
   * @return true if rename successful or newfn already exists.
   */
  bool renameFile(const QString& oldfn, const QString& newfn,
                  const QPersistentModelIndex& index, QString* errorMsg) const;

  /**
   * Add a rename action.
   *
   * @param type type of action
   * @param src  source file or directory name
   * @param dest destination file or directory name
   * @param index model index of item to rename
   */
  void addAction(RenameAction::Type type, const QString& src,
                 const QString& dest,
                 const QPersistentModelIndex& index = QPersistentModelIndex());

  /**
   * Add a rename action.
   *
   * @param type type of action
   * @param dest destination file or directory name
   */
  void addAction(RenameAction::Type type, const QString& dest);

  /**
   * Check if there is already an action scheduled for this source.
   *
   * @return true if a rename action for the source exists.
   */
  bool actionHasSource(const QString& src) const;

  /**
   * Check if there is already an action scheduled for this destination.
   *
   * @return true if a rename or create action for the destination exists.
   */
  bool actionHasDestination(const QString& dest) const;

  /**
   * Replace directory name if there is already a rename action.
   *
   * @param src directory name, will be replaced if there is a rename action
   */
  void replaceIfAlreadyRenamed(QString& src) const;

  /**
   * Get description of an actions to be performed.
   * @return (action, [src,] dst) list describing the action to be
   * performed.
   */
  QStringList describeAction(const RenameAction& action) const;

  RenameActionList m_actions;
  TrackData::TagVersion m_tagVersion;
  QString m_format;
  QString m_dirName;
  bool m_aborted;
  bool m_actionCreate;
};

#endif // DIRRENAMER_H
