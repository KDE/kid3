/**
 * \file externalprocess.cpp
 * Handler for external process.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 22 Feb 2007
 *
 * Copyright (C) 2007-2018  Urs Fleisch
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

#include "externalprocess.h"
#include <QProcess>
#include <QString>
#include <QStringList>
#include "taggedfile.h"
#include "iusercommandprocessor.h"
#include "kid3application.h"

/**
 * Destructor.
 */
ExternalProcess::IOutputViewer::~IOutputViewer()
{
}


/**
 * Constructor.
 *
 * @param app application context
 * @param parent parent object
 */
ExternalProcess::ExternalProcess(Kid3Application* app, QObject* parent)
  : QObject(parent),
    m_app(app), m_process(nullptr), m_outputViewer(nullptr)
{
  setObjectName(QLatin1String("ExternalProcess"));
  const auto userCommandProcessors = m_app->getUserCommandProcessors();
  for (IUserCommandProcessor* userCommandProcessor : userCommandProcessors) {
    userCommandProcessor->initialize(m_app);
    connect(userCommandProcessor->qobject(), SIGNAL(commandOutput(QString)), // clazy:exclude=old-style-connect
            this, SLOT(showOutputLine(QString)));
  }
}

/**
 * Destructor.
 */
ExternalProcess::~ExternalProcess()
{
  const auto userCommandProcessors = m_app->getUserCommandProcessors();
  for (IUserCommandProcessor* userCommandProcessor : userCommandProcessors) {
    userCommandProcessor->cleanup();
  }
}

/**
 * Launch a command.
 *
 * @param name       display name
 * @param args       command and arguments
 * @param showOutput true to show output of process
 * @param vars       variables to make available in command
 * @return false if process could not be executed.
 */
bool ExternalProcess::launchCommand(const QString& name, const QStringList& args,
                                    bool showOutput, const QVariantMap& vars)
{
  if (args.isEmpty())
    return true;

  if (!m_process) {
    m_process = new QProcess(parent());
  }
  if (m_process->state() != QProcess::NotRunning) {
    m_process = new QProcess(parent());
  }
  connect(m_process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
            &QProcess::finished),
          this, &ExternalProcess::finished, Qt::UniqueConnection);

  if (showOutput && m_outputViewer) {
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &ExternalProcess::readFromStdout);
    m_outputViewer->setCaption(name);
    m_outputViewer->scrollToBottom();
  } else {
    disconnect(m_process, &QProcess::readyReadStandardOutput,
               this, &ExternalProcess::readFromStdout);
  }

  QStringList arguments = args;
  QString program = arguments.takeFirst();
  if (program.startsWith(QLatin1Char('@'))) {
    program = program.mid(1);
    const auto userCommandProcessors = m_app->getUserCommandProcessors();
    for (IUserCommandProcessor* userCommandProcessor : userCommandProcessors) {
      if (userCommandProcessor->userCommandKeys().contains(program)) {
        connect(userCommandProcessor->qobject(), SIGNAL(finished(int)),
                this, SIGNAL(finished(int)), Qt::UniqueConnection);
        if (userCommandProcessor->startUserCommand(program, arguments,
                                                   showOutput, vars)) {
          return true;
        }
      }
    }
  }
  m_process->start(program, arguments);
  if (!m_process->waitForStarted(10000)) {
    return false;
  }
  return true;
}

/**
 * Read data from standard output and display it in the output viewer.
 */
void ExternalProcess::readFromStdout()
{
  if (m_outputViewer) {
    m_outputViewer->append(QString::fromLocal8Bit(
                             m_process->readAllStandardOutput()));
  }
}

/**
 * Show a line in the output viewer.
 * @param msg message to be displayed
 */
void ExternalProcess::showOutputLine(const QString& msg)
{
  if (m_outputViewer) {
    m_outputViewer->append(msg + QLatin1Char('\n'));
  }
}
