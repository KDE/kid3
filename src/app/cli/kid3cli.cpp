/**
 * \file kid3cli.cpp
 * Command line interface for Kid3.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Aug 2013
 *
 * Copyright (C) 2013  Urs Fleisch
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

#include "kid3cli.h"
#include <QDir>
#include <QCoreApplication>
#include <QItemSelectionModel>
#include <QTimer>
#include "kid3application.h"
#include "fileproxymodel.h"
#include "frametablemodel.h"
#include "coreplatformtools.h"
#include "clicommand.h"

/** @cond */
namespace {

/**
 * Split string into command line arguments supporting quotes and escape
 * characters.
 * @param str command line string
 * @return list of arguments.
 */
QStringList splitArgs(const QString& str)
{
  QStringList params;

  for (int pos = 0; ; ) {
    QChar c;
    do {
      if (pos >= str.size())
        return params;
      c = str.at(pos++);
    } while (c.isSpace());
    QString param;
    do {
      if (c == QLatin1Char('\'')) {
        int spos = pos;
        do {
          if (pos >= str.size())
            return QStringList();
          c = str.at(pos++);
        } while (c != QLatin1Char('\''));
        param += str.mid(spos, pos - spos - 1);
      } else if (c == QLatin1Char('"')) {
        for (;;) {
          if (pos >= str.size())
            return QStringList();
          c = str.at(pos++);
          if (c == QLatin1Char('"'))
            break;
          if (c == QLatin1Char('\\')) {
            if (pos >= str.size())
              return QStringList();
            c = str.at(pos++);
            if (c != QLatin1Char('"') && c != QLatin1Char('\\'))
              param += QLatin1Char('\\');
          }
          param += c;
        }
      } else {
        if (c == QLatin1Char('\\')) {
          if (pos >= str.size())
            return QStringList();
          c = str.at(pos++);
        }
        param += c;
      }
      if (pos >= str.size())
        break;
      c = str.at(pos++);
    } while (!c.isSpace());
    params.append(param);
  }
  return QStringList();
}

}
/** @endcond */


/**
 * Constructor.
 * @param parent parent object
 */
Kid3Cli::Kid3Cli(QObject* parent) : AbstractCli(parent),
  m_platformtools(new CorePlatformTools),
  m_app(new Kid3Application(m_platformtools, this)),
  m_tagMask(TrackData::TagV2V1), m_fileNameChanged(false)
{
  m_cmds << new HelpCommand(this)
         << new QuitCommand(this)
         << new CdCommand(this)
         << new PwdCommand(this)
         << new LsCommand(this)
         << new SaveCommand(this)
         << new SelectCommand(this)
         << new TagCommand(this)
         << new GetCommand(this)
         << new SetCommand(this)
         << new RevertCommand(this)
         << new ImportCommand(this)
         << new BatchImportCommand(this)
         << new AlbumArtCommand(this)
         << new ExportCommand(this)
         << new PlaylistCommand(this)
         << new FilenameFormatCommand(this)
         << new TagFormatCommand(this)
         << new TextEncodingCommand(this)
         << new RenameDirectoryCommand(this)
         << new NumberTracksCommand(this)
         << new FilterCommand(this)
         << new ToId3v24Command(this)
         << new ToId3v23Command(this)
         << new TagToFilenameCommand(this)
         << new FilenameToTagCommand(this)
         << new TagToOtherTagCommand(this)
         << new CopyCommand(this)
         << new PasteCommand(this)
         << new RemoveCommand(this)
         << new PlayCommand(this);
  connect(m_app, SIGNAL(fileSelectionUpdateRequested()),
          this, SLOT(updateSelectedFiles()));
  connect(m_app, SIGNAL(selectedFilesUpdated()),
          this, SLOT(updateSelection()));
}

/**
 * Destructor.
 */
Kid3Cli::~Kid3Cli()
{
}

/**
 * Get command for a command line.
 * @param line command line
 * @return command, 0 if no command found.
 */
