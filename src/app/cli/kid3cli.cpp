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
#include "taggedfileselection.h"
#include "clicommand.h"
#include "cliconfig.h"

#ifdef HAVE_READLINE

#include "readlinecompleter.h"

class Kid3CliCompleter : public ReadlineCompleter {
public:
  explicit Kid3CliCompleter(const QList<CliCommand*>& cmds);
  virtual ~Kid3CliCompleter();

  virtual QList<QByteArray> getCommandList() const;
  virtual QList<QByteArray> getParameterList() const;
  virtual bool updateParameterList(const char* buffer);

private:
  const QList<CliCommand*>& m_cmds;
  QList<QByteArray> m_commands;
  QList<QByteArray> m_parameters;
};

Kid3CliCompleter::Kid3CliCompleter(const QList<CliCommand*>& cmds) :
  m_cmds(cmds)
{
  foreach (const CliCommand* cmd, cmds) {
    m_commands.append(cmd->name().toLocal8Bit());
  }
}

Kid3CliCompleter::~Kid3CliCompleter()
{
}

QList<QByteArray> Kid3CliCompleter::getCommandList() const
{
  return m_commands;
}

QList<QByteArray> Kid3CliCompleter::getParameterList() const
{
  return m_parameters;
}

bool Kid3CliCompleter::updateParameterList(const char* buffer)
{
  QString cmdName = QString::fromLocal8Bit(buffer);
  bool isFirstParameter = true;
  int cmdNameEndIdx = cmdName.indexOf(QLatin1Char(' '));
  if (cmdNameEndIdx != -1) {
    isFirstParameter =
        cmdName.indexOf(QLatin1Char(' '), cmdNameEndIdx + 1) == -1;
    cmdName.truncate(cmdNameEndIdx);
  }

  QString argSpec;
  if (isFirstParameter) {
    foreach (const CliCommand* cmd, m_cmds) {
      if (cmdName == cmd->name()) {
        argSpec = cmd->argumentSpecification();
        break;
      }
    }
  }

  m_parameters.clear();
  if (!argSpec.isEmpty()) {
    QStringList argSpecs = argSpec.split(QLatin1Char('\n'));
    if (!argSpecs.isEmpty()) {
      QString argTypes = argSpecs.first().
          remove(QLatin1Char('[')).remove(QLatin1Char(']'));
      if (!argTypes.isEmpty()) {
        char argType = argTypes.at(0).toLatin1();
        switch (argType) {
        case 'P':
          // file path
          return false;
        case 'T':
          // tagnumbers
          m_parameters << "1" << "2" << "12";
          break;
        case 'N':
        {
          // frame name
          static QList<QByteArray> frameNames;
          if (frameNames.isEmpty()) {
            for (int k = Frame::FT_FirstFrame; k <= Frame::FT_LastFrame; ++k) {
              frameNames.append(
                    Frame::ExtendedType(static_cast<Frame::Type>(k),
                                        QLatin1String("")).
                    getName().toLower().remove(QLatin1Char(' ')).toLocal8Bit());
            }
          }
          m_parameters = frameNames;
          break;
        }
        case 'S':
          // specific command
          if (argSpecs.size() > 0) {
            const QString& valuesStr = argSpecs.at(1);
            int valuesIdx = valuesStr.indexOf(QLatin1String("S = \""));
            if (valuesIdx != -1) {
              QStringList values =
                  valuesStr.mid(valuesIdx + 4).split(QLatin1String(" | "));
              foreach (const QString& value, values) {
                if (value.startsWith(QLatin1Char('"')) &&
                    value.endsWith(QLatin1Char('"'))) {
                  m_parameters.append(
                        value.mid(1, value.length() - 2).toLocal8Bit());
                }
              }
            }
          }
          break;
        default:
          break;
        }
      }
    }
  }
  return true;
}

