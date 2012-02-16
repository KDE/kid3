/**
 * \file dirrenamer.cpp
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

#include "dirrenamer.h"
#include <QFileInfo>
#include <QDir>
#include "qtcompatmac.h"
#include "saferename.h"

/**
 * Constructor.
 * @param parent parent object
 */
DirRenamer::DirRenamer(QObject* parent) : QObject(parent), m_aborted(false)
{
  setObjectName("DirRenamer");
}

/**
 * Destructor.
 */
DirRenamer::~DirRenamer()
{
}

/**
 * Get parent directory.
 *
 * @param dir directory
 *
 * @return parent directory (terminated by separator),
 *         empty string if no separator in dir.
 */
static QString parentDirectory(const QString& dir)
{
  QString parent(dir);
  int slashPos = parent.lastIndexOf('/');
  if (slashPos != -1) {
    parent.truncate(slashPos + 1);
  } else {
    parent = "";
  }
  return parent;
}

/** Only defined for generation of translation files */
#define CREATE_DIR_FAILED_FOR_PO I18N_NOOP("Create directory %1 failed\n")

/**
 * Create a directory if it does not exist.
 *
 * @param dir      directory path
 * @param errorMsg if not NULL and an error occurred, a message is appended here,
 *                 otherwise it is not touched
 *
 * @return true if directory exists or was created successfully.
 */
bool DirRenamer::createDirectory(const QString& dir,
                   QString* errorMsg) const
{
  if (QFileInfo(dir).isDir() ||
    (QDir().mkdir(dir) && QFileInfo(dir).isDir())) {
    return true;
  } else {
    if (errorMsg) {
      errorMsg->append(i18n("Create directory %1 failed\n").arg(dir));
    }
    return false;
  }
}

/** Only defined for generation of translation files */
#define FILE_ALREADY_EXISTS_FOR_PO I18N_NOOP("File %1 already exists\n")
/** Only defined for generation of translation files */
#define IS_NOT_DIR_FOR_PO I18N_NOOP("%1 is not a directory\n")
/** Only defined for generation of translation files */
#define RENAME_FAILED_FOR_PO I18N_NOOP("Rename %1 to %2 failed\n")

/**
 * Rename a directory.
 *
 * @param olddir   old directory name
 * @param newdir   new directory name
 * @param errorMsg if not NULL and an error occurred, a message is
 *                 appended here, otherwise it is not touched
 *
 * @return true if rename successful.
 */
bool DirRenamer::renameDirectory(
  const QString& olddir, const QString& newdir, QString* errorMsg) const
{
  if (QFileInfo(newdir).exists()) {
    if (errorMsg) {
      errorMsg->append(i18n("File %1 already exists\n").arg(newdir));
    }
    return false;
  }
  if (!QFileInfo(olddir).isDir()) {
    if (errorMsg) {
      errorMsg->append(i18n("%1 is not a directory\n").arg(olddir));
    }
    return false;
  }
  if (Utils::safeRename(olddir, newdir) && QFileInfo(newdir).isDir()) {
    return true;
  } else {
    if (errorMsg) {
      errorMsg->append(i18n("Rename %1 to %2 failed\n").arg(olddir).arg(newdir));
    }
    return false;
  }
}

/** Only defined for generation of translation files */
#define ALREADY_EXISTS_FOR_PO I18N_NOOP("%1 already exists\n")
/** Only defined for generation of translation files */
#define IS_NOT_FILE_FOR_PO I18N_NOOP("%1 is not a file\n")

/**
 * Rename a file.
 *
 * @param oldfn    old file name
 * @param newfn    new file name
 * @param errorMsg if not NULL and an error occurred, a message is
 *                 appended here, otherwise it is not touched
 *
 * @return true if rename successful or newfn already exists.
 */
bool DirRenamer::renameFile(const QString& oldfn, const QString& newfn,
                QString* errorMsg) const
{
  if (QFileInfo(newfn).isFile()) {
    return true;
  }
  if (QFileInfo(newfn).exists()) {
    if (errorMsg) {
      errorMsg->append(i18n("%1 already exists\n").arg(newfn));
    }
    return false;
  }
  if (!QFileInfo(oldfn).isFile()) {
    if (errorMsg) {
      errorMsg->append(i18n("%1 is not a file\n").arg(oldfn));
    }
    return false;
  }
  if (Utils::safeRename(oldfn, newfn) && QFileInfo(newfn).isFile()) {
    return true;
  } else {
    if (errorMsg) {
      errorMsg->append(i18n("Rename %1 to %2 failed\n").arg(oldfn).arg(newfn));
    }
    return false;
  }
}

