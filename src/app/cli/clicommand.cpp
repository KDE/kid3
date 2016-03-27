/**
 * \file clicommand.cpp
 * Command line interface commands.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 11 Aug 2013
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

#include "clicommand.h"
#include <QStringList>
#include <QTimer>
#include <QDir>
#include <QItemSelectionModel>
#include "kid3cli.h"
#include "kid3application.h"
#include "fileproxymodel.h"
#include "frametablemodel.h"
#include "importconfig.h"
#include "exportconfig.h"
#include "filterconfig.h"
#include "fileconfig.h"
#include "rendirconfig.h"
#include "batchimportconfig.h"
#include "batchimporter.h"
#include "downloadclient.h"
#include "dirrenamer.h"
#include "audioplayer.h"

/** Default command timeout in milliseconds. */
static const int DEFAULT_TIMEOUT_MS = 3000;

/**
 * Constructor.
 * @param processor command line processor
 * @param name name with which command is invoked
 * @param help help text for command
 * @param argspec argument specification
 */
CliCommand::CliCommand(Kid3Cli* processor,
                       const QString& name, const QString& help,
                       const QString& argspec) :
  QObject(processor), m_processor(processor), m_name(name), m_help(help),
  m_argspec(argspec), m_timerId(0), m_timeoutMs(DEFAULT_TIMEOUT_MS), m_result(0)
{
}

/**
 * Destructor.
 */
CliCommand::~CliCommand()
{
}

/**
 * Reset state to defaults.
 */
void CliCommand::clear()
{
  if (m_timerId != 0) {
    killTimer(m_timerId);
    m_timerId = 0;
  }
  m_errorMsg.clear();
  m_args.clear();
  m_result = 0;
}

/**
 * Execute command.
 */
void CliCommand::execute()
{
  if (m_timerId != 0) {
    killTimer(m_timerId);
    m_timerId = 0;
  }
  int msec = m_processor->getTimeout();
  if (msec == 0) {
    msec = getTimeout();
  }
  if (msec > 0) {
    m_timerId = startTimer(msec);
  }
  connectResultSignal();
  startCommand();
}

/**
 * Terminate command.
 */
void CliCommand::terminate() {
  if (m_timerId != 0) {
    killTimer(m_timerId);
    m_timerId = 0;
  }
  disconnectResultSignal();
  emit finished();
}

/**
 * Connect signals used to emit finished().
 * This method is called after startCommand(). The default implementation
 * invokes terminate() in the event loop. It can be overridden to connect
 * signals connected to terminate() to signal termination of the command.
 */
void CliCommand::connectResultSignal()
{
  QTimer::singleShot(0, this, SLOT(terminate()));
}

/**
 * Disconnect signals used to emit finished().
 * This method is called from terminate(). The default implementation
 * does nothing. It can be overridden to disconnect signals connected
 * in connectResultSignal().
 */
void CliCommand::disconnectResultSignal()
{
}

/**
 * Called on timeout.
 * @param event timer event
 */
void CliCommand::timerEvent(QTimerEvent*) {
  setError(tr("Timeout"));
  terminate();
}

/**
 * Get parameter for task mask.
 * @param nr index in args()
 * @param useDefault if true use cli()->tagMask() if no parameter found
 * @return tag versions.
 */
Frame::TagVersion CliCommand::getTagMaskParameter(int nr,
                                                  bool useDefault) const
{
  int tagMask = 0;
  if (m_args.size() > nr) {
    const QString& tagStr = m_args.at(nr);
    if (!tagStr.isEmpty() && tagStr.at(0).isDigit()) {
      if (tagStr.contains(QLatin1Char('1')))
        tagMask |= 1;
      if (tagStr.contains(QLatin1Char('2')))
        tagMask |= 2;
      if (tagMask == 0)
        tagMask = tagStr.toInt();
    }
  }
  if (tagMask == 0 && useDefault) {
    tagMask = m_processor->tagMask();
  }
  return Frame::tagVersionCast(tagMask);
}

/**
 * Show usage of command.
 */
void CliCommand::showUsage()
{
  cli()->writeLine(tr("Usage:"));
  cli()->writeHelp(name());
  setError(QLatin1String("_Usage"));
}



