/**
 * \file editframefieldsdialog.cpp
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

#include "editframefieldsdialog.h"
#include "kid3.h"
#include "imageviewer.h"
#include "taggedfile.h"
#include "dirinfo.h"
#include <qpushbutton.h>
#include <qimage.h>
#include <qclipboard.h>
#include <qtextedit.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qapplication.h>
#include <qfile.h>
#include <qbuffer.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QVBoxLayout>
#include <QMimeData>
#else
#include <qlayout.h>
#endif
#ifdef CONFIG_USE_KDE
#include <kfiledialog.h>
#else
#include <qfiledialog.h>
#endif

/** QTextEdit with label above */
class LabeledTextEdit : public QWidget {
public:
	/**
	 * Constructor.
	 *
	 * @param parent parent widget
	 */
	LabeledTextEdit(QWidget* parent);

	/**
	 * Get text.
	 *
	 * @return text.
	 */
	QString text() const {
		return m_edit->QCM_toPlainText();
	}

	/**
	 * Set text.
	 *
	 * @param txt text
	 */
	void setText(const QString& txt) {
		m_edit->QCM_setPlainText(txt);
	}

	/**
	 * Set label.
	 *
	 * @param txt label
	 */
	void setLabel(const QString& txt) { m_label->setText(txt); }

private:
	/** Label above edit */
	QLabel* m_label;
	/** Text editor */
	QTextEdit* m_edit;
};


/** LineEdit with label above */
class LabeledLineEdit : public QWidget {
public:
	/**
	 * Constructor.
	 *
	 * @param parent parent widget
	 */
	LabeledLineEdit(QWidget* parent);

	/**
	 * Get text.
	 *
	 * @return text.
	 */
	QString text() const { return m_edit->text(); }

	/**
	 * Set text.
	 *
	 * @param txt text
	 */
	void setText(const QString& txt) { m_edit->setText(txt); }

	/**
	 * Set label.
	 *
	 * @param txt label
	 */
	void setLabel(const QString& txt) { m_label->setText(txt); }

private:
	/** Label above edit */
	QLabel* m_label;
	/** Line editor */
	QLineEdit* m_edit;
};


/** Combo box with label above */
class LabeledComboBox : public QWidget {
public:
	/**
	 * Constructor.
	 *
	 * @param parent parent widget
	 * @param strlst list with ComboBox items, terminated by NULL
	 */
	LabeledComboBox(QWidget* parent, const char** strlst);

	/**
	 * Get index of selected item.
	 *
	 * @return index.
	 */
	int currentItem() const {
		return m_combo->QCM_currentIndex();
	}

	/**
	 * Set index of selected item.
	 *
	 * @param idx index
	 */
	void setCurrentItem(int idx) {
		m_combo->QCM_setCurrentIndex(idx);
	}

	/**
	 * Set label.
	 *
	 * @param txt label
	 */
	void setLabel(const QString& txt) { m_label->setText(txt); }

private:
	/** Label above combo box */
	QLabel* m_label;
	/** Combo box */
	QComboBox* m_combo;
};


/** QSpinBox with label above */
class LabeledSpinBox : public QWidget {
public:
	/**
	 * Constructor.
	 *
	 * @param parent parent widget
	 */
	LabeledSpinBox(QWidget* parent);

	/**
	 * Get value.
	 *
	 * @return text.
	 */
	int value() const { return m_spinbox->value(); }

	/**
	 * Set value.
	 *
	 * @param value value
	 */
	void setValue(int value) { m_spinbox->setValue(value); }

	/**
	 * Set label.
	 *
	 * @param txt label
	 */
	void setLabel(const QString& txt) { m_label->setText(txt); }

private:
	/** Label above edit */
	QLabel* m_label;
	/** Text editor */
	QSpinBox* m_spinbox;
};


/**
 * Constructor.
 *
 * @param parent parent widget
 */
LabeledTextEdit::LabeledTextEdit(QWidget* parent) :
	QWidget(parent)
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	m_label = new QLabel(this);
	m_edit = new QTextEdit(this);
	if (layout && m_label && m_edit) {
		layout->setMargin(0);
		layout->setSpacing(2);
		m_edit->QCM_setTextFormat_PlainText();
		layout->addWidget(m_label);
		layout->addWidget(m_edit);
	}
}

/**
 * Constructor.
 *
 * @param parent parent widget
 */