CliCommand* Kid3Cli::commandForArgs(const QString& line)
{
  if (line.isEmpty())
    return 0;

  QStringList args = splitArgs(line);
  if (!args.isEmpty()) {
    const QString& name = args.at(0);
    for (QList<CliCommand*>::iterator it = m_cmds.begin();
         it != m_cmds.end();
         ++it) {
      CliCommand* cmd = *it;
      if (name == cmd->name()) {
        cmd->setArgs(args);
        return cmd;
      }
    }
  }
  return 0;
}

/**
 * Display help about available commands.
 * @param cmdName command name, for all commands if empty
 */
void Kid3Cli::writeHelp(const QString& cmdName)
{
  if (cmdName.isEmpty()) {
    writeLine(tr("Parameter"));
    writeLine(QLatin1String("  P = ") + tr("File path"));
    writeLine(QLatin1String("  U = ") + tr("URL"));
    writeLine(QLatin1String("  T = ") + tr("Tag numbers") +
              QLatin1String(" \"1\" | \"2\" | \"12\""));
    writeLine(QLatin1String("  N = ") + tr("Frame name") +
              QLatin1String(" \"album\" | \"album artist\" | \"arranger\" | "
                            "\" artist\" | ..."));
    writeLine(QLatin1String("  V = ") + tr("Frame value"));
    writeLine(QLatin1String("  F = ") + tr("Format"));
    writeLine(QLatin1String("  S = ") + tr("Command specific"));
    writeLine(tr("Available Commands"));
  }
  QList<QStringList> cmdStrs;
  int maxLength = 0;
  for (QList<CliCommand*>::const_iterator it = m_cmds.constBegin();
       it != m_cmds.constEnd();
       ++it) {
    const CliCommand* cmd = *it;
    QString cmdStr = cmd->name();
    if (cmdName.isEmpty() || cmdName == cmdStr) {
      QStringList spec = cmd->argumentSpecification().split(QLatin1Char('\n'));
      if (!spec.isEmpty()) {
        cmdStr += QLatin1Char(' ');
        cmdStr += spec.takeFirst();
      }
      cmdStrs.append(QStringList() << cmdStr << cmd->help() << spec);
      maxLength = qMax(cmdStr.size(), maxLength);
    }
  }
  foreach (QStringList strs, cmdStrs) {
    QString cmdStr = strs.takeFirst();
    cmdStr += QString(maxLength - cmdStr.size() + 2, QLatin1Char(' '));
    cmdStr += strs.takeFirst();
    writeLine(cmdStr);
    while (!strs.isEmpty()) {
      cmdStr = QString(maxLength + 2, QLatin1Char(' '));
      cmdStr += strs.takeFirst();
      writeLine(cmdStr);
    }
  }
}

/**
 * Execute process.
 */
void Kid3Cli::execute()
{
  if (!parseOptions()) {
    // Interactive mode
    AbstractCli::execute();
    writePrompt();
  }
}

/**
 * Open directory
 * @param paths directory or file paths
 * @return true if ok.
 */
bool Kid3Cli::openDirectory(const QStringList& paths)
{
  bool ok = false;
  if (!paths.isEmpty() && QFileInfo(paths.first()).exists()) {
    ok = m_app->openDirectory(paths);
    if (ok) {
      QDir::setCurrent(m_app->getDirPath());
      m_app->getFileSelectionModel()->clearSelection();
    }
  }
  return ok;
}

/**
 * Select a file in the current directory.
 * @param fileName file name
 * @return true if file found and selected.
 */
bool Kid3Cli::selectFile(const QString& fileName)
{
  FileProxyModel* model = m_app->getFileProxyModel();
  QModelIndex index = model->index(fileName);
  if (index.isValid()) {
    m_app->getFileSelectionModel()->setCurrentIndex(
          index, QItemSelectionModel::Select);
    return true;
  }
  return false;
}

/**
 * Get indexes of selected files.
 * @return selected indexes.
 */