HelpCommand::HelpCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("help"), tr("Help"),
             QLatin1String("[S]\nS = ") + tr("Command name"))
{
}

void HelpCommand::startCommand()
{
  cli()->writeHelp(args().size() > 1 ? args().at(1) : QString());
}


TimeoutCommand::TimeoutCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("timeout"), tr("Overwrite timeout"),
             QLatin1String("[S]\nS = \"default\" | \"off\" | ") + tr("Time") +
             QLatin1String(" [ms]"))
{
}

void TimeoutCommand::startCommand()
{
  int cliTimeout = cli()->getTimeout();
  if (args().size() > 1) {
    const QString& val = args().at(1);
    if (val == QLatin1String("off")) {
      cliTimeout = -1;
    } else if (val == QLatin1String("default")) {
      cliTimeout = 0;
    } else {
      bool ok;
      int ms = val.toInt(&ok);
      if (ok && ms > 0) {
        cliTimeout = ms;
      }
    }
    cli()->setTimeout(cliTimeout);
  }
  QString msg(tr("Timeout"));
  msg += QLatin1String(": ");
  if (cliTimeout < 0) {
    msg += QLatin1String("off");
  } else if (cliTimeout == 0) {
    msg += QLatin1String("default");
  } else {
    msg += QString::number(cliTimeout);
    msg += QLatin1String(" ms");
  }
  cli()->writeLine(msg);
}


QuitCommand::QuitCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("exit"), tr("Quit application"),
             QLatin1String("[S]\nS = \"force\""))
{
}

void QuitCommand::startCommand()
{
  if (cli()->app()->isModified() && !cli()->app()->getDirName().isEmpty()) {
    if (!(args().size() > 1 && args().at(1) == QLatin1String("force"))) {
      cli()->writeLine(tr("The current directory has been modified."));
      cli()->writeLine(tr("Type 'exit force' to quit."));
      terminate();
      return;
    }
  }
  disconnect(this, SIGNAL(finished()), cli(), SLOT(onCommandFinished()));
  cli()->terminate();
}

void QuitCommand::connectResultSignal()
{
  // Do not signal finished() to avoid printing prompt.
}


CdCommand::CdCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("cd"), tr("Change directory"),
             QLatin1String("[P]"))
{
}

void CdCommand::startCommand()
{
  QStringList paths;
  if (args().size() > 1) {
    paths = args().mid(1);
  } else {
    paths.append(QDir::homePath());
  }
  if (!cli()->openDirectory(cli()->expandWildcards(paths))) {
    setError(tr("%1 does not exist").arg(paths.join(QLatin1String(", "))));
    terminate();
  }
}

void CdCommand::connectResultSignal()
{
  connect(cli()->app(), SIGNAL(directoryOpened()),
    this, SLOT(terminate()));
}

void CdCommand::disconnectResultSignal()
{
  disconnect(cli()->app(), SIGNAL(directoryOpened()),
    this, SLOT(terminate()));
}


PwdCommand::PwdCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("pwd"),
             tr("Print the filename of the current working directory"))
{
}

void PwdCommand::startCommand()
{
  QString path = cli()->app()->getDirPath();
  if (path.isNull()) {
    path = QDir::currentPath();
    cli()->app()->openDirectory(QStringList() << path);
  }
  cli()->writeLine(path);
}


LsCommand::LsCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("ls"), tr("Directory list"))
{
  setTimeout(10000);
}

void LsCommand::startCommand()
{
  cli()->writeFileList();
}


SaveCommand::SaveCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("save"), tr("Saves the changed files"))
{
}

void SaveCommand::startCommand()
{
  QStringList errorFiles = cli()->app()->saveDirectory();
  if (errorFiles.isEmpty()) {
    cli()->updateSelection();
  } else {
    setError(tr("Error while writing file:\n") +
             errorFiles.join(QLatin1String("\n")));
  }
}


SelectCommand::SelectCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("select"), tr("Select file"),
             QLatin1String("[P|S]\n"
             "S = \"all\" | \"none\" | \"first\" | \"previous\" | \"next\""))
{
}

