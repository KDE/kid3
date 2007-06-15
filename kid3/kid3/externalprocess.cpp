/**
 * \file externalprocess.cpp
 * Handler for external process.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 22 Feb 2007
 */

#include "externalprocess.h"
#include <qlayout.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#if QT_VERSION >= 0x040000
#include <Q3Process>
#else
#include <qprocess.h>
#endif
#include "qtcompatmac.h"
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
		QHBoxLayout* buttonLayout = new QHBoxLayout(vlayout);
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
		}
	}
	resize(586, 424);
}

/**
 * Destructor.
 */
ExternalProcess::OutputViewer::~OutputViewer() {}


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
		m_process = new Q3Process(m_parent);
	}
	if (m_process) {
		m_process->setArguments(args);

		if (showOutput) {
			if (!m_outputViewer) {
				m_outputViewer = new OutputViewer(0);
			}
			if (m_outputViewer) {
				connect(m_process, SIGNAL(readyReadStdout()),
								this, SLOT(readFromStdout()));
				connect(m_process, SIGNAL(readyReadStderr()),
								this, SLOT(readFromStderr()));
				m_outputViewer->setCaption(name);
				m_outputViewer->show();
				m_outputViewer->raise();
				m_outputViewer->scrollToBottom();
			}
		} else {
			disconnect(m_process, SIGNAL(readyReadStdout()),
								 this, SLOT(readFromStdout()));
			disconnect(m_process, SIGNAL(readyReadStderr()),
								 this, SLOT(readFromStderr()));
		}

		if (!m_process->launch(QString(""))) {
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
	m_outputViewer->append(m_process->readStdout());
}

/**
 * Read data from standard error and display it in the output viewer.
 */
void ExternalProcess::readFromStderr()
{
	m_outputViewer->append(m_process->readStderr());
}
