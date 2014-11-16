/**
 * \file clicommand.h
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

#ifndef CLICOMMAND_H
#define CLICOMMAND_H

#include <QObject>
#include "trackdata.h"
#include "filefilter.h"
#include "config.h"

class QModelIndex;
class Kid3Cli;

/**
 * Base class for command line interface command.
 */
class CliCommand : public QObject {
  Q_OBJECT
public:
  /**
   * Destructor.
   */
  virtual ~CliCommand();

  /**
   * Reset state to defaults.
   */
  virtual void clear();

  /**
   * Execute command.
   */
  virtual void execute();

  /**
   * Get command name.
   * @return name with which command is invoked.
   */
  QString name() const { return m_name; }

  /**
   * Get help text for command.
   * @return help text for command.
   */
  QString help() const { return m_help; }

  /**
   * Get argument specification
   * @return argument specification for command.
   */
  QString argumentSpecification() const { return m_argspec; }

  /**
   * Get description of error.
   * @return error message, empty if no error occurred.
   */
  QString getErrorMessage() const { return m_errorMsg; }

  /**
   * Check if an error occurred.
   * @return true if an error exists.
   */
  bool hasError() const { return !m_errorMsg.isEmpty(); }

  /**
   * Set error message.
   * @param errorMsg error message, null string to clear error
   */
  void setError(const QString& errorMsg) { m_errorMsg = errorMsg; }

  /**
   * Get timeout for operation.
   * @return timeout in milliseconds.
   */
  int getTimeout() const { return m_timeoutMs; }

  /**
   * Set timeout for operation.
   * @param msec timeout in milliseconds, -1 for no timeout
   */
  void setTimeout(int msec) { m_timeoutMs = msec; }

  /**
   * Get processor handling commands.
   * @return processor.
   */
  Kid3Cli* cli() const { return m_processor; }

  /**
   * Get list of arguments.
   * @return arguments, the first element is the command name.
   */
  const QStringList& args() const { return m_args; }

  /**
   * Set list of arguments.
   * @param args arguments, the first element must be the command name
   */
  void setArgs(const QStringList& args) { m_args = args; }

  /**
   * Get result code of command.
   * @return result code, default is 0 for OK.
   */
  int result() const { return m_result; }

  /**
   * Set result code of command.
   * @param result result code
   */
  void setResult(int result) { m_result = result; }

signals:
  /**
   * Emitted when the command is finished.
   */
  void finished();

protected slots:
  /**
   * Terminate command.
   * Must be invoked when command is finished, which is set up in
   * connectResultSignal().
   */
  void terminate();

protected:
  /**
   * Constructor.
   * @param processor command line processor
   * @param name name with which command is invoked
   * @param help help text for command
   * @param argspec argument specification
   */
  CliCommand(Kid3Cli* processor, const QString& name, const QString& help,
             const QString& argspec = QString());

  /**
   * Called on timeout.
   * @param event timer event
   */
  void timerEvent(QTimerEvent* event);

  /**
   * Start specific command.
   * This method has to be implemented by the specific subclasses
   * to execute the specific command. It should invoke terminate()
   * when the command is finished.
   */
  virtual void startCommand() = 0;

  /**
   * Connect signals used to emit finished().
   * This method is called after startCommand(). The default implementation
   * invokes terminate() in the event loop. It can be overridden to connect
   * signals connected to terminate() to signal termination of the command.
   */
  virtual void connectResultSignal();

  /**
   * Stop command.
   * This method is called from terminate(). The default implementation
   * does nothing. It can be overridden to disconnect signals connected
   * in startCommand().
   */
  virtual void disconnectResultSignal();

  /**
   * Get parameter for task mask.
   * @param nr index in args()
   * @param useDefault if true use cli()->tagMask() if no parameter found
   * @return tag versions.
   */
  Frame::TagVersion getTagMaskParameter(int nr,
                                        bool useDefault = true) const;