void SelectCommand::startCommand()
{
  if (args().size() > 1) {
    const QString& param = args().at(1);
    if (param == QLatin1String("all")) {
      cli()->app()->selectAllFiles();
    } else if (param == QLatin1String("none")) {
      cli()->app()->deselectAllFiles();
    } else if (param == QLatin1String("first")) {
      setResult(cli()->app()->firstFile(true) ? 0 : 1);
    } else if (param == QLatin1String("previous")) {
      setResult(cli()->app()->previousFile(true) ? 0 : 1);
    } else if (param == QLatin1String("next")) {
      setResult(cli()->app()->nextFile(true) ? 0 : 1);
    } else {
      QStringList paths = args().mid(1);
      if (!cli()->selectFile(cli()->expandWildcards(paths))) {
        setError(tr("%1 not found").arg(paths.join(QLatin1String(", "))));
      }
    }
  } else {
    cli()->updateSelection();
  }
}


TagCommand::TagCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("tag"), tr("Select tag"),
             QLatin1String("[T]"))
{
}

void TagCommand::startCommand()
{
  Frame::TagVersion tagMask = getTagMaskParameter(1, false);
  if (tagMask != Frame::TagNone) {
    cli()->setTagMask(tagMask);
  } else {
    cli()->writeTagMask();
  }
}


GetCommand::GetCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("get"), tr("Get tag frame"),
             QLatin1String("[N|S] [T]\nS = \"all\""))
{
}

void GetCommand::startCommand()
{
  int numArgs = args().size();
  QString name = numArgs > 1 ? args().at(1) : QLatin1String("all");
  Frame::TagVersion tagMask = getTagMaskParameter(2);
  if (name == QLatin1String("all")) {
    cli()->writeFileInformation(tagMask);
  } else {
    foreach (Frame::TagNumber tagNr, Frame::tagNumbersFromMask(tagMask)) {
      QString value = cli()->app()->getFrame(Frame::tagVersionFromNumber(tagNr),
                                             name);
      if (!value.isEmpty()) {
        cli()->writeLine(value);
        break;
      }
    }
  }
}


SetCommand::SetCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("set"), tr("Set tag frame"),
             QLatin1String("N V [T]"))
{
}

void SetCommand::startCommand()
{
  int numArgs = args().size();
  if (numArgs > 2) {
    const QString& name = args().at(1);
    const QString& value = args().at(2);
    Frame::TagVersion tagMask = getTagMaskParameter(3);
    if (cli()->app()->setFrame(tagMask, name, value)) {
      cli()->updateSelectedFiles();
      cli()->updateSelection();
    } else {
      setError(tr("Could not set \"%1\" for %2").arg(value).arg(name));
    }
  } else {
    showUsage();
  }
}


RevertCommand::RevertCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("revert"),
             tr("Revert"))
{
}

void RevertCommand::startCommand()
{
  cli()->app()->revertFileModifications();
}


ImportCommand::ImportCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("import"),
             tr("Import from file or clipboard"),
             QLatin1String("P S [T]\nP S = ") +
             tr("File path") + QLatin1Char(' ') + tr("Format name") +
             QLatin1String(" | ") +
             QLatin1String("clipboard ") + tr("Format name") +
             QLatin1String("\n    | tags ") + tr("Source") + QLatin1Char(' ') +
             tr("Extraction"))
{
}

void ImportCommand::startCommand()
{
  int numArgs = args().size();
  if (numArgs > 3 && args().at(1) == QLatin1String("tags")) {
    const QString& source = args().at(2);
    const QString& extraction = args().at(3);
    Frame::TagVersion tagMask = getTagMaskParameter(4);
    cli()->app()->importFromTags(tagMask, source, extraction);
  } else if (numArgs > 2) {
    const QString& path = args().at(1);
    const QString& fmtName = args().at(2);
    bool ok;
    int fmtIdx = fmtName.toInt(&ok);
    if (!ok) {
      fmtIdx = ImportConfig::instance().importFormatNames().indexOf(fmtName);
      if (fmtIdx == -1) {
        QString errMsg = tr("%1 not found.").arg(fmtName);
        errMsg += QLatin1Char('\n');
        errMsg += tr("Available");
        errMsg += QLatin1String(": ");
        errMsg += ImportConfig::instance().importFormatNames().join(
              QLatin1String(", "));
        errMsg += QLatin1Char('.');
        setError(errMsg);
        return;
      }
    }
    Frame::TagVersion tagMask = getTagMaskParameter(3);
    if (!cli()->app()->importTags(tagMask, path, fmtIdx)) {
        setError(tr("Error"));
    }
  } else {
    showUsage();
  }
}


