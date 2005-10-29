/**
 * \file mp3framelist.h
 * List of ID3v2.3 frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 26 Sep 2005
 */

#ifndef MP3FRAMELIST_H
#define MP3FRAMELIST_H

#include "framelist.h"

#if QT_VERSION < 300
#include <qlist.h>
#define QPtrList QList
#else
#include <qptrlist.h>
#endif
#include <qpushbutton.h>
#include <id3/tag.h>

class Mp3File;
class IntComboBoxControl;
class TextFieldControl;

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


/**
 * List of ID3v2.3 frames.
 */
class Mp3FrameList : public FrameList {
public:
	/**
	 * Constructor.
	 */
	Mp3FrameList();
	/**
	 * Destructor.
	 */
	virtual ~Mp3FrameList();
	/**
	 * Set file and fill the list box with its frames.
	 * The listbox has to be set with setListBox() before calling this
	 * function.
	 *
	 * @param taggedFile file
	 */
	virtual void setTags(TaggedFile* taggedFile);
	/**
	 * Create dialog to edit the selected frame and update the fields
	 * if Ok is returned.
	 *
	 * @return TRUE if Ok selected in dialog.
	 */
	virtual bool editFrame(void);
	/**
	 * Delete selected frame.
	 *
	 * @return FALSE if frame not found.
	 */
	virtual bool deleteFrame(void);
	/**
	 * Add a new frame.
	 *
	 * @param frameId ID of frame to add
	 * @return TRUE if frame added.
	 */
	virtual bool addFrame(int frameId);
	/**
	 * Display a dialog to select a frame type.
	 *
	 * @return ID of selected frame,
	 *         -1 if no frame selected.
	 */
	virtual int selectFrameId(void);

	friend class IntComboBoxControl; // access to setSelectedEncoding()
	friend class TextFieldControl;   // access to getSelectedEncoding()

 private:
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
	 * @param lbIndex if not 0, the listbox index of the selected frame
	 *                (-1 if none selected) is returned here
	 *
	 * @return selected frame.
	 */
	ID3_Frame *getSelectedFrame(int* lbIndex = 0) const;
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

	/** ID3v2 tags containing frames */
	ID3_Tag *tags;
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

#endif // MP3FRAMELIST_H
