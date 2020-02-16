/**
 * \file kid3cli.cpp
 * Command line interface for Kid3.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Aug 2013
 *
 * Copyright (C) 2013-2018  Urs Fleisch
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
#include <QStringBuilder>
#include "kid3application.h"
#include "icoreplatformtools.h"
#include "coretaggedfileiconprovider.h"
#include "fileproxymodel.h"
#include "frametablemodel.h"
#include "taggedfileselection.h"
#include "clicommand.h"
#include "cliconfig.h"
#include "clierror.h"
#include "textcliformatter.h"
#include "jsoncliformatter.h"

#ifdef HAVE_READLINE

#include "readlinecompleter.h"

class Kid3CliCompleter : public ReadlineCompleter {
public:
  explicit Kid3CliCompleter(const QList<CliCommand*>& cmds);
  virtual ~Kid3CliCompleter() override = default;

  virtual QList<QByteArray> getCommandList() const override;
  virtual QList<QByteArray> getParameterList() const override;
  virtual bool updateParameterList(const char* buffer) override;

private:
  Q_DISABLE_COPY(Kid3CliCompleter)

  const QList<CliCommand*>& m_cmds;
  QList<QByteArray> m_commands;
  QList<QByteArray> m_parameters;
};

Kid3CliCompleter::Kid3CliCompleter(const QList<CliCommand*>& cmds)
  : m_cmds(cmds)
{
  m_commands.reserve(cmds.size());
  for (const CliCommand* cmd : cmds) {
    m_commands.append(cmd->name().toLocal8Bit());
  }
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
    for (const CliCommand* cmd : m_cmds) {
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
      QString argTypes = argSpecs.first().remove(QLatin1Char('['))
                                         .remove(QLatin1Char(']'));
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
            frameNames.reserve(Frame::FT_LastFrame - Frame::FT_FirstFrame + 1);
            for (int k = Frame::FT_FirstFrame; k <= Frame::FT_LastFrame; ++k) {
              frameNames.append(
                    Frame::ExtendedType(
                      static_cast<Frame::Type>(k), QLatin1String(""))
                    .getName().toLower().remove(QLatin1Char(' ')).toLocal8Bit());
            }
          }
          m_parameters = frameNames;
          break;
        }
        case 'S':
          // specific command
          if (argSpecs.size() > 1) {
            const QString& valuesStr = argSpecs.at(1);
            int valuesIdx = valuesStr.indexOf(QLatin1String("S = \""));
            if (valuesIdx != -1) {
              const QStringList values =
                  valuesStr.mid(valuesIdx + 4).split(QLatin1String(" | "));
              for (const QString& value : values) {
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


/**
 * Constructor.
 * @param app application context
 * @param io I/O handler
 * @param args command line arguments
 * @param parent parent object
 */
Kid3Cli::Kid3Cli(Kid3Application* app,
                 AbstractCliIO* io, const QStringList& args, QObject* parent) :
  AbstractCli(io, parent),
  m_app(app), m_args(args),
  m_tagMask(Frame::TagV2V1), m_timeoutMs(0), m_fileNameChanged(false)
{
  m_formatters << new JsonCliFormatter(io)
               << new TextCliFormatter(io);
#if QT_VERSION >= 0x050600
  m_formatter = m_formatters.constLast();
#else
  m_formatter = m_formatters.last();
#endif

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
         << new RemoveCommand(this);
  connect(m_app, &Kid3Application::fileSelectionUpdateRequested,
          this, &Kid3Cli::updateSelectedFiles);
  connect(m_app, &Kid3Application::selectedFilesUpdated,
          this, &Kid3Cli::updateSelection);
  connect(m_app, &Kid3Application::selectedFilesChanged,
          this, &Kid3Cli::updateSelection);
#ifdef HAVE_READLINE
  m_completer.reset(new Kid3CliCompleter(m_cmds));
  m_completer->install();
#endif
}

/**
 * Destructor.
 */
Kid3Cli::~Kid3Cli()
{
  // Must not be inline because of forwared declared QScopedPointer.
}