BatchImportCommand::BatchImportCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("autoimport"),
             tr("Automatic import"), QLatin1String("[S] [T]\nS = ") +
             tr("Profile name"))
{
  setTimeout(60000);
}

void BatchImportCommand::startCommand()
{
  int numArgs = args().size();
  const QString& profileName = numArgs > 1
      ? args().at(1) : QLatin1String("All");
  Frame::TagVersion tagMask = getTagMaskParameter(2);
  if (!cli()->app()->batchImport(profileName, tagMask)) {
    QString errMsg = tr("%1 not found.").arg(profileName);
    errMsg += QLatin1Char('\n');
    errMsg += tr("Available");
    errMsg += QLatin1String(": ");
    errMsg += BatchImportConfig::instance().profileNames().join(
          QLatin1String(", "));
    errMsg += QLatin1Char('.');
    setError(errMsg);
    terminate();
  }
}

void BatchImportCommand::connectResultSignal()
{
  BatchImporter* importer = cli()->app()->getBatchImporter();
  connect(importer, SIGNAL(reportImportEvent(int,QString)),
          this, SLOT(onReportImportEvent(int,QString)));
  connect(importer, SIGNAL(finished()),
          this, SLOT(terminate()));
}

void BatchImportCommand::disconnectResultSignal()
{
  BatchImporter* importer = cli()->app()->getBatchImporter();
  disconnect(importer, SIGNAL(reportImportEvent(int,QString)),
             this, SLOT(onReportImportEvent(int,QString)));
  disconnect(importer, SIGNAL(finished()),
             this, SLOT(terminate()));
}

void BatchImportCommand::onReportImportEvent(int type, const QString& text)
{
  QString eventText;
  switch (type) {
  case BatchImporter::ReadingDirectory:
    eventText = tr("Reading Directory");
    break;
  case BatchImporter::Started:
    eventText = tr("Started");
    break;
  case BatchImporter::SourceSelected:
    eventText = tr("Source");
    break;
  case BatchImporter::QueryingAlbumList:
    eventText = tr("Querying");
    break;
  case BatchImporter::FetchingTrackList:
  case BatchImporter::FetchingCoverArt:
    eventText = tr("Fetching");
    break;
  case BatchImporter::TrackListReceived:
    eventText = tr("Data received");
    break;
  case BatchImporter::CoverArtReceived:
    eventText = tr("Cover");
    break;
  case BatchImporter::Finished:
    eventText = tr("Finished");
    break;
  case BatchImporter::Aborted:
    eventText = tr("Aborted");
    break;
  case BatchImporter::Error:
    eventText = tr("Error");
  }
  if (!text.isEmpty()) {
    eventText += QLatin1String(": ");
    eventText += text;
  }
  cli()->writeLine(eventText);
}


AlbumArtCommand::AlbumArtCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("albumart"),
             tr("Download album cover artwork"),
             QLatin1String("U [S]\nS = \"all\""))
{
  setTimeout(10000);
}

void AlbumArtCommand::startCommand()
{
  int numArgs = args().size();
  if (numArgs > 1) {
    const QString& url = args().at(1);
    cli()->app()->downloadImage(url,
                  numArgs > 2 && args().at(2) == QLatin1String("all"));
  } else {
    showUsage();
    terminate();
  }
}

void AlbumArtCommand::connectResultSignal()
{
  DownloadClient* downloadClient = cli()->app()->getDownloadClient();
  connect(downloadClient, SIGNAL(downloadFinished(QByteArray,QString,QString)),
          this, SLOT(onDownloadFinished(QByteArray,QString,QString)));
}

void AlbumArtCommand::disconnectResultSignal()
{
  DownloadClient* downloadClient = cli()->app()->getDownloadClient();
  disconnect(downloadClient,
             SIGNAL(downloadFinished(QByteArray,QString,QString)),
             this, SLOT(onDownloadFinished(QByteArray,QString,QString)));
}

