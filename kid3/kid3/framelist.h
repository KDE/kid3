/**
 * \file framelist.h
 * List of ID3v2.3 frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

#ifndef FRAMELIST_H
#define FRAMELIST_H

#include <qlabel.h>
#include <qlayout.h>
#if QT_VERSION < 300
#include <qmultilineedit.h>
#define QTextEdit QMultiLineEdit
#define setTextFormat(x) isReadOnly() /* just something which does nothing */
#include <qlist.h>
#define QPtrList QList
#else
#include <qtextedit.h>
#include <qptrlist.h>
#endif
#include <qdialog.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qcombobox.h>

#include <id3/tag.h>

class QListBox;
class QPaintEvent;
class Mp3File;

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

/** Row of buttons to load, save and view binary data */
class BinaryOpenSave : public QWidget {
 Q_OBJECT

 public:
	/**
	 * Constructor.
	 *
	 * @param parent parent widget
	 * @param name   internal name or 0
	 * @param fld    ID3_Field containing binary data
	 */
	BinaryOpenSave(QWidget *parent, const char *name, ID3_Field *fld);
	/**
	 * Set label.
	 *
	 * @param txt label
	 */
	void setLabel(const QString & txt) { label->setText(txt); }
	/**
	 * Get filename of file to import.
	 *
	 * @return filename.
	 */
	QString getFilename(void) { return loadfilename; }
 
 public slots:
 	/**
	 * Request name of file to import binary data from.
	 * The data is imported later when Ok is pressed in the parent dialog.
	 */
	void loadData(void);
	/**
	 * Request name of file and export binary data.
	 */
	void saveData(void);
	/**
	 * Create image from binary data and display it in window.
	 */
	void viewData(void);

 private:
	/** ID3 field containing binary data */
	ID3_Field *field;
	/** horizontal layout */
	QHBoxLayout *layout;
	/** Label left of buttons */
	QLabel *label;
	/** filename of file to import */
	QString loadfilename;
	/** Import push button */
	QPushButton *openButton;
	/** Export push button */
	QPushButton *saveButton;
	/** View push button */
	QPushButton *viewButton;
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

class FrameList;

/** Base class for field controls */
class FieldControl : public QObject {
 public:
	/**
	 * Constructor.
	 *
	 * @param fl  frame list
	 * @param id  ID of field
	 * @param fld ID3 field
	 */
	FieldControl(FrameList *fl, ID3_FieldID id, ID3_Field *fld) :
	    frmlst(fl), field_id(id), field(fld) {}
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

 protected:
	/**
	 * Get description for ID3_Field.
	 *
	 * @param id ID of field
	 * @return description or NULL if id unknown.
	 */
	const char *getFieldIDString(ID3_FieldID id) const;
	/** Frame list */
	FrameList *frmlst;
	/** Field ID */
	ID3_FieldID field_id;
	/** ID3 field */
	ID3_Field *field;
};

/** Control to edit standard UTF text fields */
class TextFieldControl : public FieldControl {
 public:
	/**
	 * Constructor.
	 *
	 * @param fl  frame list
	 * @param id  ID of field
	 * @param fld ID3 field
	 */
	TextFieldControl(FrameList *fl, ID3_FieldID id, ID3_Field *fld) :
	    FieldControl(fl, id, fld) {}
	/**
	 * Destructor.
	 */
	virtual ~TextFieldControl() {}
	/**
	 * Update field from data in field control.
	 */
	virtual void updateTag(void);
	/**
	 * Create widget to edit field data.
	 *
	 * @param parent parent widget
	 *
	 * @return widget to edit field data.
	 */
	virtual QWidget *createWidget(QWidget *parent);

 protected:
	/** Text editor widget */
	LabeledTextEdit *edit;
};

/** Control to edit single line text fields */
class LineFieldControl : public FieldControl {
 public:
	/**
	 * Constructor.
	 *
	 * @param fl  frame list
	 * @param id  ID of field
	 * @param fld ID3 field
	 */
	LineFieldControl(FrameList *fl, ID3_FieldID id, ID3_Field *fld) :
	    FieldControl(fl, id, fld) {}
	/**
	 * Destructor.
	 */
	virtual ~LineFieldControl() {}
	/**
	 * Update field from data in field control.
	 */
	virtual void updateTag(void);
	/**
	 * Create widget to edit field data.
	 *
	 * @param parent parent widget
	 *
	 * @return widget to edit field data.
	 */
	virtual QWidget *createWidget(QWidget *parent);

 protected:
	/** Line editor widget */
	LabeledLineEdit *edit;
};

/** Control to edit integer fields */
class IntFieldControl : public FieldControl {
 public:
	/**
	 * Constructor.
	 *
	 * @param fl  frame list
	 * @param id  ID of field
	 * @param fld ID3 field
	 */
	IntFieldControl(FrameList *fl, ID3_FieldID id, ID3_Field *fld) :
	    FieldControl(fl, id, fld) {};
	/**
	 * Destructor.
	 */
	virtual ~IntFieldControl() {}
	/**
	 * Update field from data in field control.
	 */
	virtual void updateTag(void);
	/**
	 * Create widget to edit field data.
	 *
	 * @param parent parent widget
	 *
	 * @return widget to edit field data.
	 */
	virtual QWidget *createWidget(QWidget *parent);

