/**
 * \file kid3cli.h
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

#ifndef KID3CLI_H
#define KID3CLI_H

#include "abstractcli.h"
#include "taggedfile.h"
#include "trackdata.h"

class QTimer;
class ICorePlatformTools;
class Kid3Application;
class FileProxyModel;
class CliCommand;

/**
 * Command line interface for Kid3.
 */
class Kid3Cli : public AbstractCli {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param parent parent object
   */
  explicit Kid3Cli(QObject* parent = 0);

  /**
   * Destructor.
   */
  virtual ~Kid3Cli();

  /**
   * Execute process.
   */
  virtual void execute();

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
   * Select a file in the current directory.
   * @param fileName file name
   * @return true if file found and selected.
   */
  bool selectFile(const QString& fileName);

  /**
   * Get indexes of selected files.
   * @return selected indexes.
   */
  QList<QPersistentModelIndex> getSelection() const;

  /**
   * Display help about available commands.
   * @param cmdName command name, for all commands if empty
   */
  void writeHelp(const QString& cmdName = QString());

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
   * Get currently active tag mask.
   * @return tag bits.
   */
  TrackData::TagVersion tagMask() const { return m_tagMask; }

  /**
   * Set currently active tag mask.
   *
   * @param tagMask tag bits
   */
  void setTagMask(TrackData::TagVersion tagMask);

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

protected:
  /**
   * Process command line.
   * @param line command line
   */
  virtual void readLine(const QString& line);

private slots:
  /**
   * Select files passed as command line arguments after the intial directory has
   * been opened. Start execution of commands if existing.
   * @param dirIndex file proxy model index of opened directory
   * @param fileIndexes file proxy model indexes of selected files
   */
  void onInitialDirectoryOpened(const QPersistentModelIndex& dirIndex,
                               const QList<QPersistentModelIndex>& fileIndexes);

  /**
   * Called when a command is finished.
   */
  void onCommandFinished();

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

  void printFileProxyModel(const FileProxyModel* model,
                           const QModelIndex& parent, int indent);
  void writePrompt();
  bool parseOptions();
  void executeNextArgCommand();

  ICorePlatformTools* m_platformtools;
  Kid3Application* m_app;
  QList<CliCommand*> m_cmds;
  QStringList m_argCommands;
  TaggedFile::DetailInfo m_detailInfo;
  QString m_filename;
  QString m_tagFormatV1;
  QString m_tagFormatV2;
  TrackData::TagVersion m_tagMask;
  QList<QPersistentModelIndex> m_selection;
  bool m_fileNameChanged;
};

#endif // KID3CLI_H