/**
 * Generate new directory name according to current settings.
 *
 * @param taggedFile file to get information from
 * @param olddir pointer to QString to place old directory name into,
 *               NULL if not used
 *
 * @return new directory name.
 */
QString DirRenamer::generateNewDirname(TaggedFile* taggedFile, QString* olddir)
{
  taggedFile->readTags(false);
  TrackData trackData(*taggedFile, m_tagVersion);
  QString newdir(taggedFile->getDirname());
#ifdef WIN32
  newdir.replace('\\', '/');
#endif
  if (newdir.endsWith(QChar('/'))) {
    // remove trailing separator
    newdir.truncate(newdir.length() - 1);
  }
  if (olddir) {
    *olddir = newdir;
  }
  if (!trackData.isEmptyOrInactive()) {
    if (!m_actionCreate) {
      newdir = parentDirectory(newdir);
    } else if (!newdir.isEmpty()) {
      newdir.append('/');
    }
    newdir.append(trackData.formatFilenameFromTags(m_format, true));
  }
  return newdir;
}

/**
 * Clear the rename actions.
 * This method has to be called before scheduling new actions.
 */
void DirRenamer::clearActions()
{
  m_actions.clear();
  m_aborted = false;
}

/**
 * Add a rename action.
 *
 * @param type type of action
 * @param src  source file or directory name
 * @param dest destination file or directory name
 */
void DirRenamer::addAction(RenameAction::Type type, const QString& src, const QString& dest)
{
  // do not add an action if the source or destination is already in an action
  for (RenameActionList::const_iterator it = m_actions.begin();
       it != m_actions.end();
       ++it) {
    if ((!src.isEmpty() && (*it).m_src == src) ||
        (!dest.isEmpty() && (*it).m_dest == dest)){
      return;
    }
  }

  m_actions.push_back(RenameAction(type, src, dest));
}

/**
 * Add a rename action.
 *
 * @param type type of action
 * @param dest destination file or directory name
 */
void DirRenamer::addAction(RenameAction::Type type, const QString& dest)
{
  addAction(type, QString(), dest);
}

/**
 * Check if there is already an action scheduled for this source.
 *
 * @return true if a rename action for the source exists.
 */
bool DirRenamer::actionHasSource(const QString& src) const
{
  if (src.isEmpty()) {
    return false;
  }
  for (RenameActionList::const_iterator it = m_actions.begin();
       it != m_actions.end();
       ++it) {
    if ((*it).m_src == src) {
      return true;
    }
  }
  return false;
}

/**
 * Check if there is already an action scheduled for this destination.
 *
 * @return true if a rename or create action for the destination exists.
 */
bool DirRenamer::actionHasDestination(const QString& dest) const
{
  if (dest.isEmpty()) {
    return false;
  }
  for (RenameActionList::const_iterator it = m_actions.begin();
       it != m_actions.end();
       ++it) {
    if ((*it).m_dest == dest) {
      return true;
    }
  }
  return false;
}

/**
 * Replace directory name if there is already a rename action.
 *
 * @param src directory name, will be replaced if there is a rename action
 */
void DirRenamer::replaceIfAlreadyRenamed(QString& src) const
{
  bool found = true;
  for (int i = 0; found && i <  5; ++i) {
    found = false;
    for (RenameActionList::const_iterator it = m_actions.begin();
         it != m_actions.end();
         ++it) {
      if ((*it).m_type == RenameAction::RenameDirectory &&
          (*it).m_src == src) {
        src = (*it).m_dest;
        found = true;
        break;
      }
    }
  }
}

/**
 * Schedule the actions necessary to rename the directory containing a file.
 *
 * @param taggedFile file in directory
 */