LabeledLineEdit::LabeledLineEdit(QWidget* parent) :
	QWidget(parent)
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	m_label = new QLabel(this);
	m_edit = new QLineEdit(this);
	if (layout && m_label && m_edit) {
		layout->setMargin(0);
		layout->setSpacing(2);
		layout->addWidget(m_label);
		layout->addWidget(m_edit);
	}
}

/**
 * Constructor.
 *
 * @param parent parent widget
 * @param strlst list with ComboBox items, terminated by NULL
 */
LabeledComboBox::LabeledComboBox(QWidget* parent,
				 const char **strlst) : QWidget(parent)
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	m_label = new QLabel(this);
	m_combo = new QComboBox(this);
	if (layout && m_label && m_combo) {
		layout->setMargin(0);
		layout->setSpacing(2);
		QStringList strList;
		while (*strlst) {
			strList += QCM_translate(*strlst++);
		}
		m_combo->QCM_addItems(strList);
		layout->addWidget(m_label);
		layout->addWidget(m_combo);
	}
}

/**
 * Constructor.
 *
 * @param parent parent widget
 */
LabeledSpinBox::LabeledSpinBox(QWidget* parent) :
	QWidget(parent)
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	m_label = new QLabel(this);
	m_spinbox = new QSpinBox(this);
	if (layout && m_label && m_spinbox) {
		m_spinbox->setRange(0, INT_MAX);
		layout->setMargin(0);
		layout->setSpacing(2);
		layout->addWidget(m_label);
		layout->addWidget(m_spinbox);
	}
}


/** Base class for MP3 field controls */
class Mp3FieldControl : public FieldControl {
public:
	/**
	 * Constructor.
	 * @param field field to edit
	 */
	Mp3FieldControl(Frame::Field& field) :
		m_field(field) {}

	/**
	 * Destructor.
	 */
	virtual ~Mp3FieldControl() {}

protected:
	/**
	 * Get description for ID3_Field.
	 *
	 * @param id ID of field
	 * @return description or NULL if id unknown.
	 */
	const char* getFieldIDString(Frame::Field::Id id) const;

	/** field */
	Frame::Field& m_field;
};

/** Control to edit standard UTF text fields */
class TextFieldControl : public Mp3FieldControl {
public:
	/**
	 * Constructor.
	 * @param field field to edit
	 */
	TextFieldControl(Frame::Field& field) :
		Mp3FieldControl(field) {}

	/**
	 * Destructor.
	 */
	virtual ~TextFieldControl() {}

	/**
	 * Update field from data in field control.
	 */
	virtual void updateTag();

	/**
	 * Create widget to edit field data.
	 *
	 * @param parent parent widget
	 *
	 * @return widget to edit field data.
	 */
	virtual QWidget* createWidget(QWidget* parent);

protected:
	/** Text editor widget */
	LabeledTextEdit* m_edit;
};

/** Control to edit single line text fields */
class LineFieldControl : public Mp3FieldControl {
public:
	/**
	 * Constructor.
	 * @param field field to edit
	 */
	LineFieldControl(Frame::Field& field) :
		Mp3FieldControl(field) {}

	/**
	 * Destructor.
	 */
	virtual ~LineFieldControl() {}

	/**
	 * Update field from data in field control.
	 */
	virtual void updateTag();

	/**
	 * Create widget to edit field data.
	 *
	 * @param parent parent widget
	 *
	 * @return widget to edit field data.
	 */
	virtual QWidget* createWidget(QWidget* parent);

protected:
	/** Line editor widget */
	LabeledLineEdit* m_edit;
};

/** Control to edit integer fields */
class IntFieldControl : public Mp3FieldControl {
public:
	/**
	 * Constructor.
	 * @param field field to edit
	 */
	IntFieldControl(Frame::Field& field) :
		Mp3FieldControl(field) {}

	/**
	 * Destructor.
	 */
	virtual ~IntFieldControl() {}

	/**
	 * Update field from data in field control.
	 */
	virtual void updateTag();

	/**
	 * Create widget to edit field data.
	 *
	 * @param parent parent widget
	 *
	 * @return widget to edit field data.
	 */
	virtual QWidget* createWidget(QWidget* parent);

protected:
	/** Spin box widget */
	LabeledSpinBox* m_numInp;
};

/** Control to edit integer fields using a combo box with given values */
class IntComboBoxControl : public Mp3FieldControl {
public:
	/**
	 * Constructor.
	 * @param field field to edit
	 * @param lst list of strings with possible selections, NULL terminated
	 */
	IntComboBoxControl(Frame::Field& field,
										 const char **lst) :
		Mp3FieldControl(field), m_strLst(lst) {}

