/**
 * \file dirrenamer.cpp
 * Directory renamer.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 23 Jul 2011
 *
 * Copyright (C) 2011-2022  Urs Fleisch
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
#include <QCoreApplication>
#include "trackdata.h"
#include "saferename.h"
#include "taggedfilesystemmodel.h"
#include "modeliterator.h"
#include "formatconfig.h"

/**
 * Data collected by DirNameFormatReplacer during a rename session.
 */
class DirNameFormatReplacerContext {
public:
  /**
   * Store value for aggregate function.
   * @param code aggregating code, e.g. "max-year"
   * @param value value of base code (e.g. "year")
   */
  void addValue(const QString& code, const QString& value);

  /**
   * Register the replaced directory name which still contains
   * placeholders for the aggregate codes.
   * @param dirName directory name with replacements and aggregate codes,
   *                QString() to terminate the rename session
   */
  void putDirName(const QString& dirName);

  /**
   * Get and clear the replacements for all the replacement codes
   * encountered during this rename session.
   * Shall be called at the end of the rename session
   * @return list of (directory with aggregate codes,
   *         directory with replaced aggregate codes) pairs.
   */
  QList<QPair<QString, QString>> takeReplacements();

  /**
   * Check if aggregated codes are used.
   * @return true if aggregated codes have been added using addValue().
   */
  bool hasAggregatedCodes() const { return !m_aggregatedCodes.isEmpty(); }

private:
  QString getAggregate(const QString& code) const;

  QList<QPair<QString, QString>> m_replacements;
  QHash<QString, QStringList> m_currentCodes;
  QHash<QString, QStringList> m_aggregatedCodes;
  QString m_aggregatedDirName;
};

void DirNameFormatReplacerContext::addValue(const QString& code,
                                            const QString& value)
{
  m_currentCodes[code].append(value);
}

void DirNameFormatReplacerContext::putDirName(const QString& dirName)
{
  if (m_aggregatedDirName.isEmpty()) {
    // First directory name, start aggregation.
    m_aggregatedDirName = dirName;
    m_aggregatedCodes = m_currentCodes;
  } else if (m_aggregatedDirName != dirName) {
    // New directory name, replace aggregated values and return result.
    QString replacedDirName = m_aggregatedDirName;
    for (auto it = m_aggregatedCodes.constBegin();
         it != m_aggregatedCodes.constEnd();
         ++it) {
      replacedDirName.replace(it.key(), getAggregate(it.key()));
    }
    if (replacedDirName != m_aggregatedDirName) {
      m_replacements.append({m_aggregatedDirName, replacedDirName});
    }
    m_aggregatedCodes = m_currentCodes;
    m_aggregatedDirName = dirName;
  } else {
    // Still the same directory name, keep on aggregating.
    for (auto it = m_currentCodes.constBegin();
         it != m_currentCodes.constEnd();
         ++it) {
      m_aggregatedCodes[it.key()].append(it.value());
    }
  }
  m_currentCodes.clear();
}

QList<QPair<QString, QString>> DirNameFormatReplacerContext::takeReplacements()
{
  // Terminate current directory aggregation.
  putDirName(QString());
  QList<QPair<QString, QString>> result;
  m_replacements.swap(result);
  return result;
}

QString DirNameFormatReplacerContext::getAggregate(const QString& code) const
{
  QString result;
  const QStringList values = m_aggregatedCodes.value(code);
  if (code.startsWith(QLatin1String("max-"))) {
    for (const QString& value : values) {
      if (value > result) {
        result = value;
      }
    }
  } else if (code.startsWith(QLatin1String("min-"))) {
    for (const QString& value : values) {
      if (result.isNull() || value < result) {
        result = value;
      }
    }
  } else if (code.startsWith(QLatin1String("unq-"))) {
    for (const QString& value : values) {
      if (result.isNull()) {
        result = value;
      } else if (value != result) {
        result.clear();
        break;
      }
    }
  }
  return result;
}


namespace {

/**
 * Specialized track data format replacer using context to support
 * aggregate functions.
 */
class DirNameFormatReplacer : public TrackDataFormatReplacer {
public:
  /**
   * Constructor.
   * @param context context to store aggregate data
   * @param trackData track data
   * @param str string with format codes
   */
  explicit DirNameFormatReplacer(
    DirNameFormatReplacerContext& context,
    const TrackData& trackData,
    const QString& str = QString());

  virtual ~DirNameFormatReplacer() override = default;