#endif // HAVE_READLINE


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
    QString param = QLatin1String("");
    if (c == QLatin1Char('~')) {
      if (pos >= str.size() || str.at(pos).isSpace()) {
        params.append(QDir::homePath());
        continue;
      }
      if (str.at(pos) == QLatin1Char('/')) {
        param = QDir::homePath();
        c = QLatin1Char('/');
        ++pos;
      }
    }
    do {
      if (c == QLatin1Char('\'')) {
        int spos = pos;
        do {
          if (pos >= str.size())
            return QStringList();
          c = str.at(pos++);
        } while (c != QLatin1Char('\''));
        param += str.midRef(spos, pos - spos - 1);
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
 * @param app application context
 * @param io I/O handler
 * @param parent parent object
 */
Kid3Cli::Kid3Cli(Kid3Application* app,
                 AbstractCliIO* io, QObject* parent) :
  AbstractCli(io, parent),
  m_app(app),
  m_tagMask(Frame::TagV2V1), m_timeoutMs(0), m_fileNameChanged(false)
{
  m_cmds << new HelpCommand(this)
         << new TimeoutCommand(this)
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
#if defined HAVE_PHONON || QT_VERSION >= 0x050000
         << new PlayCommand(this)
#endif
         ;
  connect(m_app, SIGNAL(fileSelectionUpdateRequested()),
          this, SLOT(updateSelectedFiles()));
  connect(m_app, SIGNAL(selectedFilesUpdated()),
          this, SLOT(updateSelection()));
#ifdef HAVE_READLINE
  m_completer = new Kid3CliCompleter(m_cmds);
  m_completer->install();
#endif
}

/**
 * Destructor.
 */
Kid3Cli::~Kid3Cli()
{
#ifdef HAVE_READLINE
  delete m_completer;
#endif
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
                            "\"artist\" | ..."));
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
  }
}

/**
 * Open directory
 * @param paths directory or file paths
 * @return true if ok.
 */
bool Kid3Cli::openDirectory(const QStringList& paths)
{
  if (m_app->openDirectory(paths, true)) {
    QDir::setCurrent(m_app->getDirPath());
    m_app->getFileSelectionModel()->clearSelection();
    return true;
  }
  return false;
}

/**
 * Expand wildcards in path list.
 * @param paths paths to expand
 * @return expanded paths.
 */
QStringList Kid3Cli::expandWildcards(const QStringList& paths)
{
  QStringList expandedPaths;
  foreach (const QString& path, paths) {
    QStringList expandedPath;
    int wcIdx = path.indexOf(QRegExp(QLatin1String("[?*]")));
    if (wcIdx != -1) {
      QString partBefore, partAfter;
      int beforeIdx = path.lastIndexOf(QDir::separator(), wcIdx);
      partBefore = path.left(beforeIdx + 1);
      int afterIdx = path.indexOf(QDir::separator(), wcIdx);
      if (afterIdx == -1) {
        afterIdx = path.length();
      }
      partAfter = path.mid(afterIdx + 1);
      QString wildcardPart = path.mid(beforeIdx + 1, afterIdx - beforeIdx);
      if (!wildcardPart.isEmpty()) {
        QDir dir(partBefore);
        if (!dir.exists(wildcardPart)) {
          QStringList entries = dir.entryList(QStringList() << wildcardPart,
                                    QDir::AllEntries | QDir::NoDotAndDotDot);
          if (!entries.isEmpty()) {
            foreach (const QString& entry, entries) {
              expandedPath.append(partBefore + entry + partAfter);
            }
          }
        }
      }
    }
    if (expandedPath.isEmpty()) {
      expandedPaths.append(path);
    } else {
      expandedPaths.append(expandedPath);
    }
  }
  return expandedPaths;
}

/**
 * Select files in the current directory.
 * @param paths file names
 * @return true if files found and selected.
 */
