/**
 * \file editframefieldsdialog.h
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

#ifndef EDITFRAMEFIELDSDIALOG_H
#define EDITFRAMEFIELDSDIALOG_H

#include <qdialog.h>
#include <qlabel.h>
#include "frame.h"
#if QT_VERSION >= 0x040000
#include <QList>
#else
#include <qptrlist.h>
#endif

class TaggedFile;

/** Row of buttons to load, save and view binary data */
class BinaryOpenSave : public QWidget {
 Q_OBJECT

 public:
	/**
	 * Constructor.
	 *
	 * @param parent parent widget
	 * @param field  field containing binary data
	 */
	BinaryOpenSave(QWidget* parent, const Frame::Field& field);

	/**
	 * Set label.
	 *
	 * @param txt label
	 */
	void setLabel(const QString& txt) { m_label->setText(txt); }

	/**
	 * Check if data changed.
	 * @return true if data changed.
	 */
	bool isChanged() const { return m_isChanged; }

	/**
	 * Get binary data.
	 * @return byte array.
	 */
	const QByteArray& getData() const { return m_byteArray; }

	/**
	 * Set default directory name.
	 * @param defaultDir default directory name
	 */
	void setDefaultDir(const QString& defaultDir) { m_defaultDir = defaultDir; }

	/**
	 * Set default file name.
	 * @param defaultFile default file name
	 */
	void setDefaultFile(const QString& defaultFile) { m_defaultFile = defaultFile; }

 public slots:
	/**
	 * Enable the "From Clipboard" button if the clipboard contains an image.
	 */
	void setClipButtonState();

	/**
	 * Load image from clipboard.
	 */
	void clipData();

	/**
	 * Request name of file to import binary data from.
	 * The data is imported later when Ok is pressed in the parent dialog.
	 */
	void loadData();

	/**
	 * Request name of file and export binary data.
	 */
	void saveData();

	/**
	 * Create image from binary data and display it in window.
	 */
	void viewData();

 private:
	/** Array with binary data */
	QByteArray m_byteArray;
	/** true if m_byteArray changed */
	bool m_isChanged;
	/** Label left of buttons */
	QLabel* m_label;
	/** From Clipboard button */
	QPushButton* m_clipButton;
	/** Default directory name */
	QString m_defaultDir;
	/** Default file name */
	QString m_defaultFile;
};


/** Base class for field controls */
class FieldControl : public QObject {
public:
	/**
	 * Constructor.
	 */
	FieldControl() {}

	/**
	 * Destructor.
	 */
	virtual ~FieldControl() {}

	/**
	 * Update field from data in field control.
	 */
	virtual void updateTag() = 0;

	/**
	 * Create widget to edit field data.
	 *
	 * @param parent parent widget
	 *
	 * @return widget to edit field data.
	 */
	virtual QWidget* createWidget(QWidget* parent) = 0;
};


/** List of field control pointers. */
#if QT_VERSION >= 0x040000
typedef QList<FieldControl*> FieldControlList;
#else
typedef QPtrList<FieldControl> FieldControlList;
#endif

/** Field edit dialog */
class EditFrameFieldsDialog : public QDialog {
Q_OBJECT
public:
	/**
	 * Constructor.
	 *
	 * @param parent     parent widget
	 * @param caption    caption
	 * @param frame      frame with fields to edit
	 * @param taggedFile file
	 */
	EditFrameFieldsDialog(QWidget* parent, const QString& caption,
												const Frame& frame, const TaggedFile* taggedFile);

	/**
	 * Destructor.
	 */
	virtual ~EditFrameFieldsDialog();

	/**
	 * Update fields and get edited fields.
	 *
	 * @return field list.
	 */
	const Frame::FieldList& getUpdatedFieldList();

private:
	Frame::FieldList m_fields;
	FieldControlList m_fieldcontrols; 
};

#endif // EDITFRAMEFIELDSDIALOG_H
