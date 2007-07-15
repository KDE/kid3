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

#include <qpushbutton.h>
#include "config.h"
#ifdef HAVE_ID3LIB

#include "framelist.h"

#include <id3/tag.h>

class Mp3File;
class IntComboBoxControl;
class TextFieldControl;
class QHBoxLayout;
class QLabel;

#endif // HAVE_ID3LIB

/** Row of buttons to load, save and view binary data */
class BinaryOpenSave : public QWidget {
 Q_OBJECT

 public:
#ifdef HAVE_ID3LIB
	/**
	 * Constructor.
	 *
	 * @param parent parent widget
	 * @param fld    ID3_Field containing binary data
	 */
	BinaryOpenSave(QWidget* parent, ID3_Field* fld);

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
#endif // HAVE_ID3LIB
 
 public slots:
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
#ifdef HAVE_ID3LIB
	/** Array with binary data */
	QByteArray m_byteArray;
	/** true if m_byteArray changed */
	bool m_isChanged;
	/** horizontal layout */
	QHBoxLayout* m_layout;
	/** Label left of buttons */
	QLabel* m_label;
	/** Import push button */
	QPushButton* m_openButton;
	/** Export push button */
	QPushButton* m_saveButton;
	/** View push button */
	QPushButton* m_viewButton;
#endif // HAVE_ID3LIB
};

#ifdef HAVE_ID3LIB

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
	 * @return true if Ok selected in dialog.
	 */
	virtual bool editFrame();

	/**
	 * Delete selected frame.
	 *
	 * @return false if frame not found.
	 */
	virtual bool deleteFrame();

	/**
	 * Add a new frame.
	 *
	 * @param frameId ID of frame to add
	 * @param edit    true to edit frame after adding it
	 * @return true if frame added.
	 */
	virtual bool addFrame(int frameId, bool edit = false);

	/**
	 * Copy the selected frame to the copy buffer.
	 *
	 * @return true if frame copied.
	 */
	virtual bool copyFrame();

	/**
	 * Paste the selected frame from the copy buffer.
	 *
	 * @return true if frame pasted.
	 */
	virtual bool pasteFrame();

	/**
	 * Display a dialog to select a frame type.
	 *
	 * @return ID of selected frame,
	 *         -1 if no frame selected.
	 */
	virtual int selectFrameId();

	friend class IntComboBoxControl; // access to setSelectedEncoding()
	friend class TextFieldControl;   // access to getSelectedEncoding()

 private:
	Mp3FrameList(const Mp3FrameList&);
	Mp3FrameList& operator=(const Mp3FrameList&);

	/**
	 * Set encoding selected in frame dialog.
	 *
	 * @param enc encoding.
	 */
	void setSelectedEncoding(ID3_TextEnc enc) { m_selectedEnc = enc; }

	/**
	 * Get encoding selected in frame dialog.
	 *
	 * @return encoding, ID3TE_NONE if none selected.
	 */
	ID3_TextEnc getSelectedEncoding() const { return m_selectedEnc; }

	/**
	 * Get frame with index.
	 *
	 * @param index index in listbox
	 * @return frame with index.
	 */
	ID3_Frame* getFrame(int index) const;

	/**
	 * Get frame which is selected in listbox.
	 *
	 * @param lbIndex if not 0, the listbox index of the selected frame
	 *                (-1 if none selected) is returned here
	 *
	 * @return selected frame.
	 */
	ID3_Frame* getSelectedFrame(int* lbIndex = 0) const;

	/**
	 * Fill listbox with frame descriptions.
	 * Before using this method, the listbox and file have to be set.
	 * @see setListBox(), setTags()
	 */
	void readTags();

	/**
	 * Create dialog to edit a frame and update the fields if Ok is
	 * returned.
	 *
	 * @param frame frame to edit
	 *
	 * @return true if Ok selected in dialog.
	 */
	bool editFrame(ID3_Frame* frame);

	/**
	 * Get description of frame.
	 *
	 * @param id ID of frame
	 * @return description or NULL if id not found.
	 */
	const char* getIdString(ID3_FrameID id) const;

	/** ID3v2 tags containing frames */
	ID3_Tag* m_tags;
	/** List with controls to edit fields in frame */
	FieldControlList m_fieldcontrols; 
	/** Number of possible frame IDs */
#if defined _WIN32 || defined WIN32
	enum { NumFrameIds = 74 };
#else
	static const int NumFrameIds = 74;
#endif
	/** Encoding selected in frame dialog */
	ID3_TextEnc m_selectedEnc;
	/** Frame storage for copy-paste */
	ID3_Frame* m_copyFrame;

	/** Alphabetically sorted list of frame descriptions */
	static const char* s_frameIdStr[NumFrameIds];
	/** Frame IDs corresponding to s_frameIdStr[] */
	static const ID3_FrameID s_frameIdCode[NumFrameIds];
};

#endif // HAVE_ID3LIB

#endif // MP3FRAMELIST_H