	/**
	 * Destructor.
	 */
	virtual ~IntComboBoxControl() {}

	/**
	 * Update field from data in field control.
	 */
	virtual void updateTag();

	/**
	 * Create widget to edit field data.
	 *
	 * @param parent parent widget
	 *
	 * @return widget to edit field data.
	 */
	virtual QWidget* createWidget(QWidget* parent);

protected:
	/** Combo box widget */
	LabeledComboBox* m_ptInp;
	/** List of strings with possible selections */
	const char** m_strLst;
};

/** Control to import, export and view data from binary fields */
class BinFieldControl : public Mp3FieldControl {
public:
	/**
	 * Constructor.
	 * @param field      field to edit
	 * @param frame      frame with fields to edit
	 * @param taggedFile file
	 */
	BinFieldControl(Frame::Field& field,
									const Frame& frame, const TaggedFile* taggedFile) :
		Mp3FieldControl(field), m_frame(frame), m_taggedFile(taggedFile) {}

	/**
	 * Destructor.
	 */
	virtual ~BinFieldControl() {}

	/**
	 * Update field from data in field control.
	 */
	virtual void updateTag();

	/**
	 * Create widget to edit field data.
	 *
	 * @param parent parent widget
	 *
	 * @return widget to edit field data.
	 */
	virtual QWidget* createWidget(QWidget* parent);

protected:
	/** Import, Export, View buttons */
	BinaryOpenSave* m_bos;
	/** frame with fields to edit */
	const Frame& m_frame;
	/** tagged file */
	const TaggedFile* m_taggedFile;
};


/**
 * Constructor.
 *
 * @param parent parent widget
 * @param field  field containing binary data
 */
BinaryOpenSave::BinaryOpenSave(QWidget* parent, const Frame::Field& field) :
	QWidget(parent), m_byteArray(field.m_value.toByteArray()),
	m_isChanged(false)
{
	QHBoxLayout* layout = new QHBoxLayout(this);
	m_label = new QLabel(this);
	m_clipButton = new QPushButton(i18n("From Clip&board"), this);
	QPushButton* openButton = new QPushButton(i18n("&Import"), this);
	QPushButton* saveButton = new QPushButton(i18n("&Export"), this);
	QPushButton* viewButton = new QPushButton(i18n("&View"), this);
	if (layout && m_label && m_clipButton && openButton && saveButton && viewButton) {
		layout->setMargin(0);
		layout->setSpacing(6);
		layout->addWidget(m_label);
		layout->addWidget(m_clipButton);
		layout->addWidget(openButton);
		layout->addWidget(saveButton);
		layout->addWidget(viewButton);
		connect(m_clipButton, SIGNAL(clicked()), this, SLOT(clipData()));
		connect(openButton, SIGNAL(clicked()), this, SLOT(loadData()));
		connect(saveButton, SIGNAL(clicked()), this, SLOT(saveData()));
		connect(viewButton, SIGNAL(clicked()), this, SLOT(viewData()));
		connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(setClipButtonState()));
		setClipButtonState();
	}
}

/**
 * Enable the "From Clipboard" button if the clipboard contains an image.
 */
void BinaryOpenSave::setClipButtonState()
{
	QClipboard* cb = QApplication::clipboard();
	m_clipButton->setEnabled(
#if QT_VERSION >= 0x040000
		cb && (cb->mimeData()->hasFormat("image/jpeg") ||
					 cb->mimeData()->hasImage())
#else
		cb && cb->data(QClipboard::Clipboard)->provides("image/jpeg")
#endif
		);
}

/**
 * Load image from clipboard.
 */
void BinaryOpenSave::clipData()
{
	QClipboard* cb = QApplication::clipboard();
#if QT_VERSION >= 0x040000
	if (cb) {
		if (cb->mimeData()->hasFormat("image/jpeg")) {
			m_byteArray = cb->mimeData()->data("image/jpeg");
			m_isChanged = true;
		} else if (cb->mimeData()->hasImage()) {
			QBuffer buffer(&m_byteArray);
			buffer.open(QIODevice::WriteOnly);
			cb->image().save(&buffer, "JPG");
			m_isChanged = true;
		}
	}
#else
	if (cb && cb->data(QClipboard::Clipboard)->provides("image/jpeg")) {
		m_byteArray = cb->data(QClipboard::Clipboard)->encodedData("image/jpeg");
		m_isChanged = true;
	}
#endif
}