/**
 * Get command for a command line.
 * @param line command line
 * @return command, 0 if no command found.
 */
CliCommand* Kid3Cli::commandForArgs(const QString& line)
{
  if (line.isEmpty())
    return nullptr;

  // Default to the last formatter
#if QT_VERSION >= 0x050600
  m_formatter = m_formatters.constLast();
#else
  m_formatter = m_formatters.last();
#endif

  QStringList args;
  for (auto fmt : m_formatters) {
    args = fmt->parseArguments(line);
    if (fmt->isFormatRecognized()) {
      m_formatter = fmt;
      break;
    }
  }

  if (!args.isEmpty()) {
    const QString& name = args.at(0);
    for (auto it = m_cmds.begin(); it != m_cmds.end(); ++it) { // clazy:exclude=detaching-member
      CliCommand* cmd = *it;
      if (name == cmd->name()) {
        cmd->setArgs(args);
        return cmd;
      }
    }
  }
  return nullptr;
}

/**
 * Display help about available commands.
 * @param cmdName command name, for all commands if empty
 * @param usageMessage true if this is a usage error message
 */
void Kid3Cli::writeHelp(const QString& cmdName, bool usageMessage)
{
  QString msg;
  if (cmdName.isEmpty()) {
    QString tagNumbersStr;
    FOR_ALL_TAGS(tagNr) {
      if (!tagNumbersStr.isEmpty()) {
        tagNumbersStr += QLatin1Char('|');
      }
      tagNumbersStr += QLatin1String(" \"");
      tagNumbersStr += Frame::tagNumberToString(tagNr);
      tagNumbersStr += QLatin1String("\" ");
    }
    msg += tr("Parameter") %
        QLatin1String("\n  P = ") % tr("File path") %
        QLatin1String("\n  U = ") % tr("URL") %
        QLatin1String("\n  T = ") % tr("Tag numbers") %
        tagNumbersStr % QLatin1String("| \"12\" | ...") %
        QLatin1String("\n  N = ") % tr("Frame name") %
        QLatin1String(" \"album\" | \"album artist\" | \"arranger\" | "
                      "\"artist\" | ...") %
        QLatin1String("\n  V = ") % tr("Frame value") %
        QLatin1String("\n  F = ") % tr("Format") %
        QLatin1String("\n  S = ") % tr("Command specific") %
        QLatin1Char('\n') % tr("Available Commands") % QLatin1Char('\n');
  }
  QList<QStringList> cmdStrs;
  int maxLength = 0;
  for (auto it = m_cmds.constBegin(); it != m_cmds.constEnd(); ++it) {
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
  const auto constCmdStrs = cmdStrs;
  for (QStringList strs : constCmdStrs) {
    QString cmdStr = strs.takeFirst();
    cmdStr += QString(maxLength - cmdStr.size() + 2, QLatin1Char(' ')) %
        strs.takeFirst() % QLatin1Char('\n');
    msg += cmdStr;
    while (!strs.isEmpty()) {
      msg += QString(maxLength + 2, QLatin1Char(' ')) %
          strs.takeFirst() % QLatin1Char('\n');
    }
  }
  if (usageMessage) {
    writeError(msg.trimmed(), CliError::Usage);
  } else {
    writeResult(msg.trimmed());
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
  for (const QString& path : paths) {
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
          const QStringList entries = dir.entryList({wildcardPart},
                                    QDir::AllEntries | QDir::NoDotAndDotDot);
          if (!entries.isEmpty()) {
            for (const QString& entry : entries) {
              expandedPath.append(partBefore + entry + partAfter); // clazy:exclude=reserve-candidates
            }
          }
        }
      }
    }
    if (expandedPath.isEmpty()) {
      expandedPaths.append(path); // clazy:exclude=reserve-candidates
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
  for (const QString& fileName : paths) {
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
  FOR_ALL_TAGS(tagNr) {
    m_tagFormat[tagNr] = selection->getTagFormat(tagNr);
  }
  m_fileNameChanged = selection->isFilenameChanged();
  m_detailInfo = selection->getDetailInfo();
}

/**
 * Display information about selected files.
 * @param tagMask tag bits (1 for tag 1, 2 for tag 2)
 */
void Kid3Cli::writeFileInformation(int tagMask)
{
  QVariantMap map;
  if (!m_detailInfo.isEmpty()) {
    map.insert(QLatin1String("format"), m_detailInfo.trimmed());
  }
  if (!m_filename.isEmpty()) {
    map.insert(QLatin1String("fileNameChanged"), m_fileNameChanged);
    map.insert(QLatin1String("fileName"), m_filename);
  }
  FOR_TAGS_IN_MASK(tagNr, tagMask) {
    FrameTableModel* ft = m_app->frameModel(tagNr);
    QVariantList frames;
    for (int row = 0; row < ft->rowCount(); ++row) {
      QString name =
          ft->index(row, FrameTableModel::CI_Enable).data().toString();
      QString value =
          ft->index(row, FrameTableModel::CI_Value).data().toString();
      if (!(tagNr == Frame::Tag_1 ? value.isEmpty() : value.isNull())) {
        QVariant background = ft->index(row, FrameTableModel::CI_Enable)
            .data(Qt::BackgroundColorRole);
        CoreTaggedFileIconProvider* colorProvider =
            m_app->getPlatformTools()->iconProvider();
        bool changed = colorProvider &&
          colorProvider->contextForColor(background) == ColorContext::Marked;
        frames.append(QVariantMap{
                        {QLatin1String("changed"), changed},
                        {QLatin1String("name"), name},
                        {QLatin1String("value"), value},
                      });
      }
    }
    if (!frames.isEmpty()) {
      map.insert(QLatin1String("tag") + Frame::tagNumberToString(tagNr),
                 QVariantMap{
                   {QLatin1String("format"), m_tagFormat[tagNr]},
                   {QLatin1String("frames"), frames}
                 });
    }
  }
  writeResult(QVariantMap{{"taggedFile", map}});
}

/**
 * Write currently active tag mask.
 */
void Kid3Cli::writeTagMask()
{
  QVariantList tags;
  QString tagStr;
  FOR_TAGS_IN_MASK(tagNr, m_tagMask) {
    tags.append(tagNr + 1);
  }
  writeResult(QVariantMap{{"tags", tags}});
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
  writeResult(QVariantMap{
    {QLatin1String("files"),
     listFiles(m_app->getFileProxyModel(), m_app->getRootIndex())}
  });
}

/**
 * List files.
 *
 * @param model file proxy model
 * @param parent index of parent item
 *
 * @return list with file properties.
 */
QVariantList Kid3Cli::listFiles(const FileProxyModel* model,
                                const QModelIndex& parent)
{
  QVariantList lst;
  if (!model->hasChildren(parent))
    return lst;

  m_app->updateCurrentSelection();
#if QT_VERSION >= 0x050e00
  const QList<QPersistentModelIndex>& selLst = m_app->getCurrentSelection();
  QSet<QPersistentModelIndex> selection(selLst.constBegin(), selLst.constEnd());
#else
  QSet<QPersistentModelIndex> selection = m_app->getCurrentSelection().toSet();
#endif
  for (int row = 0; row < model->rowCount(parent); ++row) {
    QModelIndex idx(model->index(row, 0, parent));
    QVariantMap map;
    map.insert(QLatin1String("selected"), selection.contains(idx));
    if (TaggedFile* taggedFile = FileProxyModel::getTaggedFileOfIndex(idx)) {
      taggedFile = FileProxyModel::readTagsFromTaggedFile(taggedFile);
      map.insert(QLatin1String("changed"), taggedFile->isChanged());
      QVariantList tags;
      FOR_ALL_TAGS(tagNr) {
        if (taggedFile->hasTag(tagNr)) {
          tags.append(1 + tagNr);
        }
      }
      map.insert(QLatin1String("tags"), tags);
      map.insert(QLatin1String("fileName"), taggedFile->getFilename());
    } else {
      QVariant value(model->data(idx));
      if (value.isValid()) {
        map.insert(QLatin1String("fileName"), value.toString());
      }
    }
    if (model->hasChildren(idx)) {
      map.insert(QLatin1String("files"), listFiles(model, idx));
    }
    lst.append(map);
  }
  return lst;
}

/**
 * Respond with an error message
 * @param errorCode error code
 */
void Kid3Cli::writeErrorCode(CliError errorCode)
{
  m_formatter->writeError(errorCode);
}

/**
 * Write a line to standard error.
 * @param line line to write
 */
void Kid3Cli::writeErrorLine(const QString& line)
{
  m_formatter->writeError(line);
}

/**
 * Respond with an error message.
 * @param msg error message
 * @param errorCode error code
 */
void Kid3Cli::writeError(const QString& msg, CliError errorCode)
{
  m_formatter->writeError(msg, errorCode);
}

/**
 * Write result of command.
 * @param str result as string
 */
void Kid3Cli::writeResult(const QString& str)
{
  m_formatter->writeResult(str);
}

/**
 * Write result of command.
 * @param strs result as string list
 */
void Kid3Cli::writeResult(const QStringList& strs)
{
  m_formatter->writeResult(strs);
}

/**
 * Write result of command.
 * @param map result as map
 */
void Kid3Cli::writeResult(const QVariantMap& map)
{
  m_formatter->writeResult(map);
}

/**
 * Called when a command is finished.
 */
void Kid3Cli::finishWriting()
{
  m_formatter->finishWriting();
  m_formatter->clear();
}

/**
 * Process command line.
 * @param line command line
 */
void Kid3Cli::readLine(const QString& line)
{
  if (line.isNull()) {
    // Terminate if EOF is received.
    terminate();
    return;
  }
  flushStandardOutput();
  CliCommand* cmd = commandForArgs(line);
  if (cmd) {
    connect(cmd, &CliCommand::finished, this, &Kid3Cli::onCommandFinished);
    cmd->execute();
  } else {
    if (!m_formatter->isIncomplete()) {
      QString errorMsg = m_formatter->getErrorMessage();
      if (errorMsg.isEmpty()) {
        writeErrorCode(CliError::MethodNotFound);
      } else {
        writeErrorLine(errorMsg);
      }
      finishWriting();
    }
    promptNextLine();
  }
}

/**
 * Called when a command is finished.
 */
void Kid3Cli::onCommandFinished() {
  if (auto cmd = qobject_cast<CliCommand*>(sender())) {
    disconnect(cmd, &CliCommand::finished, this, &Kid3Cli::onCommandFinished);
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
  if (auto cmd = qobject_cast<CliCommand*>(sender())) {
    disconnect(cmd, &CliCommand::finished, this, &Kid3Cli::onArgCommandFinished);
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
  const QStringList args = m_args.mid(1);
  QStringList paths;
  bool isCommand = false;
  for (const QString& arg : args) {
    if (isCommand) {
      m_argCommands.append(arg);
      isCommand = false;
    } else if (arg == QLatin1String("-c")) {
      isCommand = true;
    } else if (arg == QLatin1String("-h") || arg == QLatin1String("--help")) {
      writeLine(QLatin1String("kid3-cli " VERSION " (c) " RELEASE_YEAR
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
  m_app->readConfig();
  connect(m_app, &Kid3Application::directoryOpened,
    this, &Kid3Cli::onInitialDirectoryOpened);
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
  disconnect(m_app, &Kid3Application::directoryOpened,
    this, &Kid3Cli::onInitialDirectoryOpened);
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
        finishWriting();
        setReturnCode(1);
      }
    }
    terminate();
    return;
  }

  QString line = m_argCommands.takeFirst();
  CliCommand* cmd = commandForArgs(line);
  if (cmd) {
    connect(cmd, &CliCommand::finished, this, &Kid3Cli::onArgCommandFinished);
    cmd->execute();
  } else {
    QString errorMsg = m_formatter->getErrorMessage();
    if (errorMsg.isEmpty()) {
      errorMsg = tr("Unknown command '%1', -h for help.").arg(line);
    }
    writeErrorLine(errorMsg);
    finishWriting();
    setReturnCode(1);
    terminate();
  }
}