void AlbumArtCommand::onDownloadFinished(
    const QByteArray& data, const QString& mimeType, const QString& url)
{
  cli()->app()->imageDownloaded(data, mimeType, url);
  terminate();
}


ExportCommand::ExportCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("export"),
             tr("Export to file or clipboard"),
             QLatin1String("P S [T]\nS = ") + tr("Format name"))
{
}

void ExportCommand::startCommand()
{
  int numArgs = args().size();
  if (numArgs > 2) {
    const QString& path = args().at(1);
    const QString& fmtName = args().at(2);
    bool ok;
    int fmtIdx = fmtName.toInt(&ok);
    if (!ok) {
      fmtIdx = ExportConfig::instance().exportFormatNames().indexOf(fmtName);
      if (fmtIdx == -1) {
        QString errMsg = tr("%1 not found.").arg(fmtName);
        errMsg += QLatin1Char('\n');
        errMsg += tr("Available");
        errMsg += QLatin1String(": ");
        errMsg += ExportConfig::instance().exportFormatNames().join(
              QLatin1String(", "));
        errMsg += QLatin1Char('.');
        setError(errMsg);
        return;
      }
    }
    Frame::TagVersion tagMask = getTagMaskParameter(3);
    if (!cli()->app()->exportTags(tagMask, path, fmtIdx)) {
      setError(tr("Error"));
    }
  } else {
    showUsage();
  }
}


PlaylistCommand::PlaylistCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("playlist"), tr("Create playlist"))
{
}

void PlaylistCommand::startCommand()
{
  if (!cli()->app()->writePlaylist()) {
    setError(tr("Error"));
  }
}


FilenameFormatCommand::FilenameFormatCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("filenameformat"),
             tr("Apply filename format"))
{
}

void FilenameFormatCommand::startCommand()
{
  cli()->app()->applyFilenameFormat();
}


TagFormatCommand::TagFormatCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("tagformat"), tr("Apply tag format"))
{
}

void TagFormatCommand::startCommand()
{
  cli()->app()->applyTagFormat();
}


TextEncodingCommand::TextEncodingCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("textencoding"),
             tr("Apply text encoding"))
{
}

void TextEncodingCommand::startCommand()
{
  cli()->app()->applyTextEncoding();
}


RenameDirectoryCommand::RenameDirectoryCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("renamedir"), tr("Rename directory"),
       QLatin1String("[F] [S] [T]\nS = \"create\" | \"rename\" | \"dryrun\"")),
  m_dryRun(false)
{
}

void RenameDirectoryCommand::startCommand()
{
  Frame::TagVersion tagMask = Frame::TagNone;
  QString format;
  bool create = false;
  m_dryRun = false;
  for (int i = 1; i < args().size(); ++i) {
    bool ok = false;
    if (tagMask == Frame::TagNone) {
      tagMask = getTagMaskParameter(i, false);
      ok = tagMask != Frame::TagNone;
    }
    if (!ok) {
      const QString& param = args().at(i);
      if (param == QLatin1String("create")) {
        create = true;
      } else if (param == QLatin1String("rename")) {
        create = false;
      } else if (param == QLatin1String("dryrun")) {
        m_dryRun = true;
      } else if (format.isEmpty()) {
        format = param;
      }
    }
  }
  if (tagMask == Frame::TagNone) {
    tagMask = cli()->tagMask();
  }
  if (format.isEmpty()) {
    format = RenDirConfig::instance().dirFormat();
  }

  if (!cli()->app()->renameDirectory(tagMask, format, create)) {
    terminate();
  }
}

void RenameDirectoryCommand::connectResultSignal()
{
  DirRenamer* renamer = cli()->app()->getDirRenamer();
  connect(renamer, SIGNAL(actionScheduled(QStringList)),
          this, SLOT(onActionScheduled(QStringList)));
  connect(cli()->app(), SIGNAL(renameActionsScheduled()),
          this, SLOT(onRenameActionsScheduled()));
}

void RenameDirectoryCommand::disconnectResultSignal()
{
  DirRenamer* renamer = cli()->app()->getDirRenamer();
  disconnect(renamer, SIGNAL(actionScheduled(QStringList)),
             this, SLOT(onActionScheduled(QStringList)));
  disconnect(cli()->app(), SIGNAL(renameActionsScheduled()),
             this, SLOT(onRenameActionsScheduled()));
}

