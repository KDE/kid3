/**
 * \file taglibframelist.h
 * List of frames in file using TagLib.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 12 Sep 2006
 */

#ifndef TAGLIBFRAMELIST_H
#define TAGLIBFRAMELIST_H

#include <qpushbutton.h>
#include "config.h"
#ifdef HAVE_TAGLIB

// Just using include <oggfile.h>, include <flacfile.h> as recommended in the
// TagLib documentation does not work, as there are files with these names
// in this directory.
#include <taglib/tbytevector.h>
#include <taglib/tstring.h>

#include "framelist.h"
#include "taglibfile.h"

#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <Q3PtrList>
#else
#include <qptrlist.h>
#endif

class QHBoxLayout;
class QLabel;
class IntComboBoxControl;
class TextFieldControl;
#ifndef TAGLIB_SUPPORTS_URLLINK_FRAMES
class UrlLinkFrame;
class UserUrlLinkFrame;
#endif
#ifndef TAGLIB_SUPPORTS_USLT_FRAMES
class UnsynchronizedLyricsFrame;
#endif

namespace TagLib {
	class Tag;
	namespace ID3v2 {
		class Frame;
		class TextIdentificationFrame;
		class AttachedPictureFrame;
		class CommentsFrame;
		class RelativeVolumeFrame;
		class UniqueFileIdentifierFrame;
#ifdef TAGLIB_SUPPORTS_GEOB_FRAMES
		class GeneralEncapsulatedObjectFrame;
#endif
#ifdef TAGLIB_SUPPORTS_URLLINK_FRAMES
		class UrlLinkFrame;
		class UserUrlLinkFrame;
#endif
#ifdef TAGLIB_SUPPORTS_USLT_FRAMES
		class UnsynchronizedLyricsFrame;
#endif
	}
	namespace APE {
		class Item;
	}
}

#endif // HAVE_TAGLIB

/** Row of buttons to load, save and view binary data */
class TagLibBinaryOpenSave : public QWidget {
 Q_OBJECT

 public:
#ifdef HAVE_TAGLIB
	/**
	 * Constructor.
	 *
	 * @param byteArray  array with binary data
	 * @param parent     parent widget
	 * @param name       internal name or 0
	 * @param viewButton true to display View button
	 */
	TagLibBinaryOpenSave(QByteArray& byteArray, QWidget* parent,
											 const char* name = 0, bool viewButton = false);

	/**
	 * Set label.
	 *
	 * @param txt label
	 */
	void setLabel(const QString& txt) { m_label->setText(txt); }
#endif // HAVE_TAGLIB

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
#ifdef HAVE_TAGLIB
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
	/** Array with binary data */
	QByteArray& m_byteArray;
#endif // HAVE_TAGLIB
};

#ifdef HAVE_TAGLIB

/**
 * List of ID3v2.3 frames.
 */
class TagLibFrameList : public FrameList {
public:
	/**
	 * Constructor.
	 */
	TagLibFrameList();

	/**
	 * Destructor.
	 */
	virtual ~TagLibFrameList();

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

protected:
	/**
	 * Fill listbox with frame descriptions.
	 * Before using this method, the listbox and file have to be set.
	 * @see setListBox(), setTags()
	 */
	void readTags();

	/**
	 * Create dialog to edit a field with key and value and update the fields
	 * if Ok selected.
	 *
	 * @param key   key of field
	 * @param value value, will be changed if true is returned
	 *
	 * @return true if Ok selected in dialog.
	 */
	bool editKeyValueField(const TagLib::String& key, TagLib::String& value);

	/**
	 * Edit a text identification frame.
	 *
	 * @param tFrame text identification frame
	 * @param id     frame ID
	 *
	 * @return new edited frame.
	 */
	TagLib::ID3v2::Frame* editTextFrame(
		const TagLib::ID3v2::TextIdentificationFrame* tFrame, const QString& id);

	/**
	 * Edit an attached picture frame.
	 *
	 * @param apicFrame attached picture frame
	 * @param id        frame ID
	 *
	 * @return new edited frame.
	 */
	TagLib::ID3v2::Frame* editApicFrame(
		const TagLib::ID3v2::AttachedPictureFrame* apicFrame, const QString& id);

	/**
	 * Edit a comments frame.
	 *
	 * @param commFrame comments frame
	 * @param id        frame ID
	 *
	 * @return new edited frame.
	 */
	TagLib::ID3v2::Frame* editCommFrame(
		const TagLib::ID3v2::CommentsFrame* commFrame, const QString& id);

