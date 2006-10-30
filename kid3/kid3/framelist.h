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

class QListBox;
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
	LabeledTextEdit(QWidget *parent, const char *name = 0);
	/**
	 * Get text.
	 *
	 * @return text.
	 */
	QString text() const { return edit->text(); }
	/**
	 * Set text.
	 *
	 * @param txt text
	 */
	void setText(const QString & txt) { edit->setText(txt); }
	/**
	 * Set label.
	 *
	 * @param txt label
	 */
	void setLabel(const QString & txt) { label->setText(txt); }

 private:
	/** Vertical layout */
	QVBoxLayout *layout;
	/** Label above edit */
	QLabel *label;
	/** Text editor */
	QTextEdit *edit;
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
	LabeledLineEdit(QWidget *parent, const char *name = 0);
	/**
	 * Get text.
	 *
	 * @return text.
	 */
	QString text() const { return edit->text(); }
	/**
	 * Set text.
	 *
	 * @param txt text
	 */
	void setText(const QString & txt) { edit->setText(txt); }
	/**
	 * Set label.
	 *
	 * @param txt label
	 */
	void setLabel(const QString & txt) { label->setText(txt); }

 private:
	/** Vertical layout */
	QVBoxLayout *layout;
	/** Label above edit */
	QLabel *label;
	/** Line editor */
	QLineEdit *edit;
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
	LabeledComboBox(QWidget *parent, const char *name,
			const char **strlst);
	/**
	 * Get index of selected item.
	 *
	 * @return index.
	 */
	int currentItem() const { return combo->currentItem(); }
	/**
	 * Set index of selected item.
	 *
	 * @param idx index
	 */
	void setCurrentItem(int idx) { combo->setCurrentItem(idx); }
	/**
	 * Set label.
	 *
	 * @param txt label
	 */
	void setLabel(const QString & txt) { label->setText(txt); }

 private:
	/** Vertical layout */
	QVBoxLayout *layout;
	/** Label above combo box */
	QLabel *label;
	/** Combo box */
	QComboBox *combo;
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
	LabeledSpinBox(QWidget *parent, const char *name = 0);
	/**
	 * Get value.
	 *
	 * @return text.
	 */
	int value() const { return spinbox->value(); }
	/**
	 * Set value.
	 *
	 * @param value value
	 */
	void setValue(int value) { spinbox->setValue(value); }
	/**
	 * Set label.
	 *
	 * @param txt label
	 */
	void setLabel(const QString & txt) { label->setText(txt); }

 private:
	/** Vertical layout */
	QVBoxLayout *layout;
	/** Label above edit */
	QLabel *label;
	/** Text editor */
	QSpinBox *spinbox;
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
	ImageViewer(QWidget *parent, const char *name, QImage *img);
 protected:
	/**
	 * Paint image, called when window has to be drawn.
	 */
	void paintEvent(QPaintEvent *);

 private:
	/** image to view */
	QImage *image; 
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
	virtual void updateTag(void) = 0;
	/**
	 * Create widget to edit field data.
	 *
	 * @param parent parent widget
	 *
	 * @return widget to edit field data.
	 */
	virtual QWidget *createWidget(QWidget *parent) = 0;
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
	void clear(void);
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
	 * @return TRUE if Ok selected in dialog.
	 */
	virtual bool editFrame(void) = 0;
	/**
	 * Delete selected frame.
	 *
	 * @return FALSE if frame not found.
	 */
	virtual bool deleteFrame(void) = 0;
	/**
	 * Add a new frame.
	 *
	 * @param frameId ID of frame to add
	 * @param edit    true to edit frame after adding it
	 * @return TRUE if frame added.
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
	TaggedFile* getFile(void) const;
	/**
	 * Display a dialog to select a frame type.
	 *
	 * @return ID of selected frame,
	 *         -1 if no frame selected.
	 */
	virtual int selectFrameId(void) = 0;

	/**
	 * Set list box to select frames.
	 *
	 * @param lb list box
	 */
	static void setListBox(QListBox *lb) { listbox = lb; }

protected:
	/** File containing tags */
	TaggedFile* m_file;

	/** List box to select frame */
	static QListBox* listbox;

private:
	FrameList(const FrameList&);
	FrameList& operator=(const FrameList&);
};

#endif // FRAMELIST_H
