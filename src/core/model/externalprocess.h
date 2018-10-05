/**
 * \file externalprocess.h
 * Handler for external process.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 22 Feb 2007
 *
 * Copyright (C) 2007  Urs Fleisch
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

#ifndef EXTERNALPROCESS_H
#define EXTERNALPROCESS_H

#include <QDialog>
#include <QTextEdit>
#include <QScopedPointer>
#include "kid3api.h"

class QProcess;
class QString;
class QStringList;
class Kid3Application;
class TaggedFile;

/**
 * Handler for external process.
 */
class KID3_CORE_EXPORT ExternalProcess : public QObject {
  Q_OBJECT
public:
  /**
   * Dialog to show output from external process.
   */
  class OutputViewer : public QDialog {
  public:
    /**
     * Constructor.
     *
     * @param parent parent widget
     */
    explicit OutputViewer(QWidget* parent);

    /**
     * Destructor.
     */
    virtual ~OutputViewer() override;

    /**
     * Append text.
     */
    void append(const QString& text);

    /**
     * Scroll text to bottom.
     */
    void scrollToBottom();

  private:
    QTextEdit* m_textEdit;
  };


  /**
   * Constructor.
   *
   * @param app application context
   * @param parent parent object
   */
  explicit ExternalProcess(Kid3Application* app, QWidget* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~ExternalProcess() override;

  /**
   * Launch a command.
   *
   * @param name       display name
   * @param args       command and arguments
   * @param confirm    true if confirmation required
   * @param showOutput true to show output of process
   */
  void launchCommand(const QString& name, const QStringList& args,
                     bool confirm = false, bool showOutput = false);

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
  QWidget* m_parent;
  QProcess* m_process;
  QScopedPointer<OutputViewer> m_outputViewer;
};

#endif // EXTERNALPROCESS_H
