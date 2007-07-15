/**
 * \file externalprocess.h
 * Handler for external process.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 22 Feb 2007
 */

#ifndef EXTERNALPROCESS_H
#define EXTERNALPROCESS_H

#include <qstring.h>
#include <qstringlist.h>
#include <qdialog.h>
#include <qtextedit.h>
#include "qtcompatmac.h"

class QProcess;
class TaggedFile;
class DirInfo;

/**
 * Handler for external process.
 */
class ExternalProcess : public QObject {
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
		OutputViewer(QWidget* parent);

		/**
		 * Destructor.
		 */
		virtual ~OutputViewer();

		/**
		 * Append text.
		 */
		void append(const QString& text) {
			m_textEdit->append(text);
		}

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
	 * @param parent parent object
	 */
	explicit ExternalProcess(QWidget* parent = 0);

	/**
	 * Destructor.
	 */
	virtual ~ExternalProcess();

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
	 * Read data from standard error and display it in the output viewer.
	 */
	void readFromStderr();

private:
	QWidget* m_parent;
	QProcess* m_process;
	OutputViewer* m_outputViewer;
};

#endif // EXTERNALPROCESS_H