/**
 * Request name of file to import binary data from.
 * The data is imported later when Ok is pressed in the parent dialog.
 */
void BinaryOpenSave::loadData()
{
#ifdef CONFIG_USE_KDE
	QString loadfilename = KFileDialog::getOpenFileName(
		m_defaultDir.isEmpty() ? Kid3App::getDirName() : m_defaultDir,
		QString::null, this);
#else
	QString loadfilename = QFileDialog::QCM_getOpenFileName(
		this, m_defaultDir.isEmpty() ? Kid3App::getDirName() : m_defaultDir);
#endif
	if (!loadfilename.isEmpty()) {
		QFile file(loadfilename);
		if (file.open(QCM_ReadOnly)) {
			size_t size = file.size();
			char* data = new char[size];
			if (data) {
				QDataStream stream(&file);
				stream.QCM_readRawData(data, size);
				QCM_duplicate(m_byteArray, data, size);
				m_isChanged = true;
				delete [] data;
			}
			file.close();
		}
	}
}

/**
 * Request name of file and export binary data.
 */
void BinaryOpenSave::saveData()
{
	QString dir = m_defaultDir.isEmpty() ? Kid3App::getDirName() : m_defaultDir;
	if (!m_defaultFile.isEmpty()) {
		QChar separator = QDir::separator();
		if (!dir.endsWith(separator)) {
			dir += separator;
		}
		dir += m_defaultFile;
	}
#ifdef CONFIG_USE_KDE
	QString fn = KFileDialog::getSaveFileName(dir, QString::null, this);
#else
	QString fn = QFileDialog::QCM_getSaveFileName(this, dir);
#endif
	if (!fn.isEmpty()) {
		QFile file(fn);
		if (file.open(QCM_WriteOnly)) {
			QDataStream stream(&file);
			stream.QCM_writeRawData(m_byteArray.data(), m_byteArray.size());
			file.close();
		}
	}
}

/**
 * Create image from binary data and display it in window.
 */
void BinaryOpenSave::viewData()
{
	QImage image;
	if (image.loadFromData(m_byteArray)) {
		ImageViewer iv(this, image);
		iv.exec();
	}
}

/**
 * Get description for ID3_Field.
 *
 * @param id ID of field
 * @return description or NULL if id unknown.
 */
const char* Mp3FieldControl::getFieldIDString(Frame::Field::Id id) const
{
	static const char* const idStr[] = {
		"Unknown",
		I18N_NOOP("Text Encoding"),
		I18N_NOOP("Text"),
		I18N_NOOP("URL"),
		I18N_NOOP("Data"),
		I18N_NOOP("Description"),
		I18N_NOOP("Owner"),
		I18N_NOOP("Email"),
		I18N_NOOP("Rating"),
		I18N_NOOP("Filename"),
		I18N_NOOP("Language"),
		I18N_NOOP("Picture Type"),
		I18N_NOOP("Image format"),
		I18N_NOOP("Mimetype"),
		I18N_NOOP("Counter"),
		I18N_NOOP("Identifier"),
		I18N_NOOP("Volume Adjustment"),
		I18N_NOOP("Number of Bits"),
		I18N_NOOP("Volume Change Right"),
		I18N_NOOP("Volume Change Left"),
		I18N_NOOP("Peak Volume Right"),
		I18N_NOOP("Peak Volume Left"),
		I18N_NOOP("Timestamp Format"),
		I18N_NOOP("Content Type")
	};
	class not_used { int array_size_check[
			sizeof(idStr) / sizeof(idStr[0]) == Frame::Field::ID_ContentType + 1
			? 1 : -1 ]; };
	return idStr[id <= Frame::Field::ID_ContentType ? id : 0];
}

/**
 * Update field with data from dialog.
 */
void TextFieldControl::updateTag()
{
	m_field.m_value = m_edit->text();
}

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* TextFieldControl::createWidget(QWidget* parent)
{
	m_edit = new LabeledTextEdit(parent);
	if (m_edit == NULL)
		return NULL;

	m_edit->setLabel(QCM_translate(getFieldIDString(static_cast<Frame::Field::Id>(m_field.m_id))));
	m_edit->setText(m_field.m_value.toString());
	return m_edit;
}

/**
 * Update field with data from dialog.
 */
void LineFieldControl::updateTag()
{
	m_field.m_value = m_edit->text();
}

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* LineFieldControl::createWidget(QWidget* parent)
{
	m_edit = new LabeledLineEdit(parent);
	if (m_edit) {
		m_edit->setLabel(QCM_translate(getFieldIDString(static_cast<Frame::Field::Id>(m_field.m_id))));
		m_edit->setText(m_field.m_value.toString());
	}
	return m_edit;
}