QList<QPersistentModelIndex> Kid3Cli::getSelection() const
{
  QList<QPersistentModelIndex> selection;
  if (QItemSelectionModel* selModel = m_app->getFileSelectionModel()) {
    foreach (QModelIndex index, selModel->selectedIndexes()) {
      selection.append(QPersistentModelIndex(index));
    }
  }
  return selection;
}

/**
 * Update the currently selected files from the frame tables.
 */
void Kid3Cli::updateSelectedFiles()
{
  int selectionSize = m_selection.size();
  if (selectionSize > 0) {
    if (selectionSize > 1) {
      m_app->frameModelV1()->selectChangedFrames();
      m_app->frameModelV2()->selectChangedFrames();
    }
    m_app->frameModelsToTags(m_selection);
    if (m_app->selectionSingleFile() && !m_filename.isEmpty()) {
      if (TaggedFile* taggedFile =
          FileProxyModel::getTaggedFileOfIndex(m_selection.first())) {
        taggedFile->setFilename(m_filename);
      }
    }
  }
}

/**
 * Has to be called when the selection changes to update the frame tables
 * and the information about the selected files.
 */
void Kid3Cli::updateSelection()
{
  m_selection = getSelection();
  m_app->tagsToFrameModels(m_selection);

  if (m_app->selectionSingleFile()) {
    m_filename = m_app->selectionSingleFile()->getFilename();
    m_app->selectionSingleFile()->getDetailInfo(m_detailInfo);
    m_tagFormatV1 = m_app->selectionSingleFile()->getTagFormatV1();
    m_tagFormatV2 = m_app->selectionSingleFile()->getTagFormatV2();
    m_fileNameChanged = m_app->selectionSingleFile()->isFilenameChanged();
  } else {
    if (m_app->selectionFileCount() > 1) {
      m_filename.clear();
    }
    m_detailInfo = TaggedFile::DetailInfo();
    m_tagFormatV1.clear();
    m_tagFormatV2.clear();
    m_fileNameChanged = false;
  }
  if (m_app->selectionTagV1SupportedCount() == 0) {
    m_app->frameModelV1()->clearFrames();
  }
  if (m_app->selectionFileCount() == 0) {
    m_app->frameModelV2()->clearFrames();
    m_filename.clear();
  }
}

/**
 * Display information about selected files.
 * @param tagMask tag bits (1 for tag 1, 2 for tag 2)
 */
void Kid3Cli::writeFileInformation(int tagMask)
{
  if (m_detailInfo.valid) {
    writeLine(tr("File") + QLatin1String(": ") + m_detailInfo.toString());
  }
  if (!m_filename.isEmpty()) {
    QString line = m_fileNameChanged ? QLatin1String("*") : QLatin1String(" ");
    line += QLatin1Char(' ');
    line += tr("Name");
    line += QLatin1String(": ");
    line += m_filename;
    writeLine(line);
  }
  foreach (TrackData::TagVersion tagBit, QList<TrackData::TagVersion>()
           << TrackData::TagV1 << TrackData::TagV2) {
    if (tagMask & tagBit) {
      FrameTableModel* ft = (tagBit == TrackData::TagV2)
          ? m_app->frameModelV2() : m_app->frameModelV1();
      int maxLength = 0;
      bool hasValue = false;
      for (int row = 0; row < ft->rowCount(); ++row) {
        QString name =
            ft->index(row, FrameTableModel::CI_Enable).data().toString();
        QString value =
            ft->index(row, FrameTableModel::CI_Value).data().toString();
        maxLength = qMax(name.size(), maxLength);
        hasValue = hasValue || !value.isEmpty();
      }
      if (hasValue) {
        if (tagBit == TrackData::TagV2) {
          writeLine(tr("Tag 2") + QLatin1String(": ") + m_tagFormatV2);
        } else {
          writeLine(tr("Tag 1") + QLatin1String(": ") + m_tagFormatV1);
        }
        for (int row = 0; row < ft->rowCount(); ++row) {
          QString name =
              ft->index(row, FrameTableModel::CI_Enable).data().toString();
          QString value =
              ft->index(row, FrameTableModel::CI_Value).data().toString();
          if (!value.isEmpty()) {
            bool changed = ft->index(row, FrameTableModel::CI_Enable).
                data(Qt::BackgroundColorRole).value<QBrush>() != Qt::NoBrush;
            QString line = changed ? QLatin1String("*") : QLatin1String(" ");
            line += QLatin1Char(' ');
            line += name;
            line += QString(maxLength - name.size() + 2,
                            QLatin1Char(' '));
            line += value;
            writeLine(line);
          }
        }
      }
    }
  }
}

