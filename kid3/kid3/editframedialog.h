/**
 * \file editframedialog.h
 * Field edit dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Jun 2009
 *
 * Copyright (C) 2003-2007  Urs Fleisch
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

#ifndef EDITFRAMEDIALOG_H
#define EDITFRAMEDIALOG_H

#include <qdialog.h>
#include <qtextedit.h>
#include "qtcompatmac.h"

class QPushButton;

/** Field edit dialog */
class EditFrameDialog : public QDialog {
Q_OBJECT
public:
 /**
	* Constructor.
	*
	* @param parent  parent widget
	* @param caption window title
	* @param text    text to edit
	*/
	EditFrameDialog(QWidget* parent, const QString& caption,
									const QString& text);

	/**
	 * Destructor.
	 */
	virtual ~EditFrameDialog();

	/**
	 * Set text to edit.
	 * @param text text
	 */
	void setText(const QString& text) {
		m_edit->QCM_setPlainText(text);
	}

	/**
	 * Get edited text.
	 * @return text.
	 */
	QString getText() const { return m_edit->QCM_toPlainText(); }

private:
	QTextEdit* m_edit;
	QPushButton* m_okButton;
	QPushButton* m_cancelButton;
};

#endif // EDITFRAMEDIALOG_H