void RenameDirectoryCommand::onActionScheduled(const QStringList& actionStrs)
{
  QString str = actionStrs.at(0);
  if (actionStrs.size() > 1) {
    str += QLatin1String("  ");
    str += actionStrs.at(1);
  }
  if (actionStrs.size() > 2) {
    str += QLatin1String("\n  ");
    str += actionStrs.at(2);
  }
  cli()->writeLine(str);
}

void RenameDirectoryCommand::onRenameActionsScheduled()
{
  if (!m_dryRun) {
    QString errMsg = cli()->app()->performRenameActions();
    if (errMsg.isEmpty()) {
      cli()->app()->deselectAllFiles();
    } else {
      setError(errMsg);
    }
  }
  terminate();
}


NumberTracksCommand::NumberTracksCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("numbertracks"), tr("Number tracks"),
             QLatin1String("[S] [T]\nS = ") + tr("Track number"))
{
}

void NumberTracksCommand::startCommand()
{
  int numArgs = args().size();
  int firstTrackNr = 1;
  bool ok = false;
  if (numArgs > 1) {
    firstTrackNr = args().at(1).toInt(&ok);
  }
  if (!ok) {
    firstTrackNr = 1;
  }
  Frame::TagVersion tagMask = getTagMaskParameter(2);
  Kid3Application::NumberTrackOptions options;
  options |= Kid3Application::NumberTracksEnabled;
  options |= Kid3Application::NumberTracksResetCounterForEachDirectory;
  cli()->app()->numberTracks(firstTrackNr, 0, tagMask, options);
}


FilterCommand::FilterCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("filter"), tr("Filter"),
             QLatin1String("F|S\nS = ") + tr("Filter name"))
{
  setTimeout(60000);
}

void FilterCommand::startCommand()
{
  if (args().size() > 1) {
    QString expression = args().at(1);
    int fltIdx = FilterConfig::instance().filterNames().indexOf(expression);
    if (fltIdx != -1) {
      expression = FilterConfig::instance().filterExpressions().at(fltIdx);
    } else if (!expression.isEmpty() &&
               !expression.contains(QLatin1Char('%'))) {
      // Probably an invalid expression
      QString errMsg = tr("%1 not found.").arg(expression);
      errMsg += QLatin1Char('\n');
      errMsg += tr("Available");
      errMsg += QLatin1String(": ");
      errMsg += FilterConfig::instance().filterNames().join(
            QLatin1String(", "));
      errMsg += QLatin1Char('.');
      setError(errMsg);
      terminate();
      return;
    }
    cli()->app()->applyFilter(expression);
  } else {
    showUsage();
    terminate();
  }
}

void FilterCommand::connectResultSignal()
{
  connect(cli()->app(), SIGNAL(fileFiltered(int,QString)),
          this, SLOT(onFileFiltered(int,QString)));
}

void FilterCommand::disconnectResultSignal()
{
  cli()->app()->abortFilter();
  disconnect(cli()->app(), SIGNAL(fileFiltered(int,QString)),
             this, SLOT(onFileFiltered(int,QString)));
}

void FilterCommand::onFileFiltered(int type, const QString& fileName)
{
  QString eventText;
  bool finish = false;
  switch (type) {
  case FileFilter::Started:
    eventText = tr("Started");
    break;
  case FileFilter::Directory:
    eventText = QLatin1String("  ") + fileName;
    break;
  case FileFilter::ParseError:
    eventText = QLatin1String("parse error");
    break;
  case FileFilter::FilePassed:
    eventText = QLatin1String("+ ") + fileName;
    break;
  case FileFilter::FileFilteredOut:
    eventText = QLatin1String("- ") + fileName;
    break;
  case FileFilter::Finished:
    eventText = tr("Finished");
    finish = true;
    break;
  case FileFilter::Aborted:
    eventText = tr("Aborted");
    finish = true;
    break;
  }
  cli()->writeLine(eventText);
  if (finish) {
    terminate();
  }
}


ToId3v24Command::ToId3v24Command(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("to24"), tr("Convert ID3v2.3 to ID3v2.4"))
{
}

void ToId3v24Command::startCommand()
{
  cli()->app()->convertToId3v24();
}