bool Kid3Cli::selectFile(const QStringList& paths)
{
  bool ok = true;
  FileProxyModel* model = m_app->getFileProxyModel();
  foreach (const QString& fileName, paths) {
    QModelIndex index = model->index(fileName);
    if (index.isValid()) {
      m_app->getFileSelectionModel()->setCurrentIndex(
            index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    } else {
      ok = false;
    }
  }
  return ok;
}

/**
 * Update the currently selected files from the frame tables.
 */
void Kid3Cli::updateSelectedFiles()
{
  TaggedFileSelection* selection = m_app->selectionInfo();
  selection->selectChangedFrames();
  if (!selection->isEmpty()) {
    m_app->frameModelsToTags();
  }
  selection->setFilename(m_filename);
}

/**
 * Has to be called when the selection changes to update the frame tables
 * and the information about the selected files.
 */
void Kid3Cli::updateSelection()
{
  m_app->tagsToFrameModels();

  TaggedFileSelection* selection = m_app->selectionInfo();
  m_filename = selection->getFilename();
  m_tagFormatV1 = selection->getTagFormatV1();
  m_tagFormatV2 = selection->getTagFormatV2();
  m_fileNameChanged = selection->isFilenameChanged();
  m_detailInfo = selection->getDetailInfo();
  selection->clearUnusedFrames();
}

/**
 * Display information about selected files.
 * @param tagMask tag bits (1 for tag 1, 2 for tag 2)
 */
void Kid3Cli::writeFileInformation(int tagMask)
{
  if (!m_detailInfo.isEmpty()) {
    writeLine(tr("File") + QLatin1String(": ") + m_detailInfo);
  }
  if (!m_filename.isEmpty()) {
    QString line = m_fileNameChanged ? QLatin1String("*") : QLatin1String(" ");
    line += QLatin1Char(' ');
    line += tr("Name");
    line += QLatin1String(": ");
    line += m_filename;
    writeLine(line);
  }
  foreach (Frame::TagVersion tagBit, QList<Frame::TagVersion>()
           << Frame::TagV1 << Frame::TagV2) {
    if (tagMask & tagBit) {
      FrameTableModel* ft = (tagBit == Frame::TagV2)
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
        if (tagBit == Frame::TagV2) {
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
  case Frame::TagV1:
    tagStr = QLatin1String("1");
    break;
  case Frame::TagV2:
    tagStr = QLatin1String("2");
    break;
  case Frame::TagV2V1:
    tagStr = QLatin1String("1 & 2");
    break;
  case Frame::TagNone:
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
void Kid3Cli::setTagMask(Frame::TagVersion tagMask)
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

  m_app->updateCurrentSelection();
  QSet<QPersistentModelIndex> selection = m_app->getCurrentSelection().toSet();
  for (int row = 0; row < model->rowCount(parent); ++row) {
    QString indentStr(indent, QLatin1Char(' ')), nameStr;
    QModelIndexList indexesWithChildren;
    QModelIndex idx(model->index(row, 0, parent));
    QString propsStr = selection.contains(idx)
        ? QLatin1String(">") : QLatin1String(" ");
    if (TaggedFile* taggedFile = FileProxyModel::getTaggedFileOfIndex(idx)) {
      taggedFile = FileProxyModel::readTagsFromTaggedFile(taggedFile);
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
    foreach (const QModelIndex& idx, indexesWithChildren) {
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
  flushStandardOutput();
  CliCommand* cmd = commandForArgs(line);
  if (cmd) {
    connect(cmd, SIGNAL(finished()), this, SLOT(onCommandFinished()));
    cmd->execute();
  } else {
    writeErrorLine(tr("Unknown command '%1'. Type 'help' for help.").arg(line));
    promptNextLine();
  }
}

/**
 * Called when a command is finished.
 */
void Kid3Cli::onCommandFinished() {
  if (CliCommand* cmd = qobject_cast<CliCommand*>(sender())) {
    disconnect(cmd, SIGNAL(finished()), this, SLOT(onCommandFinished()));
    if (cmd->hasError()) {
      QString msg(cmd->getErrorMessage());
      if (!msg.startsWith(QLatin1Char('_'))) {
        writeErrorLine(msg);
      }
    }
    cmd->clear();
    promptNextLine();
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
      QString msg(cmd->getErrorMessage());
      if (!msg.startsWith(QLatin1Char('_'))) {
        writeErrorLine(msg);
      }
      cmd->clear();
      setReturnCode(1);
      terminate();
    }
  }
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
      writeLine(QLatin1String("kid3-qt " VERSION " (c) " RELEASE_YEAR
                              " Urs Fleisch"));
      writeLine(tr("Usage:") + QLatin1String(
          " kid3-cli [-c command1] [-c command2 ...] [path ...]"));
      writeHelp();
      flushStandardOutput();
      terminate();
      return true;
    } else {
      paths.append(arg);
    }
  }

  if (paths.isEmpty()) {
    paths.append(QDir::currentPath());
  }
  connect(m_app, SIGNAL(directoryOpened()),
    this, SLOT(onInitialDirectoryOpened()));
  if (!openDirectory(expandWildcards(paths))) {
    writeErrorLine(tr("%1 does not exist").arg(paths.join(QLatin1String(", "))));
  }
  return !m_argCommands.isEmpty();
}

/**
 * Select files passed as command line arguments after the initial directory has
 * been opened. Start execution of commands if existing.
 */
void Kid3Cli::onInitialDirectoryOpened()
{
  disconnect(m_app, SIGNAL(directoryOpened()),
    this, SLOT(onInitialDirectoryOpened()));
  if (!m_argCommands.isEmpty()) {
    if (!m_app->getRootIndex().isValid()) {
      // Do not execute commands if directory could not be opened.
      m_argCommands.clear();
    }
    executeNextArgCommand();
  }
}

void Kid3Cli::executeNextArgCommand()
{
  if (m_argCommands.isEmpty()) {
    if (m_app->isModified() && !m_app->getDirName().isEmpty()) {
      // Automatically save changes in command mode.
      QStringList errorFiles = m_app->saveDirectory();
      if (!errorFiles.isEmpty()) {
        writeErrorLine(tr("Error while writing file:\n") +
                       errorFiles.join(QLatin1String("\n")));
        setReturnCode(1);
      }
    }
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
    setReturnCode(1);
    terminate();
  }
}
