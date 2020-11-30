/**
 * \file kid3cli.h
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

#pragma once

#include "abstractcli.h"
#include "frame.h"
#include "cliconfig.h"
#ifdef HAVE_READLINE
#include <QScopedPointer>
#endif

class QTimer;
class Kid3Application;
class FileProxyModel;
class CliCommand;
class AbstractCliFormatter;
enum class CliError : int;

#ifdef HAVE_READLINE
class Kid3CliCompleter;
#endif

/**
 * Command line interface for Kid3.
 */
class Kid3Cli : public AbstractCli {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param app application context
   * @param io I/O handler
   * @param args command line arguments
   * @param parent parent object
   */
  explicit Kid3Cli(Kid3Application* app,
                   AbstractCliIO* io, const QStringList& args,
                   QObject* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~Kid3Cli() override;

  /**
   * Execute process.
   */
  virtual void execute() override;

  /**
   * Write a line to standard error.
   * @param line line to write
   */
  virtual void writeErrorLine(const QString& line) override;

  /**
   * Write result of command.
   * @param str result as string
   */
  void writeResult(const QString& str);

  /**
   * Write result of command.
   * @param strs result as string list
   */
  void writeResult(const QStringList& strs);

  /**
   * Write result of command.
   * @param map result as map
   */
  void writeResult(const QVariantMap& map);

  /**
   * Write result of command.
   * @param map result as boolean
   */
  void writeResult(bool result);

  /**
   * Called when a command is finished.
   */
  void finishWriting();

  /**
   * Access to application.
   * @return application.
   */
  Kid3Application* app() const { return m_app; }

  /**
   * Open directory.
   * @param paths directory or file paths
   * @return true if ok.
   */
  bool openDirectory(const QStringList& paths);

  /**
   * Select files in the current directory.
   * @param paths file names
   * @return true if files found and selected.
   */
  bool selectFile(const QStringList &paths);

  /**
   * Display help about available commands.
   * @param cmdName command name, for all commands if empty
   * @param usageMessage true if this is a usage error message
   */
  void writeHelp(const QString& cmdName = QString(),
                 bool usageMessage = false);

  /**
   * Display information about selected files.
   * @param tagMask tag bits (1 for tag 1, 2 for tag 2)
   */
  void writeFileInformation(int tagMask);

  /**
   * Write currently active tag mask.
   */
  void writeTagMask();

  /**
   * List files.
   */
  void writeFileList();

  /**
   * Respond with an error message.
   * @param errorCode error code
   */
  void writeErrorCode(CliError errorCode);

  /**
   * Respond with an error message.
   * @param msg error message
   * @param errorCode error code
   */
  void writeError(const QString& msg, CliError errorCode);

  /**
   * Get currently active tag mask.
   * @return tag bits.
   */
  Frame::TagVersion tagMask() const { return m_tagMask; }

  /**
   * Set currently active tag mask.
   *
   * @param tagMask tag bits
   */
  void setTagMask(Frame::TagVersion tagMask);

  /**
   * Get timeout value.
   * @return timeout in ms, 0 to use defaults, -1 to switch off.
   */
  int getTimeout() const { return m_timeoutMs; }

  /**
   * Set timeout value.
   * @param timeout timeout in ms, 0 to use defaults, -1 to switch off
   */
  void setTimeout(int timeout) { m_timeoutMs = timeout; }

  /**
   * Expand wildcards in path list.
   * @param paths paths to expand
   * @return expanded paths.
   */
  static QStringList expandWildcards(const QStringList& paths);

public slots:
  /**
   * Update the currently selected files from the frame tables.
   */
  void updateSelectedFiles();

  /**
   * Has to be called when the selection changes to update the frame tables
   * and the information about the selected files.
   */
  void updateSelection();

  /**
   * Called when a command is finished.
   */
  void onCommandFinished();

protected:
  /**
   * Process command line.
   * @param line command line
   */
  virtual void readLine(const QString& line) override;

private slots:
  /**
   * Select files passed as command line arguments after the initial directory has
   * been opened. Start execution of commands if existing.
   */
  void onInitialDirectoryOpened();

  /**
   * Called when an argument command is finished.
   */
  void onArgCommandFinished();

private:
  /**
   * Get command for a command line.
   * @param line command line
   * @return command, 0 if no command found.
   */
  CliCommand* commandForArgs(const QString& line);

  QVariantList listFiles(const FileProxyModel* model,
                           const QModelIndex& parent);
  bool parseOptions();
  void executeNextArgCommand();

  Kid3Application* m_app;
  QStringList m_args;
#ifdef HAVE_READLINE
  QScopedPointer<Kid3CliCompleter> m_completer;
#endif
  AbstractCliFormatter* m_formatter;
  QList<AbstractCliFormatter*> m_formatters;
  QList<CliCommand*> m_cmds;
  QStringList m_argCommands;
  QString m_detailInfo;
  QString m_filename;
  QString m_tagFormat[Frame::Tag_NumValues];
  Frame::TagVersion m_tagMask;
  /** Overwrites command timeout, -1 to switch off, 0 for defaults, else ms. */
  int m_timeoutMs;
  bool m_fileNameChanged;
};
