/**
 * \file externalprocess.cpp
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

#include "externalprocess.h"
#include <qlayout.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qprocess.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QTextCursor>
#endif
#include "taggedfile.h"
#include "dirinfo.h"

/**
 * Constructor.
 *
 * @param parent parent widget
 */
ExternalProcess::OutputViewer::OutputViewer(QWidget* parent) : QDialog(parent)
{
	setModal(false);
	QVBoxLayout* vlayout = new QVBoxLayout(this);
	m_textEdit = new QTextEdit(this);
	if (vlayout && m_textEdit) {
		vlayout->setSpacing(6);
		vlayout->setMargin(6);
		m_textEdit->setReadOnly(true);
		vlayout->addWidget(m_textEdit);
		QHBoxLayout* buttonLayout = new QHBoxLayout;
		QPushButton* clearButton = new QPushButton(i18n("C&lear"), this);
		QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
																					 QSizePolicy::Minimum);
		QPushButton* closeButton = new QPushButton(i18n("&Close"), this);
		if (buttonLayout && clearButton && hspacer && closeButton) {
			buttonLayout->addWidget(clearButton);
			buttonLayout->addItem(hspacer);
			buttonLayout->addWidget(closeButton);
			connect(clearButton, SIGNAL(clicked()), m_textEdit, SLOT(clear()));
			connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));
			vlayout->addLayout(buttonLayout);
		}
	}
	resize(586, 424);
}

/**
 * Destructor.
 */
ExternalProcess::OutputViewer::~OutputViewer() {}

/**
 * Scroll text to bottom.
 */
void ExternalProcess::OutputViewer::scrollToBottom()
{
#if QT_VERSION >= 0x040200
	m_textEdit->moveCursor(QTextCursor::End);
#elif QT_VERSION >= 0x040000
	QTextCursor cursor = m_textEdit->textCursor();
	cursor.movePosition(QTextCursor::End);
	m_textEdit->setTextCursor(cursor);
#else
	m_textEdit->scrollToBottom();
#endif
}


/**
 * Constructor.
 *
 * @param parent parent object
 */
ExternalProcess::ExternalProcess(QWidget* parent) :
	QObject(parent), m_parent(parent), m_process(0), m_outputViewer(0)
{
}

/**
 * Destructor.
 */
ExternalProcess::~ExternalProcess()
{
	delete m_outputViewer;
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
	if (confirm &&
#if QT_VERSION >= 0x030100
			QMessageBox::question
#else
			QMessageBox::warning
#endif
			(
				m_parent, name,
				i18n("Execute ") + args.join(" ") + "?",
				QMessageBox::Ok, QMessageBox::Cancel) != QMessageBox::Ok) {
		return;
	}
	if (!m_process) {
		m_process = new QProcess(m_parent);
	}
	if (m_process) {
#if QT_VERSION >= 0x040000
		if (m_process->state() != QProcess::NotRunning) {
			m_process = new QProcess(m_parent);
		}
#else
		m_process->setArguments(args);
#endif

		if (showOutput) {
			if (!m_outputViewer) {
				m_outputViewer = new OutputViewer(0);
			}
			if (m_outputViewer) {
				connect(m_process, QCM_SIGNAL_readyReadStandardOutput,
								this, SLOT(readFromStdout()));
				connect(m_process, QCM_SIGNAL_readyReadStandardError,
								this, SLOT(readFromStderr()));
				m_outputViewer->QCM_setWindowTitle(name);
				m_outputViewer->show();
				m_outputViewer->raise();
				m_outputViewer->scrollToBottom();
			}
		} else {
			disconnect(m_process, QCM_SIGNAL_readyReadStandardOutput,
								 this, SLOT(readFromStdout()));
			disconnect(m_process, QCM_SIGNAL_readyReadStandardError,
								 this, SLOT(readFromStderr()));
		}

#if QT_VERSION >= 0x040000
		QStringList arguments = args;
		QString program = arguments.takeFirst();
		m_process->start(program, arguments);
		if (!m_process->waitForStarted(10000))
#else
		if (!m_process->launch(QString("")))
#endif
		{
			QMessageBox::warning(
				m_parent, name,
				i18n("Could not execute ") + args.join(" "),
				QMessageBox::Ok, QCM_NoButton);
		}
	}
}

/**
 * Read data from standard output and display it in the output viewer.
 */
void ExternalProcess::readFromStdout()
{
	m_outputViewer->append(m_process->QCM_readAllStandardOutput());
}

/**
 * Read data from standard error and display it in the output viewer.
 */
void ExternalProcess::readFromStderr()
{
	m_outputViewer->append(m_process->QCM_readAllStandardError());
}