/**
 * Write currently active tag mask.
 */
void Kid3Cli::writeTagMask()
{
  QString tagStr;
  switch (m_tagMask) {
  case TrackData::TagV1:
    tagStr = QLatin1String("1");
    break;
  case TrackData::TagV2:
    tagStr = QLatin1String("2");
    break;
  case TrackData::TagV2V1:
    tagStr = QLatin1String("1 & 2");
    break;
  case TrackData::TagNone:
    tagStr = QLatin1String("-");
    break;
  }
  writeLine(tr("Tags") + QLatin1String(": ") + tagStr);
}

/**
 * Set currently active tag mask.
 *
 * @param tagMask tag bits
 */
void Kid3Cli::setTagMask(TrackData::TagVersion tagMask)
{
  m_tagMask = tagMask;
}

/**
 * List files.
 */
void Kid3Cli::writeFileList()
{
  printFileProxyModel(m_app->getFileProxyModel(), m_app->getRootIndex(), 1);
}

/**
 * List files.
 *
 * @param model file proxy model
 * @param parent index of parent item
 * @param indent indentation as number of spaces
 */
void Kid3Cli::printFileProxyModel(const FileProxyModel* model,
                                  const QModelIndex& parent, int indent) {
  if (!model->hasChildren(parent))
    return;

  QSet<QPersistentModelIndex> selection = getSelection().toSet();
  for (int row = 0; row < model->rowCount(parent); ++row) {
    QString indentStr(indent, QLatin1Char(' ')), nameStr;
    QModelIndexList indexesWithChildren;
    QModelIndex idx(model->index(row, 0, parent));
    QString propsStr = selection.contains(idx)
        ? QLatin1String(">") : QLatin1String(" ");
    if (TaggedFile* taggedFile = FileProxyModel::getTaggedFileOfIndex(idx)) {
      taggedFile->readTags(false);
      taggedFile = FileProxyModel::readWithId3V24IfId3V24(taggedFile);
      propsStr +=
          (taggedFile->isChanged() ? QLatin1String("*") : QLatin1String(" ")) +
          (taggedFile->hasTagV1() ? QLatin1Char('1') : QLatin1Char('-')) +
          (taggedFile->hasTagV2() ? QLatin1Char('2') : QLatin1Char('-'));
      nameStr = taggedFile->getFilename();
    } else {
      propsStr += QString(3, QLatin1Char(' '));
      QVariant value(model->data(idx));
      if (value.isValid()) {
        nameStr = value.toString();
      }
    }
    if (model->hasChildren(idx)) {
      indexesWithChildren.append(idx);
    }
    writeLine(propsStr + indentStr + nameStr);
    foreach (QModelIndex idx, indexesWithChildren) {
      printFileProxyModel(model, idx, indent + 2);
    }
  }
}

/**
 * Process command line.
 * @param line command line
 */
void Kid3Cli::readLine(const QString& line)
{
  cout().flush();
  CliCommand* cmd = commandForArgs(line);
  if (cmd) {
    connect(cmd, SIGNAL(finished()), this, SLOT(onCommandFinished()));
    cmd->execute();
  } else {
    writeErrorLine(tr("Unknown command '%1'. Type 'help' for help.").arg(line));
    writePrompt();
  }
}