	/**
	 * Edit a relative volume frame.
	 *
	 * @param rva2Frame relative volume frame
	 * @param id        frame ID
	 *
	 * @return new edited frame.
	 */
	TagLib::ID3v2::Frame* editRva2Frame(
		const TagLib::ID3v2::RelativeVolumeFrame* rva2Frame, const QString& id);

	/**
	 * Edit a unique file identifier frame.
	 *
	 * @param ufidFrame unique file identifier frame
	 * @param id        frame ID
	 *
	 * @return new edited frame.
	 */
	TagLib::ID3v2::Frame* editUfidFrame(
		const TagLib::ID3v2::UniqueFileIdentifierFrame* ufidFrame, const QString& id);

#ifdef TAGLIB_SUPPORTS_GEOB_FRAMES
	/**
	 * Edit a general encapsulated object frame.
	 *
	 * @param geobFrame general encapsulated object frame
	 * @param id        frame ID
	 *
	 * @return new edited frame.
	 */
	TagLib::ID3v2::Frame* editGeobFrame(
		const TagLib::ID3v2::GeneralEncapsulatedObjectFrame* geobFrame,
		const QString& id);
#endif // TAGLIB_SUPPORTS_GEOB_FRAMES

	/**
	 * Edit a URL link frame.
	 *
	 * @param wFrame URL link frame
	 * @param id     frame ID
	 *
	 * @return new edited frame.
	 */
	TagLib::ID3v2::Frame* editUrlFrame(
#ifdef TAGLIB_SUPPORTS_URLLINK_FRAMES
		const TagLib::ID3v2::UrlLinkFrame* wFrame
#else
		const UrlLinkFrame* wFrame
#endif
		, const QString& id);

	/**
	 * Edit a user URL link frame.
	 *
	 * @param wxxxFrame user URL link frame
	 * @param id        frame ID
	 *
	 * @return new edited frame.
	 */
	TagLib::ID3v2::Frame* editUserUrlFrame(
#ifdef TAGLIB_SUPPORTS_URLLINK_FRAMES
		const TagLib::ID3v2::UserUrlLinkFrame* wxxxFrame
#else
		const UserUrlLinkFrame* wxxxFrame
#endif
		, const QString& id);

	/**
	 * Edit an unsynchronized lyrics frame.
	 *
	 * @param usltFrame unsynchronized frame
	 * @param id        frame ID
	 *
	 * @return new edited frame.
	 */
	TagLib::ID3v2::Frame* editUsltFrame(
#ifdef TAGLIB_SUPPORTS_USLT_FRAMES
		const TagLib::ID3v2::UnsynchronizedLyricsFrame* usltFrame
#else
		const UnsynchronizedLyricsFrame* usltFrame
#endif
		, const QString& id);

	/**
	 * Edit an unknown frame.
	 *
	 * @param unknownFrame unknown frame
	 * @param id           frame ID
	 *
	 * @return new edited frame.
	 */
	TagLib::ID3v2::Frame* editUnknownFrame(
		const TagLib::ID3v2::Frame* unknownFrame, const QString& id);

	/**
	 * Create dialog to edit an ID3v2 frame and return a modified copy
	 * if Ok is selected.
	 *
	 * @param frame frame to edit
	 *
	 * @return a new frame if Ok selected in dialog, else 0.
	 */
	TagLib::ID3v2::Frame* editId3v2Frame(const TagLib::ID3v2::Frame* frame);

	/**
	 * Get a description of an ID3v2 frame.
	 *
	 * @param frameId ID3v2 frame ID
	 *
	 * @return description.
	 */
	QString getId3v2FrameDescription(TagLib::ByteVector frameId) const;

	/**
	 * Copy an ID3v2 frame.
	 *
	 * @param frame original frame
	 *
	 * @return new frame.
	 */
	TagLib::ID3v2::Frame* copyId3v2Frame(const TagLib::ID3v2::Frame* frame) const;

	/**
	 * Create m_tag if it does not already exist so that it can be set.
	 *
	 * @return true if m_tag can be set.
	 */
	bool makeTagSettable();

	TagLib::Tag* m_tag;            /**< tags of current file */
	QString m_selectedName;        /**< the type of the frame to add */
	/** List with controls to edit fields in frame */
	Q3PtrList<FieldControl> m_fieldcontrols; 
	TagLib::ByteVector m_copyData; /**< data used by copyFrame(), pasteFrame */
	TagLib::String m_copyKey;      /**< key used by copyFrame(), pasteFrame */
	TagLib::String m_copyValue;    /**< value used by copyFrame(), pasteFrame */

private:
	TagLibFrameList(const TagLibFrameList&);
	TagLibFrameList& operator=(const TagLibFrameList&);
};

#endif // HAVE_TAGLIB

#endif // TAGLIBFRAMELIST_H