void DirRenamer::scheduleAction(TaggedFile* taggedFile)
{
  QString currentDirname;
  QString newDirname(generateNewDirname(taggedFile, &currentDirname));
  bool again = false;
  for (int round = 0; round < 2; ++round) {
    replaceIfAlreadyRenamed(currentDirname);
    if (newDirname != currentDirname) {
      if (newDirname.startsWith(currentDirname + '/')) {
        // A new directory is created in the current directory.
        bool createDir = true;
        QString dirWithFiles(currentDirname);
        for (int i = 0;
             createDir && newDirname.startsWith(currentDirname) && i < 5;
             i++) {
          QString newPart(newDirname.mid(currentDirname.length()));
          // currentDirname does not end with a separator, so newPart
          // starts with a separator and the search starts with the
          // second character.
          int slashPos = newPart.indexOf('/', 1);
          if (slashPos != -1 && slashPos != (int)newPart.length() - 1) {
            newPart.truncate(slashPos);
            // the new part has multiple directories
            // => create one directory
          } else {
            createDir = false;
          }
          // Create a directory for each file and move it.
          addAction(RenameAction::CreateDirectory, currentDirname + newPart);
          if (!createDir) {
            addAction(RenameAction::RenameFile,
                      dirWithFiles + '/' + taggedFile->getFilename(),
                      currentDirname + newPart + '/' + taggedFile->getFilename());
          }
          currentDirname = currentDirname + newPart;
        }
      } else {
        QString parent(parentDirectory(currentDirname));
        if (newDirname.startsWith(parent)) {
          QString newPart(newDirname.mid(parent.length()));
          int slashPos = newPart.indexOf('/');
          if (slashPos != -1 && slashPos != (int)newPart.length() - 1) {
            newPart.truncate(slashPos);
            // the new part has multiple directories
            // => rename current directory, then create additional
            // directories.
            again = true;
          }
          QString parentWithNewPart = parent + newPart;
          if ((QFileInfo(parentWithNewPart).isDir() &&
               !actionHasSource(parentWithNewPart)) ||
              actionHasDestination(parentWithNewPart)) {
            // directory already exists => move files
            addAction(RenameAction::RenameFile,
                      currentDirname + '/' + taggedFile->getFilename(),
                      parentWithNewPart + '/' + taggedFile->getFilename());
            currentDirname = parentWithNewPart;
          } else {
            addAction(RenameAction::RenameDirectory, currentDirname, parentWithNewPart);
            currentDirname = parentWithNewPart;
          }
        } else {
          // new directory name is too different
          addAction(RenameAction::ReportError, i18n("New directory name is too different\n"));
        }
      }
    }
    if (!again) break;
  }
}

/**
 * Perform the scheduled rename actions.
 *
 * @param errorMsg if not 0 and an error occurred, a message is appended here,
 *                 otherwise it is not touched
 */
void DirRenamer::performActions(QString* errorMsg)
{
  for (RenameActionList::const_iterator it = m_actions.begin();
       it != m_actions.end();
       ++it) {
    switch ((*it).m_type) {
      case RenameAction::CreateDirectory:
        createDirectory((*it).m_dest, errorMsg);
        break;
      case RenameAction::RenameDirectory:
        if (renameDirectory((*it).m_src, (*it).m_dest, errorMsg)) {
          if ((*it).m_src == m_dirName) {
            m_dirName = (*it).m_dest;
          }
        }
        break;
      case RenameAction::RenameFile:
        renameFile((*it).m_src, (*it).m_dest, errorMsg);
        break;
      case RenameAction::ReportError:
      default:
        if (errorMsg) {
          *errorMsg += (*it).m_dest;
        }
    }
  }
}

/**
 * Get description of actions to be performed.
 * @return list of (action, [src,] dst) lists describing the actions to be
 * performed.
 */
QList<QStringList> DirRenamer::describeActions() const
{
  static const char* const typeStr[] = {
    I18N_NOOP("Create directory"),
    I18N_NOOP("Rename directory"),
    I18N_NOOP("Rename file"),
    I18N_NOOP("Error")
  };
  static const unsigned numTypeStr = sizeof(typeStr) / sizeof(typeStr[0]);

  QList<QStringList> lst;
  for (RenameActionList::const_iterator it = m_actions.constBegin();
       it != m_actions.constEnd();
       ++it) {
    QStringList actionStrs;
    unsigned typeIdx = static_cast<unsigned>((*it).m_type);
    if (typeIdx >= numTypeStr) {
      typeIdx = numTypeStr - 1;
    }
    actionStrs.append(QCM_translate(typeStr[typeIdx]));
    if (!(*it).m_src.isEmpty()) {
      actionStrs.append((*it).m_src);
    }
    actionStrs.append((*it).m_dest);
    lst.append(actionStrs);
  }
  return lst;
}