  /**
   * Show usage of command.
   */
  void showUsage();

private:
  Kid3Cli* m_processor;
  const QString m_name;
  const QString m_help;
  const QString m_argspec;
  QStringList m_args;
  QString m_errorMsg;
  int m_timerId;
  int m_timeoutMs;
  int m_result;
};


/** Display help. */
class HelpCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit HelpCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
};


/** Overwrite timeout. */
class TimeoutCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit TimeoutCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
};


/** Quit application. */
class QuitCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit QuitCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
  virtual void connectResultSignal();
};


/** Change directory. */
class CdCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit CdCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
  virtual void connectResultSignal();
  virtual void disconnectResultSignal();
};


/** Print current working directory. */
class PwdCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit PwdCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
};


/** List directory. */
class LsCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit LsCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
};


/** Save changes. */
class SaveCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit SaveCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
};


/** Select file. */
class SelectCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit SelectCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
};


/** Display or set tag mask. */
class TagCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit TagCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
};


/** Get tag frame or file information. */
class GetCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit GetCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
};


/** Set tag frame. */
class SetCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit SetCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
};


/** Revert changes. */
class RevertCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit RevertCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
};


/** Import from file. */
class ImportCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit ImportCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
};

/** Automatic import from servers. */
class BatchImportCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit BatchImportCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
  virtual void connectResultSignal();
  virtual void disconnectResultSignal();

private slots:
  void onReportImportEvent(int type, const QString& text);
};

/** Download album cover art. */
class AlbumArtCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit AlbumArtCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
  virtual void connectResultSignal();
  virtual void disconnectResultSignal();

private slots:
  void onDownloadFinished(const QByteArray& data,
                          const QString& mimeType, const QString& url);
};

/** Export to file. */
class ExportCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit ExportCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
};

/** Create playlist file. */
class PlaylistCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit PlaylistCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
};

/** Apply file name format. */
class FilenameFormatCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit FilenameFormatCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
};

/** Apply tag format. */
class TagFormatCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit TagFormatCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
};

/** Apply text encoding. */
class TextEncodingCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit TextEncodingCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
};

/** Rename directory. */
class RenameDirectoryCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit RenameDirectoryCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
  virtual void connectResultSignal();
  virtual void disconnectResultSignal();

private slots:
  void onActionScheduled(const QStringList& actionStrs);
  void onRenameActionsScheduled();

private:
  bool m_dryRun;
};

/** Number tracks. */
class NumberTracksCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit NumberTracksCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
};

/** Filter files. */
class FilterCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit FilterCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
  virtual void connectResultSignal();
  virtual void disconnectResultSignal();

private slots:
  void onFileFiltered(int type, const QString& fileName);
};

/** Convert ID3v2.3 to ID3v2.4. */
class ToId3v24Command : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit ToId3v24Command(Kid3Cli* processor);

protected:
  virtual void startCommand();
};

/** Convert ID3v2.4 to ID3v2.3. */
class ToId3v23Command : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit ToId3v23Command(Kid3Cli* processor);

protected:
  virtual void startCommand();
};

/** Set file name from tags. */
class TagToFilenameCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit TagToFilenameCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
};

/** Set tags from file name. */
class FilenameToTagCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit FilenameToTagCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
};

/** Copy between tag 1 and tag 2. */
class TagToOtherTagCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit TagToOtherTagCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
};

/** Copy to clipboard. */
class CopyCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit CopyCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
};

/** Paste from clipboard. */
class PasteCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit PasteCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
};

/** Remove tags. */
class RemoveCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit RemoveCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
};

#if defined HAVE_PHONON || QT_VERSION >= 0x050000
/** Play audio file. */
class PlayCommand : public CliCommand {
  Q_OBJECT
public:
  /** Constructor. */
  explicit PlayCommand(Kid3Cli* processor);

protected:
  virtual void startCommand();
};
#endif

#endif // CLICOMMAND_H