  DirNameFormatReplacer(const DirNameFormatReplacer& other) = delete;
  DirNameFormatReplacer &operator=(const DirNameFormatReplacer& other) = delete;

protected:
  virtual QString getReplacement(const QString& code) const override;

private:
  DirNameFormatReplacerContext& m_context;
};

DirNameFormatReplacer::DirNameFormatReplacer(
    DirNameFormatReplacerContext& context,
    const TrackData& trackData,
    const QString& str)
  : TrackDataFormatReplacer(trackData, str), m_context(context)
{
}

QString DirNameFormatReplacer::getReplacement(const QString& code) const
{
  if (code.startsWith(QLatin1String("max-")) ||
      code.startsWith(QLatin1String("min-")) ||
      code.startsWith(QLatin1String("unq-"))) {
    QString value = TrackDataFormatReplacer::getReplacement(code.mid(4));
    m_context.addValue(code, value);
    return code;
  }
  return TrackDataFormatReplacer::getReplacement(code);
}


/**
 * Get parent directory.
 *
 * @param dir directory
 *
 * @return parent directory (terminated by separator),
 *         empty string if no separator in dir.
 */
QString parentDirectory(const QString& dir)
{
  QString parent(dir);
  int slashPos = parent.lastIndexOf(QLatin1Char('/'));
  if (slashPos != -1) {
    parent.truncate(slashPos + 1);
  } else {
    parent = QLatin1String("");
  }
  return parent;
}

}

/**
 * Constructor.
 * @param parent parent object
 */
DirRenamer::DirRenamer(QObject* parent) : QObject(parent),
  m_fmtContext(new DirNameFormatReplacerContext),
  m_tagVersion(Frame::TagVAll), m_aborted(false), m_actionCreate(false)
{
  setObjectName(QLatin1String("DirRenamer"));
}

/**
 * Destructor.
 */
DirRenamer::~DirRenamer()
{
  delete m_fmtContext;
}

/** Only defined for generation of translation files */
#define CREATE_DIR_FAILED_FOR_PO QT_TRANSLATE_NOOP("@default", "Create folder %1 failed\n")

/**
 * Create a directory if it does not exist.
 *
 * @param dir      directory path
 * @param index    model index of item to rename
 * @param errorMsg if not NULL and an error occurred, a message is appended here,
 *                 otherwise it is not touched
 *
 * @return true if directory exists or was created successfully.
 */
bool DirRenamer::createDirectory(
    const QString& dir, const QPersistentModelIndex& index,
    QString* errorMsg) const
{
  if (auto model = const_cast<TaggedFileSystemModel*>(
        qobject_cast<const TaggedFileSystemModel*>(index.model()))) {
    const QString parentDirName = model->filePath(index.parent());
    const QString relativeName = QDir(parentDirName).relativeFilePath(dir);
    if (model->mkdir(index.parent(), relativeName).isValid() &&
        QFileInfo(dir).isDir()) {
      return true;
    }
  }
  if (QFileInfo(dir).isDir() ||
    (QDir().mkdir(dir) && QFileInfo(dir).isDir())) {
    return true;
  } else {
    if (errorMsg) {
      errorMsg->append(tr("Create folder %1 failed\n").arg(dir));
    }
    return false;
  }
}

/** Only defined for generation of translation files */
#define FILE_ALREADY_EXISTS_FOR_PO QT_TRANSLATE_NOOP("@default", "File %1 already exists\n")
/** Only defined for generation of translation files */
#define IS_NOT_DIR_FOR_PO QT_TRANSLATE_NOOP("@default", "%1 is not a folder\n")
/** Only defined for generation of translation files */
#define RENAME_FAILED_FOR_PO QT_TRANSLATE_NOOP("@default", "Rename %1 to %2 failed\n")

/**
 * Rename a directory.
 *
 * @param olddir   old directory name
 * @param newdir   new directory name
 * @param index    model index of item to rename
 * @param errorMsg if not NULL and an error occurred, a message is
 *                 appended here, otherwise it is not touched
 *
 * @return true if rename successful.
 */