/**
 * Called when a command is finished.
 */
void Kid3Cli::onCommandFinished() {
  if (CliCommand* cmd = qobject_cast<CliCommand*>(sender())) {
    disconnect(cmd, SIGNAL(finished()), this, SLOT(onCommandFinished()));
    if (cmd->hasError()) {
      writeErrorLine(cmd->getErrorMessage());
    }
    cmd->clear();
    writePrompt();
  }
}

/**
 * Called when an argument command is finished.
 */
void Kid3Cli::onArgCommandFinished() {
  if (CliCommand* cmd = qobject_cast<CliCommand*>(sender())) {
    disconnect(cmd, SIGNAL(finished()), this, SLOT(onArgCommandFinished()));
    if (!cmd->hasError()) {
      cmd->clear();
      executeNextArgCommand();
    } else {
      writeErrorLine(cmd->getErrorMessage());
      cmd->clear();
    }
  }
}

void Kid3Cli::writePrompt()
{
  cout() << QLatin1String("kid3-cli> ");
  cout().flush();
}

bool Kid3Cli::parseOptions()
{
  QStringList args = QCoreApplication::arguments().mid(1);
  QStringList paths;
  bool isCommand = false;
  foreach (const QString& arg, args) {
    if (isCommand) {
      m_argCommands.append(arg);
      isCommand = false;
    } else if (arg == QLatin1String("-c")) {
      isCommand = true;
    } else if (arg == QLatin1String("-h") || arg == QLatin1String("--help")) {
      cout() << tr("Usage:") + QLatin1String(
          " kid3-cli [-c command1] [-c command2 ...] [path ...]\n");
      writeHelp();
      cout().flush();
      terminate();
      return true;
    } else {
      paths.append(arg);
    }
  }

  if (paths.isEmpty()) {
    paths.append(QDir::currentPath());
  }
  connect(m_app,
    SIGNAL(directoryOpened(QPersistentModelIndex,QList<QPersistentModelIndex>)),
    this,
    SLOT(onInitialDirectoryOpened(QPersistentModelIndex,QList<QPersistentModelIndex>)));
  openDirectory(paths);
  //! @todo select all given paths (incl. wildcards).
  return !m_argCommands.isEmpty();
}

/**
 * Select files passed as command line arguments after the intial directory has
 * been opened. Start execution of commands if existing.
 * @param dirIndex file proxy model index of opened directory
 * @param fileIndexes file proxy model indexes of selected files
 */
void Kid3Cli::onInitialDirectoryOpened(
    const QPersistentModelIndex& dirIndex,
    const QList<QPersistentModelIndex>& fileIndexes)
{
  Q_UNUSED(dirIndex)
  disconnect(m_app,
    SIGNAL(directoryOpened(QPersistentModelIndex,QList<QPersistentModelIndex>)),
    this,
    SLOT(onInitialDirectoryOpened(QPersistentModelIndex,QList<QPersistentModelIndex>)));
  QItemSelectionModel* selModel = m_app->getFileSelectionModel();
  if (selModel && !fileIndexes.isEmpty()) {
    foreach (const QPersistentModelIndex& fileIndex, fileIndexes) {
      selModel->select(fileIndex, QItemSelectionModel::Select);
    }
    selModel->setCurrentIndex(fileIndexes.first(),
                              QItemSelectionModel::NoUpdate);
  }
  if (!m_argCommands.isEmpty()) {
    executeNextArgCommand();
  }
}

void Kid3Cli::executeNextArgCommand()
{
  if (m_argCommands.isEmpty()) {
    terminate();
    return;
  }

  QString line = m_argCommands.takeFirst();
  CliCommand* cmd = commandForArgs(line);
  if (cmd) {
    connect(cmd, SIGNAL(finished()), this, SLOT(onArgCommandFinished()));
    cmd->execute();
  } else {
    writeErrorLine(tr("Unknown command '%1', -h for help.").arg(line));
    terminate();
  }
}
