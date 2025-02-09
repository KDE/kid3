/**
 * \file externalprocess.h
 * Handler for external process.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 22 Feb 2007
 *
 * Copyright (C) 2007-2024  Urs Fleisch
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

#include <QObject>
#include <QStringList>
#include <QVariantMap>
#include "kid3api.h"

class QProcess;
class Kid3Application;

/**
 * Handler for external process.
 */
class KID3_CORE_EXPORT ExternalProcess : public QObject {
  Q_OBJECT
public:
  /**
   * Interface for viewer to show output from external process.
   */
  class KID3_CORE_EXPORT IOutputViewer {
  public:
    /**
     * Destructor.
     */
    virtual ~IOutputViewer() = 0;

    /**
     * Set caption.
     * @param title caption
     */
    virtual void setCaption(const QString& title) = 0;

    /**
     * Append text.
     */
    virtual void append(const QString& text) = 0;

    /**
     * Scroll text to bottom.
     */
    virtual void scrollToBottom() = 0;
  };


  /**
   * Constructor.
   *
   * @param app application context
   * @param parent parent object
   */
  explicit ExternalProcess(Kid3Application* app, QObject* parent = nullptr);

  /**
   * Destructor.
   */
  ~ExternalProcess() override;

  /**
   * Get output viewer.
   * @return output viewer, default is null.
   */
  IOutputViewer* outputViewer() const { return m_outputViewer; }

  /**
   * Set output viewer.
   * @param viewer output viewer to be used
   */
  void setOutputViewer(IOutputViewer* viewer) { m_outputViewer = viewer; }

  /**
   * Launch a command.
   *
   * @param name       display name
   * @param args       command and arguments
   * @param showOutput true to show output of process
   * @param vars       variables to make available in command
   * @return false if process could not be executed.
   */
  bool launchCommand(const QString& name, const QStringList& args,
                     bool showOutput = false,
                     const QVariantMap& vars = QVariantMap());

signals:
  /**
   * Emitted when the process finishes.
   * @param exitCode exit code of process
   */
  void finished(int exitCode);

private slots:
  /**
   * Read data from standard output and display it in the output viewer.
   */
  void readFromStdout();

  /**
   * Show a line in the output viewer.
   * @param msg message to be displayed
   */
  void showOutputLine(const QString& msg);

private:
  Kid3Application* m_app;
  QProcess* m_process;
  IOutputViewer* m_outputViewer;
};