bool DirRenamer::renameDirectory(
  const QString& olddir, const QString& newdir,
  const QPersistentModelIndex& index, QString* errorMsg) const
{
  if (QFileInfo::exists(newdir)) {
    if (errorMsg) {
      errorMsg->append(tr("File %1 already exists\n").arg(newdir));
    }
    return false;
  }
  if (!QFileInfo(olddir).isDir()) {
    if (errorMsg) {
      errorMsg->append(tr("%1 is not a folder\n").arg(olddir));
    }
    return false;
  }
  if (index.isValid()) {
    // The directory must be closed before renaming on Windows.
    TaggedFileIterator::closeFileHandles(index);
  }
  if (auto model = const_cast<TaggedFileSystemModel*>(
        qobject_cast<const TaggedFileSystemModel*>(index.model()))) {
    const QString parentDirName = model->filePath(index.parent());
    const QString relativeName = QDir(parentDirName).relativeFilePath(newdir);
    if (model->rename(index, relativeName) && QFileInfo(newdir).isDir()) {
      return true;
    }
  }
  if (Utils::safeRename(olddir, newdir) && QFileInfo(newdir).isDir()) {
    return true;
  } else {
    if (errorMsg) {
      errorMsg->append(tr("Rename %1 to %2 failed\n").arg(olddir, newdir));
    }
    return false;
  }
}