 protected:
	/** Spin box widget */
	LabeledSpinBox *numinp;
};

/** Control to edit integer fields using a combo box with given values */
class IntComboBoxControl : public FieldControl {
 public:
	/**
	 * Constructor.
	 *
	 * @param fl  frame list
	 * @param id  ID of field
	 * @param fld ID3 field
	 * @param lst list of strings with possible selections, NULL terminated
	 */
	IntComboBoxControl(FrameList *fl, ID3_FieldID id, ID3_Field *fld, const char **lst) :
	    FieldControl(fl, id, fld), strlst(lst) {};
	/**
	 * Destructor.
	 */
	virtual ~IntComboBoxControl() {}
	/**
	 * Update field from data in field control.
	 */
	virtual void updateTag(void);
	/**
	 * Create widget to edit field data.
	 *
	 * @param parent parent widget
	 *
	 * @return widget to edit field data.
	 */
	virtual QWidget *createWidget(QWidget *parent);

 protected:
	/** Combo box widget */
	LabeledComboBox *ptinp;
	/** List of strings with possible selections */
	const char **strlst;
};

/** Control to import, export and view data from binary fields */
class BinFieldControl : public FieldControl {
 public:
	/**
	 * Constructor.
	 *
	 * @param fl  frame list
	 * @param id  ID of field
	 * @param fld ID3 field
	 */
	BinFieldControl(FrameList *fl, ID3_FieldID id, ID3_Field *fld) :
	    FieldControl(fl, id, fld) {};
	/**
	 * Destructor.
	 */
	virtual ~BinFieldControl() {}
	/**
	 * Update field from data in field control.
	 */
	virtual void updateTag(void);
	/**
	 * Create widget to edit field data.
	 *
	 * @param parent parent widget
	 *
	 * @return widget to edit field data.
	 */
	virtual QWidget *createWidget(QWidget *parent);

 protected:
	/** Import, Export, View buttons */
	BinaryOpenSave *bos;
};

/**
 * List of ID3v2.3 frames.
 */
class FrameList : public QObject {
 public: 
	/**
	 * Constructor.
	 */
	FrameList();
	/**
	 * Clear listbox and file reference.
	 */
	void clear(void);
	/**
	 * Set list box to select frames.
	 *
	 * @param lb list box
	 */
	void setListBox(QListBox *lb) { listbox = lb; }
	/**
	 * Set file and fill the list box with its frames.
	 * The listbox has to be set with setListBox() before calling this
	 * function.
	 *
	 * @param mp3file file
	 */
	void setTags(Mp3File *mp3file);
	/**
	 * Create dialog to edit the selected frame and update the fields
	 * if Ok is returned.
	 *
	 * @return TRUE if Ok selected in dialog.
	 */
	bool editFrame(void);
	/**
	 * Delete selected frame.
	 *
	 * @return FALSE if frame not found.
	 */
	bool deleteFrame(void);
	/**
	 * Add a new frame.
	 *
	 * @param id ID of frame to add
	 * @return TRUE if frame added.
	 */
	bool addFrame(ID3_FrameID id);
	/**
	 * Get file containing frames.
	 *
	 * @return file, NULL if no file selected.
	 */
	Mp3File *getFile(void) const { return file; }
	/**
	 * Display a dialog to select a frame type.
	 *
	 * @return ID of selected frame,
	 *         ID3FID_NOFRAME if no frame selected.
	 */
	static ID3_FrameID selectFrameId(void);
	/**
	 * Set encoding selected in frame dialog.
	 *
	 * @param enc encoding.
	 */
	void setSelectedEncoding(ID3_TextEnc enc) { selected_enc = enc; }
	/**
	 * Get encoding selected in frame dialog.
	 *
	 * @return encoding, ID3TE_NONE if none selected.
	 */
	ID3_TextEnc getSelectedEncoding(void) const { return selected_enc; }

 private:
	/**
	 * Get frame with index.
	 *
	 * @param index index in listbox
	 * @return frame with index.
	 */
	ID3_Frame *getFrame(int index) const;
	/**
	 * Get frame which is selected in listbox.
	 *
	 * @return selected frame.
	 */
	ID3_Frame *getSelectedFrame(void) const;
	/**
	 * Fill listbox with frame descriptions.
	 * Before using this method, the listbox and file have to be set.
	 * @see setListBox(), setTags()
	 */
	void readTags(void);
	/**
	 * Get description of frame.
	 *
	 * @param id ID of frame
	 * @return description or NULL if id not found.
	 */
	const char *getIdString(ID3_FrameID id) const;

	/** List box to select frame */
	QListBox *listbox;
	/** ID3v2 tags containing frames */
	ID3_Tag *tags;
	/** File containing tags */
	Mp3File *file;
	/** List with controls to edit fields in frame */
	QPtrList<FieldControl> fieldcontrols; 
	/** Number of possible frame IDs */
#if defined _WIN32 || defined WIN32
	enum { num_frameid = 76 };
#else
	static const int num_frameid = 76;
#endif
	/** Encoding selected in frame dialog */
	ID3_TextEnc selected_enc;
	/** Alphabetically sorted list of frame descriptions */
	static const char *frameid_str[num_frameid];
	/** Frame IDs corresponding to frameid_str[] */
	static const ID3_FrameID frameid_code[num_frameid];
};

#endif // FRAMELIST_H
