/**
 * \file framelist.h
 * List of frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

#ifndef FRAMELIST_H
#define FRAMELIST_H

#include <qlabel.h>
#include <qlayout.h>
#include <qtextedit.h>
#include <qdialog.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include "qtcompatmac.h"

class Q3ListBox;
class QVBoxLayout;
class QPaintEvent;
class TaggedFile;
class FrameList;


/** QTextEdit with label above */
class LabeledTextEdit : public QWidget {
 public:
	/**
	 * Constructor.
	 *
	 * @param parent parent widget
	 * @param name   internal name or 0
	 */
	LabeledTextEdit(QWidget* parent, const char* name = 0);

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
	/** Vertical layout */
	QVBoxLayout* m_layout;
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
	 * @param name   internal name or 0
	 */
	LabeledLineEdit(QWidget* parent, const char* name = 0);

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
	/** Vertical layout */
	QVBoxLayout* m_layout;
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
	 * @param name   internal name or 0
	 * @param strlst list with ComboBox items, terminated by NULL
	 */
	LabeledComboBox(QWidget* parent, const char* name,
			const char** strlst);

	/**
	 * Get index of selected item.
	 *
	 * @return index.
	 */
	int currentItem() const { return m_combo->currentItem(); }

	/**
	 * Set index of selected item.
	 *
	 * @param idx index
	 */
	void setCurrentItem(int idx) { m_combo->setCurrentItem(idx); }

	/**
	 * Set label.
	 *
	 * @param txt label
	 */
	void setLabel(const QString& txt) { m_label->setText(txt); }

 private:
	/** Vertical layout */
	QVBoxLayout* m_layout;
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
	 * @param name   internal name or 0
	 */
	LabeledSpinBox(QWidget* parent, const char* name = 0);

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
	/** Vertical layout */
	QVBoxLayout* m_layout;
	/** Label above edit */
	QLabel* m_label;
	/** Text editor */
	QSpinBox* m_spinbox;
};


/** Window to view image */
class ImageViewer : public QDialog {
 public:
	/**
	 * Constructor.
	 *
	 * @param parent parent widget
	 * @param name   internal name or 0
	 * @param img    image to display in window
	 */
	ImageViewer(QWidget* parent, const char* name, QImage* img);

 protected:
	/**
	 * Paint image, called when window has to be drawn.
	 */
	void paintEvent(QPaintEvent*);

 private:
	/** image to view */
	QImage* m_image; 
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


/**
 * List of frames.
 */
class FrameList : public QObject {
public:
	/**
	 * Constructor.
	 */
	FrameList();

	/**
	 * Destructor.
	 */
	virtual ~FrameList();

	/**
	 * Clear listbox and file reference.
	 */
	void clear();

	/**
	 * Set file and fill the list box with its frames.
	 * The listbox has to be set with setListBox() before calling this
	 * function.
	 *
	 * @param taggedFile file
	 */
	virtual void setTags(TaggedFile* taggedFile) = 0;

	/**
	 * Create dialog to edit the selected frame and update the fields
	 * if Ok is returned.
	 *
	 * @return true if Ok selected in dialog.
	 */
	virtual bool editFrame() = 0;

	/**
	 * Delete selected frame.
	 *
	 * @return false if frame not found.
	 */
	virtual bool deleteFrame() = 0;

	/**
	 * Add a new frame.
	 *
	 * @param frameId ID of frame to add
	 * @param edit    true to edit frame after adding it
	 * @return true if frame added.
	 */
	virtual bool addFrame(int frameId, bool edit = false) = 0;

	/**
	 * Copy the selected frame to the copy buffer.
	 *
	 * @return true if frame copied.
	 */
	virtual bool copyFrame() = 0;

	/**
	 * Paste the selected frame from the copy buffer.
	 *
	 * @return true if frame pasted.
	 */
	virtual bool pasteFrame() = 0;

	/**
	 * Get file containing frames.
	 *
	 * @return file, NULL if no file selected.
	 */
	TaggedFile* getFile() const;

	/**
	 * Display a dialog to select a frame type.
	 *
	 * @return ID of selected frame,
	 *         -1 if no frame selected.
	 */
	virtual int selectFrameId() = 0;

	/**
	 * Set list box to select frames.
	 *
	 * @param lb list box
	 */
	static void setListBox(Q3ListBox* lb) { s_listbox = lb; }

protected:
	/** File containing tags */
	TaggedFile* m_file;

	/** List box to select frame */
	static Q3ListBox* s_listbox;

private:
	FrameList(const FrameList&);
	FrameList& operator=(const FrameList&);
};

#endif // FRAMELIST_H