/** Only defined for generation of translation files */
#define ALREADY_EXISTS_FOR_PO QT_TRANSLATE_NOOP("@default", "%1 already exists\n")
/** Only defined for generation of translation files */
#define IS_NOT_FILE_FOR_PO QT_TRANSLATE_NOOP("@default", "%1 is not a file\n")

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
bool DirRenamer::renameFile(const QString& oldfn, const QString& newfn,
                const QPersistentModelIndex& index, QString* errorMsg) const
{
  if (QFileInfo(newfn).isFile()) {
    return true;
  }
  if (QFileInfo::exists(newfn)) {
    if (errorMsg) {
      errorMsg->append(tr("%1 already exists\n").arg(newfn));
    }
    return false;
  }
  if (!QFileInfo(oldfn).isFile()) {
    if (errorMsg) {
      errorMsg->append(tr("%1 is not a file\n").arg(oldfn));
    }
    return false;
  }
  if (TaggedFile* taggedFile =
      TaggedFileSystemModel::getTaggedFileOfIndex(index)) {
    // The file must be closed before renaming on Windows.
    taggedFile->closeFileHandle();
  }
  if (Utils::safeRename(oldfn, newfn) && QFileInfo(newfn).isFile()) {
    return true;
  } else {
    if (errorMsg) {
      errorMsg->append(tr("Rename %1 to %2 failed\n").arg(oldfn, newfn));
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
#ifdef Q_OS_WIN32
  newdir.replace(QLatin1Char('\\'), QLatin1Char('/'));
#endif
  if (newdir.endsWith(QLatin1Char('/'))) {
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
      newdir.append(QLatin1Char('/'));
    }
    DirNameFormatReplacer fmt(*m_fmtContext, trackData, m_format);
    fmt.replacePercentCodes(FormatReplacer::FSF_ReplaceSeparators);
    QString baseName = fmt.getString();
    const FormatConfig& fnCfg = FilenameFormatConfig::instance();
    if (fnCfg.useForOtherFileNames()) {
      if (!baseName.contains(QLatin1Char('/'))) {
        fnCfg.formatString(baseName);
      } else {
        // If the new folder name contains multiple path components separated
        // by '/', make sure not to replace the '/' when applying the format.
        QStringList baseNameComponents = baseName.split(QLatin1Char('/'));
        for (auto it = baseNameComponents.begin();
             it != baseNameComponents.end();
             ++it) {
          fnCfg.formatString(*it);
        }
        baseName = baseNameComponents.join(QLatin1Char('/'));
      }
    }
    m_fmtContext->putDirName(baseName);
    newdir.append(
          FilenameFormatConfig::instance().joinFileName(baseName, QString()));
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
}

/**
 * Add a rename action.
 *
 * @param type type of action
 * @param src  source file or directory name
 * @param dest destination file or directory name
 * @param index model index of item to rename
 */
void DirRenamer::addAction(RenameAction::Type type, const QString& src, const QString& dest,
                           const QPersistentModelIndex& index)
{
  // do not add an action if the source or destination is already in an action
  for (auto it = m_actions.constBegin(); it != m_actions.constEnd(); ++it) {
    if ((!src.isEmpty() && (*it).m_src == src) ||
        (!dest.isEmpty() && (*it).m_dest == dest)){
      return;
    }
  }

  RenameAction action(type, src, dest, index);
  m_actions.append(action);
  if (!m_fmtContext->hasAggregatedCodes()) {
    emit actionScheduled(describeAction(action));
  }
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
  for (auto it = m_actions.constBegin(); it != m_actions.constEnd(); ++it) {
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
  for (auto it = m_actions.constBegin(); it != m_actions.constEnd(); ++it) {
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
    for (auto it = m_actions.constBegin(); it != m_actions.constEnd(); ++it) {
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
      if (newDirname.startsWith(currentDirname + QLatin1Char('/'))) {
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
          int slashPos = newPart.indexOf(QLatin1Char('/'), 1);
          if (slashPos != -1 && slashPos != newPart.length() - 1) {
            newPart.truncate(slashPos);
            // the new part has multiple directories
            // => create one directory
          } else {
            createDir = false;
          }
          // Create a directory for each file and move it.
          addAction(RenameAction::CreateDirectory, QString(),
                    currentDirname + newPart, taggedFile->getIndex());
          if (!createDir) {
            addAction(RenameAction::RenameFile,
                      dirWithFiles + QLatin1Char('/') + taggedFile->getFilename(),
                      currentDirname + newPart + QLatin1Char('/') + taggedFile->getFilename(),
                      taggedFile->getIndex());
          }
          currentDirname = currentDirname + newPart;
        }
      } else {
        QString parent(parentDirectory(currentDirname));
        if (newDirname.startsWith(parent)) {
          QString newPart(newDirname.mid(parent.length()));
          int slashPos = newPart.indexOf(QLatin1Char('/'));
          if (slashPos != -1 && slashPos != newPart.length() - 1) {
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
                      currentDirname + QLatin1Char('/') + taggedFile->getFilename(),
                      parentWithNewPart + QLatin1Char('/') + taggedFile->getFilename(),
                      taggedFile->getIndex());
            currentDirname = parentWithNewPart;
          } else {
            addAction(RenameAction::RenameDirectory, currentDirname, parentWithNewPart,
                      taggedFile->getIndex().parent());
            currentDirname = parentWithNewPart;
          }
        } else {
          // new directory name is too different
          addAction(RenameAction::ReportError, tr("New folder name is too different\n"));
        }
      }
    }
    if (!again) break;
  }
}

/**
 * Terminate scheduling of actions.
 */
void DirRenamer::endScheduleActions()
{
  if (m_fmtContext->hasAggregatedCodes()) {
    const auto replacements = m_fmtContext->takeReplacements();
    for (RenameAction& action : m_actions) {
      for (const auto& replacement : replacements) {
        action.m_src.replace(replacement.first, replacement.second);
        action.m_dest.replace(replacement.first, replacement.second);
      }
      emit actionScheduled(describeAction(action));
    }
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
  for (auto it = m_actions.constBegin(); it != m_actions.constEnd(); ++it) {
    switch ((*it).m_type) {
      case RenameAction::CreateDirectory:
        createDirectory((*it).m_dest, (*it).m_index, errorMsg);
        break;
      case RenameAction::RenameDirectory:
        if (renameDirectory((*it).m_src, (*it).m_dest, (*it).m_index,
                            errorMsg)) {
          if ((*it).m_src == m_dirName) {
            m_dirName = (*it).m_dest;
          }
        }
        break;
      case RenameAction::RenameFile:
        renameFile((*it).m_src, (*it).m_dest, (*it).m_index, errorMsg);
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
 * Get description of an actions to be performed.
 * @return (action, [src,] dst) list describing the action to be
 * performed.
 */
QStringList DirRenamer::describeAction(const RenameAction& action) const
{
  static const char* const typeStr[] = {
    QT_TRANSLATE_NOOP("@default", "Create folder"),
    QT_TRANSLATE_NOOP("@default", "Rename folder"),
    QT_TRANSLATE_NOOP("@default", "Rename file"),
    QT_TRANSLATE_NOOP("@default", "Error")
  };
  static const unsigned numTypeStr = sizeof(typeStr) / sizeof(typeStr[0]);

  QStringList actionStrs;
  auto typeIdx = static_cast<unsigned>(action.m_type);
  if (typeIdx >= numTypeStr) {
    typeIdx = numTypeStr - 1;
  }
  actionStrs.append(QCoreApplication::translate("@default", typeStr[typeIdx]));
  if (!action.m_src.isEmpty()) {
    actionStrs.append(action.m_src);
  }
  actionStrs.append(action.m_dest);
  return actionStrs;
}

/**
 * Check if operation is aborted.
 *
 * @return true if aborted.
 */
bool DirRenamer::isAborted() const
{
  return m_aborted;
}

/**
 * Clear state which is reported by isAborted().
 */
void DirRenamer::clearAborted()
{
  m_aborted = false;
}

/**
 * Abort operation.
 */
void DirRenamer::abort()
{
  m_aborted = true;
}