/**
 * Update field with data from dialog.
 */
void IntFieldControl::updateTag()
{
	m_field.m_value = m_numInp->value();
}

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* IntFieldControl::createWidget(QWidget* parent)
{
	m_numInp = new LabeledSpinBox(parent);
	if (m_numInp) {
		m_numInp->setLabel(QCM_translate(getFieldIDString(static_cast<Frame::Field::Id>(m_field.m_id))));
		m_numInp->setValue(m_field.m_value.toInt());
	}
	return m_numInp;
}

/**
 * Update field with data from dialog.
 */
void IntComboBoxControl::updateTag()
{
	m_field.m_value = m_ptInp->currentItem();
}

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* IntComboBoxControl::createWidget(QWidget* parent)
{
	m_ptInp = new LabeledComboBox(parent, m_strLst);
	if (m_ptInp) {
		m_ptInp->setLabel(QCM_translate(getFieldIDString(static_cast<Frame::Field::Id>(m_field.m_id))));
		m_ptInp->setCurrentItem(m_field.m_value.toInt());
	}
	return m_ptInp;
}

/**
 * Update field with data from dialog.
 */
void BinFieldControl::updateTag()
{
	if (m_bos && m_bos->isChanged()) {
		m_field.m_value = m_bos->getData();
	}
}

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* BinFieldControl::createWidget(QWidget* parent)
{
	m_bos = new BinaryOpenSave(parent, m_field);
	if (m_bos) {
		m_bos->setLabel(QCM_translate(getFieldIDString(static_cast<Frame::Field::Id>(m_field.m_id))));
		if (m_taggedFile && m_taggedFile->getDirInfo()) {
			m_bos->setDefaultDir(m_taggedFile->getDirInfo()->getDirname());
		}
		if (m_frame.getType() == Frame::FT_Picture) {
			m_bos->setDefaultFile("folder.jpg");
		}
	}
	return m_bos;
}


/**
 * Update fields and get edited fields.
 *
 * @return field list.
 */
const Frame::FieldList& EditFrameFieldsDialog::getUpdatedFieldList()
{
#if QT_VERSION >= 0x040000
	QListIterator<FieldControl*> it(m_fieldcontrols);
	while (it.hasNext()) {
		it.next()->updateTag();
	}
#else
	FieldControl* fldCtl = m_fieldcontrols.first();
	while (fldCtl != NULL) {
		fldCtl->updateTag();
		fldCtl = m_fieldcontrols.next();
	}
#endif
	return m_fields;
}


/**
 * Constructor.
 *
 * @param parent     parent widget
 * @param caption    caption
 * @param frame      frame with fields to edit
 * @param taggedFile file
 */
