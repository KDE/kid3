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
#if QT_VERSION >= 0x040000
#include <QListWidget>
#include <QList>
#else
#include <qlistbox.h>
#include <qptrlist.h>
#endif

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
	 * @param img    image to display in window
	 */
	ImageViewer(QWidget* parent, QImage* img);

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

/** List of field control pointers. */
#if QT_VERSION >= 0x040000
typedef QList<FieldControl*> FieldControlList;
#else
typedef QPtrList<FieldControl> FieldControlList;
#endif

/**
 * Item in frame list box.
 */
class FrameListItem : public
#if QT_VERSION >= 0x040000
QListWidgetItem
#else
QListBoxText
#endif
{
public:
	/**
	 * Constructor.
	 * @param listbox listbox
	 * @param text    title
	 * @param id      ID
	 */
#if QT_VERSION >= 0x040000
	FrameListItem(QListWidget* listbox, const QString& text, int id) :
		QListWidgetItem(text, listbox), m_id(id) {}
#else
	FrameListItem(QListBox* listbox, const QString& text, int id) :
		QListBoxText(listbox, text), m_id(id) {}
#endif

	/**
	 * Destructor.
	 */
	virtual ~FrameListItem() {}

	/**
	 * Get ID.
	 * @return ID.
	 */
	int getId() const { return m_id; }

private:
	int m_id;
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
	 * Reload the frame list, keeping the same row selected.
	 */
	void reloadTags();

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
#if QT_VERSION >= 0x040000
	static void setListBox(QListWidget* lb) { s_listbox = lb; }
#else
	static void setListBox(QListBox* lb) { s_listbox = lb; }
#endif

	/**
	 * Get the name of the selected frame.
	 *
	 * @return name, QString::null if nothing selected.
	 */
	static QString getSelectedName();

	/**
	 * Select a frame with a given name.
	 *
	 * @param name name of frame
	 *
	 * @return true if a frame with that name could be selected.
	 */
	static bool selectByName(const QString& name);

	/**
	 * Get ID of selected frame list item.
	 *
	 * @return ID of selected item,
	 *         -1 if not item is selected.
	 */
	static int getSelectedId();

	/**
	 * Select the frame by ID.
	 *
	 * @param id ID of frame to select
	 */
	static void setSelectedId(int id);

	/**
	 * Clear list box.
	 */
	static void clearListBox();

protected:
	/** File containing tags */
	TaggedFile* m_file;

	/** List box to select frame */
#if QT_VERSION >= 0x040000
	static QListWidget* s_listbox;
#else
	static QListBox* s_listbox;
#endif

private:
	FrameList(const FrameList&);
	FrameList& operator=(const FrameList&);
};

#endif // FRAMELIST_H
