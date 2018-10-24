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
#include <QLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTextCursor>
#include <QTextEdit>
#include "taggedfile.h"
#include "iusercommandprocessor.h"
#include "kid3application.h"

/**
 * Constructor.
 *
 * @param parent parent widget
 */
ExternalProcess::OutputViewer::OutputViewer(QWidget* parent) : QDialog(parent)
{
  setObjectName(QLatin1String("OutputViewer"));
  setModal(false);
  auto vlayout = new QVBoxLayout(this);
  m_textEdit = new QTextEdit(this);
  m_textEdit->setReadOnly(true);
  m_textEdit->setLineWrapMode(QTextEdit::NoWrap);
  m_textEdit->setStyleSheet(QLatin1String("font-family: \"Courier\";"));
  vlayout->addWidget(m_textEdit);
  auto buttonLayout = new QHBoxLayout;
  QPushButton* clearButton = new QPushButton(tr("C&lear"), this);
  auto hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                         QSizePolicy::Minimum);
  QPushButton* closeButton = new QPushButton(tr("&Close"), this);
  buttonLayout->addWidget(clearButton);
  buttonLayout->addItem(hspacer);
  buttonLayout->addWidget(closeButton);
  connect(clearButton, &QAbstractButton::clicked, m_textEdit, &QTextEdit::clear);
  connect(closeButton, &QAbstractButton::clicked, this, &QDialog::accept);
  vlayout->addLayout(buttonLayout);
  resize(600, 424);
}

/**
 * Destructor.
 */
ExternalProcess::OutputViewer::~OutputViewer()
{
  // not inline or default to silence weak-vtables warning
}

/**
 * Append text.
 */
void ExternalProcess::OutputViewer::append(const QString& text)
{
  if (text.isEmpty())
    return;

  QString txt(text);
  txt.replace(QLatin1String("\r\n"), QLatin1String("\n"));
  int startPos = 0;
  int txtLen = txt.length();
  while (startPos < txtLen) {
    QChar ch;
    int len;
    int crLfPos = txt.indexOf(QRegExp(QLatin1String("[\\r\\n]")), startPos);
    if (crLfPos >= startPos) {
      ch = txt.at(crLfPos);
      len = crLfPos - startPos;
    } else {
      ch = QChar();
      len = -1;
    }
    QString line(txt.mid(startPos, len));
    if (!m_textEdit->textCursor().atBlockEnd()) {
      QTextCursor cursor = m_textEdit->textCursor();
      cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                          line.length());
      m_textEdit->setTextCursor(cursor);
    }
    m_textEdit->insertPlainText(line);
    if (ch == QLatin1Char('\r')) {
      m_textEdit->moveCursor(QTextCursor::StartOfLine);
    } else if (ch == QLatin1Char('\n')) {
      m_textEdit->moveCursor(QTextCursor::EndOfLine);
      m_textEdit->insertPlainText(ch);
    }
    if (len == -1) {
      break;
    }
    startPos = crLfPos + 1;
  }
}

/**
 * Scroll text to bottom.
 */
void ExternalProcess::OutputViewer::scrollToBottom()
{
  m_textEdit->moveCursor(QTextCursor::End);
}


/**
 * Constructor.
 *
 * @param app application context
 * @param parent parent object
 */
ExternalProcess::ExternalProcess(Kid3Application* app, QWidget* parent)
  : QObject(parent), m_app(app), m_parent(parent), m_process(nullptr)
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
  if (m_outputViewer) {
    m_outputViewer->close();
  }
}

/**
 * Launch a command.
 *
 * @param name       display name
 * @param args       command and arguments
 * @param confirm    true if confirmation required
 * @param showOutput true to show output of process
 */
void ExternalProcess::launchCommand(const QString& name, const QStringList& args,
                                    bool confirm, bool showOutput)
{
  if (args.isEmpty())
    return;

  if (confirm && QMessageBox::question(
        m_parent, name,
        tr("Execute ") + args.join(QLatin1String(" ")) + QLatin1Char('?'),
        QMessageBox::Ok, QMessageBox::Cancel) != QMessageBox::Ok) {
    return;
  }
  if (!m_process) {
    m_process = new QProcess(m_parent);
  }
  if (m_process->state() != QProcess::NotRunning) {
    m_process = new QProcess(m_parent);
  }

  if (showOutput) {
    if (!m_outputViewer) {
      m_outputViewer.reset(new OutputViewer(nullptr));
    }
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &ExternalProcess::readFromStdout);
    m_outputViewer->setWindowTitle(name);
    m_outputViewer->show();
    m_outputViewer->raise();
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
      if (userCommandProcessor->userCommandKeys().contains(program) &&
          userCommandProcessor->startUserCommand(program, arguments, showOutput))
        return;
    }
  }
  m_process->start(program, arguments);
  if (!m_process->waitForStarted(10000)) {
    QMessageBox::warning(
      m_parent, name,
      tr("Could not execute ") + args.join(QLatin1String(" ")),
      QMessageBox::Ok, Qt::NoButton);
  }
}

/**
 * Read data from standard output and display it in the output viewer.
 */
void ExternalProcess::readFromStdout()
{
  m_outputViewer->append(QString::fromLocal8Bit(m_process->readAllStandardOutput()));
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