EditFrameFieldsDialog::EditFrameFieldsDialog(
	QWidget* parent, const QString& caption,
	const Frame& frame, const TaggedFile* taggedFile) :
	QDialog(parent), m_fields(frame.getFieldList())
{
	setModal(true);
	QCM_setWindowTitle(caption);
#if QT_VERSION >= 0x040000
	qDeleteAll(m_fieldcontrols);
#else
	m_fieldcontrols.setAutoDelete(true);
#endif
	m_fieldcontrols.clear();
	QVBoxLayout* vlayout = new QVBoxLayout(this);
	if (vlayout) {
		vlayout->setSpacing(6);
		vlayout->setMargin(6);

		for (Frame::FieldList::iterator fldIt = m_fields.begin();
				 fldIt != m_fields.end();
				 ++fldIt) {
			Frame::Field& fld = *fldIt;
			switch (fld.m_value.type()) {
				case QVariant::Int:
				case QVariant::UInt:
					if (fld.m_id == Frame::Field::ID_TextEnc) {
						static const char* strlst[] = {
							I18N_NOOP("ISO-8859-1"),
							I18N_NOOP("UTF16"),
							I18N_NOOP("UTF16BE"),
							I18N_NOOP("UTF8"),
							NULL
						};
						IntComboBoxControl* cbox =
							new IntComboBoxControl(fld, strlst);
						if (cbox) {
							m_fieldcontrols.append(cbox);
						}
					}
					else if (fld.m_id == Frame::Field::ID_PictureType) {
						static const char* strlst[] = {
							I18N_NOOP("Other"),
							I18N_NOOP("32x32 pixels PNG file icon"),
							I18N_NOOP("Other file icon"),
							I18N_NOOP("Cover (front)"),
							I18N_NOOP("Cover (back)"),
							I18N_NOOP("Leaflet page"),
							I18N_NOOP("Media"),
							I18N_NOOP("Lead artist/lead performer/soloist"),
							I18N_NOOP("Artist/performer"),
							I18N_NOOP("Conductor"),
							I18N_NOOP("Band/Orchestra"),
							I18N_NOOP("Composer"),
							I18N_NOOP("Lyricist/text writer"),
							I18N_NOOP("Recording Location"),
							I18N_NOOP("During recording"),
							I18N_NOOP("During performance"),
							I18N_NOOP("Movie/video screen capture"),
							I18N_NOOP("A bright coloured fish"),
							I18N_NOOP("Illustration"),
							I18N_NOOP("Band/artist logotype"),
							I18N_NOOP("Publisher/Studio logotype"),
							NULL
						};
						IntComboBoxControl* cbox =
							new IntComboBoxControl(fld, strlst);
						if (cbox) {
							m_fieldcontrols.append(cbox);
						}
					}
					else if (fld.m_id == Frame::Field::ID_TimestampFormat) {
						static const char* strlst[] = {
							I18N_NOOP("Other"),
							I18N_NOOP("MPEG frames as unit"),
							I18N_NOOP("Milliseconds as unit"),
							NULL
						};
						IntComboBoxControl* cbox =
							new IntComboBoxControl(fld, strlst);
						if (cbox) {
							m_fieldcontrols.append(cbox);
						}
					}
					else if (fld.m_id == Frame::Field::ID_ContentType) {
						static const char* strlst[] = {
							I18N_NOOP("Other"),
							I18N_NOOP("Lyrics"),
							I18N_NOOP("Text transcription"),
							I18N_NOOP("Movement/part name"),
							I18N_NOOP("Events"),
							I18N_NOOP("Chord"),
							I18N_NOOP("Trivia/pop up"),
							NULL
						};
						IntComboBoxControl* cbox =
							new IntComboBoxControl(fld, strlst);
						if (cbox) {
							m_fieldcontrols.append(cbox);
						}
					}
					else {
						IntFieldControl* intctl =
							new IntFieldControl(fld);
						if (intctl) {
							m_fieldcontrols.append(intctl);
						}
					}
					break;

				case QVariant::String:
					if (fld.m_id == Frame::Field::ID_Text) {
						// Large textedit for text fields
						TextFieldControl* textctl =
							new TextFieldControl(fld);
						if (textctl) {
							m_fieldcontrols.append(textctl);
						}
					}
					else {
						LineFieldControl* textctl =
							new LineFieldControl(fld);
						if (textctl) {
							m_fieldcontrols.append(textctl);
						}
					}
					break;

				case QVariant::ByteArray:
				{
					BinFieldControl* binctl =
						new BinFieldControl(fld, frame, taggedFile);
					if (binctl) {
						m_fieldcontrols.append(binctl);
					}
					break;
				}

				default:
					qDebug("Unknown type %d in field %d", fld.m_value.type(), fld.m_id);
			}
		}

#if QT_VERSION >= 0x040000
		QListIterator<FieldControl*> it(m_fieldcontrols);
		while (it.hasNext()) {
			vlayout->addWidget(it.next()->createWidget(this));
		}
#else
		FieldControl* fldCtl = m_fieldcontrols.first();
		while (fldCtl != NULL) {
			vlayout->addWidget(fldCtl->createWidget(this));
			fldCtl = m_fieldcontrols.next();
		}
#endif
	}
	QHBoxLayout* hlayout = new QHBoxLayout;
	QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
					   QSizePolicy::Minimum);
	QPushButton* okButton = new QPushButton(i18n("&OK"), this);
	QPushButton* cancelButton = new QPushButton(i18n("&Cancel"), this);
	if (hlayout && okButton && cancelButton) {
		hlayout->addItem(hspacer);
		hlayout->addWidget(okButton);
		hlayout->addWidget(cancelButton);
		okButton->setDefault(true);
		connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
		vlayout->addLayout(hlayout);
	}
#if QT_VERSION >= 0x040000
	setMinimumWidth(525);
#else
	resize(525, -1);
#endif
}

/**
 * Destructor.
 */
EditFrameFieldsDialog::~EditFrameFieldsDialog()
{
#if QT_VERSION >= 0x040000
	qDeleteAll(m_fieldcontrols);
	m_fieldcontrols.clear();
#endif
}