ToId3v23Command::ToId3v23Command(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("to23"), tr("Convert ID3v2.4 to ID3v2.3"))
{
}

void ToId3v23Command::startCommand()
{
  cli()->app()->convertToId3v23();
}


TagToFilenameCommand::TagToFilenameCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("fromtag"), tr("Filename from tag"),
             QLatin1String("[F] [T]"))
{
}

void TagToFilenameCommand::startCommand()
{
  Frame::TagVersion tagMask = Frame::TagNone;
  QString format;
  for (int i = 1; i < qMin(args().size(), 3); ++i) {
    bool ok = false;
    if (tagMask == Frame::TagNone) {
      tagMask = getTagMaskParameter(i, false);
      ok = tagMask != Frame::TagNone;
    }
    if (!ok && format.isEmpty()) {
      format = args().at(i);
    }
  }
  if (tagMask == Frame::TagNone) {
    tagMask = cli()->tagMask();
  }
  if (!format.isEmpty()) {
    FileConfig::instance().setToFilenameFormat(format);
  }
  cli()->app()->getFilenameFromTags(tagMask);
}


FilenameToTagCommand::FilenameToTagCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("totag"), tr("Tag from filename"),
             QLatin1String("[F] [T]"))
{
}

void FilenameToTagCommand::startCommand()
{
  Frame::TagVersion tagMask = Frame::TagNone;
  QString format;
  for (int i = 1; i < qMin(args().size(), 3); ++i) {
    bool ok = false;
    if (tagMask == Frame::TagNone) {
      tagMask = getTagMaskParameter(i, false);
      ok = tagMask != Frame::TagNone;
    }
    if (!ok && format.isEmpty()) {
      format = args().at(i);
    }
  }
  if (tagMask == Frame::TagNone) {
    tagMask = cli()->tagMask();
  }
  if (!format.isEmpty()) {
    FileConfig::instance().setFromFilenameFormat(format);
  }
  cli()->app()->getTagsFromFilename(tagMask);
}


TagToOtherTagCommand::TagToOtherTagCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("syncto"), tr("Tag to other tag"),
             QLatin1String("T"))
{
}

void TagToOtherTagCommand::startCommand()
{
  Frame::TagVersion tagMask = getTagMaskParameter(1, false);
  if (tagMask != Frame::TagNone) {
    cli()->app()->copyToOtherTag(tagMask);
  } else {
    showUsage();
  }
}


CopyCommand::CopyCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("copy"), tr("Copy"),
             QLatin1String("[T]"))
{
}

void CopyCommand::startCommand()
{
  Frame::TagVersion tagMask = getTagMaskParameter(1);
  cli()->app()->copyTags(tagMask);
}


PasteCommand::PasteCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("paste"), tr("Paste"),
             QLatin1String("[T]"))
{
}

void PasteCommand::startCommand()
{
  Frame::TagVersion tagMask = getTagMaskParameter(1);
  cli()->app()->pasteTags(tagMask);
}


RemoveCommand::RemoveCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("remove"), tr("Remove"),
             QLatin1String("[T]"))
{
}

void RemoveCommand::startCommand()
{
  Frame::TagVersion tagMask = getTagMaskParameter(1);
  cli()->app()->removeTags(tagMask);
}


#if defined HAVE_PHONON || QT_VERSION >= 0x050000
PlayCommand::PlayCommand(Kid3Cli* processor) :
  CliCommand(processor, QLatin1String("play"), tr("Play"),
             QLatin1String("[S]\n"
             "S = \"pause\" | \"stop\" | \"previous\" | \"next\""))
{
}

void PlayCommand::startCommand()
{
  AudioPlayer* player = 0;
  QString param;
  if (args().size() > 1) {
    param = args().at(1);
    player = cli()->app()->getAudioPlayer();
  }
  if (param == QLatin1String("pause")) {
    if (player) {
      player->playOrPause();
    }
  } else if (param == QLatin1String("stop")) {
    if (player) {
      player->stop();
    }
  } else if (param == QLatin1String("previous")) {
    if (player) {
      player->previous();
    }
  } else if (param == QLatin1String("next")) {
    if (player) {
      player->next();
    }
  } else {
    cli()->app()->playAudio();
  }
}
#endif
