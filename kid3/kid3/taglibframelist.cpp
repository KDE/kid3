/**
 * \file taglibframelist.cpp
 * List of frames in file using TagLib.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 12 Sep 2006
 */

#include "taglibframelist.h"
#ifdef HAVE_TAGLIB

#ifdef CONFIG_USE_KDE
#include <klocale.h>
#include <kdialogbase.h>
#include <kfiledialog.h>
#else
#include <qdialog.h>
#include <qfiledialog.h>
#define i18n(s) tr(s)
#define I18N_NOOP(s) QT_TR_NOOP(s)
#endif

#include <qfile.h>
#include <qdatastream.h>
#include <qlistbox.h>
#include <qimage.h>
#include <qinputdialog.h>
#include <qvalidator.h>
#include <qhbox.h>
#include <qvbox.h>

// Just using #include <oggfile.h>, #include <flacfile.h> as recommended in the
// TagLib documentation does not work, as there are files with these names
// in this directory.
#include <taglib/id3v2tag.h>
#include <taglib/xiphcomment.h>
#include <taglib/apetag.h>
#include <taglib/apeitem.h>
#include <taglib/textidentificationframe.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/commentsframe.h>
#include <taglib/relativevolumeframe.h>
#include <taglib/uniquefileidentifierframe.h>
#include <taglib/mpegfile.h>

#ifdef TAGLIB_SUPPORTS_GEOB_FRAMES
#include <taglib/generalencapsulatedobjectframe.h>
#endif
#ifdef TAGLIB_SUPPORTS_URLLINK_FRAMES
#include <taglib/urllinkframe.h>
#else
#include "urllinkframe.h"
#endif
#ifdef TAGLIB_SUPPORTS_USLT_FRAMES
#include <taglib/unsynchronizedlyricsframe.h>
#else
#include "unsynchronizedlyricsframe.h"
#endif

/**
 * Constructor.
 *
 * @param byteArray array with binary data
 * @param parent    parent widget
 * @param name      internal name or 0
 */
TagLibBinaryOpenSave::TagLibBinaryOpenSave(
	QByteArray& byteArray, QWidget* parent, const char* name, bool viewButton) :
	QWidget(parent, name), m_byteArray(byteArray)
{
	m_layout = new QHBoxLayout(this);
	m_label = new QLabel(this);
	m_openButton = new QPushButton(i18n("&Import"), this);
	m_saveButton = new QPushButton(i18n("&Export"), this);
	m_viewButton = viewButton ? new QPushButton(i18n("&View"), this) : 0;
	if (m_layout && m_label && m_openButton && m_saveButton) {
		m_layout->addWidget(m_label);
		m_layout->addWidget(m_openButton);
		m_layout->addWidget(m_saveButton);
		if (m_viewButton) {
			m_layout->addWidget(m_viewButton);
			connect(m_viewButton, SIGNAL(clicked()), this, SLOT(viewData()));
		}
		connect(m_openButton, SIGNAL(clicked()), this, SLOT(loadData()));
		connect(m_saveButton, SIGNAL(clicked()), this, SLOT(saveData()));
	}
}

/**
 * Request name of file to import binary data from.
 * The data is imported later when Ok is pressed in the parent dialog.
 */
void TagLibBinaryOpenSave::loadData()
{
#ifdef CONFIG_USE_KDE
	QString loadfilename = KFileDialog::getOpenFileName(
		QString::null, QString::null, this);
#else
	QString loadfilename = QFileDialog::getOpenFileName(
		QString::null, QString::null, this);
#endif
	if (!loadfilename.isEmpty()) {
		QFile file(loadfilename);
		if (file.open(IO_ReadOnly)) {
			size_t size = file.size();
			char* data = new char[size];
			if (data) {
				QDataStream stream(&file);
				stream.readRawBytes(data, size);
				m_byteArray.duplicate(data, size);
				delete [] data;
			}
			file.close();
		}
	}
}

/**
 * Request name of file and export binary data.
 */
void TagLibBinaryOpenSave::saveData()
{
#ifdef CONFIG_USE_KDE
	QString fn = KFileDialog::getSaveFileName(QString::null, QString::null,
																						this);
#else
	QString fn = QFileDialog::getSaveFileName(QString::null, QString::null,
																						this);
#endif
	if (!fn.isEmpty()) {
		QFile file(fn);
		if (file.open(IO_WriteOnly)) {
			QDataStream stream(&file);
			stream.writeRawBytes(m_byteArray.data(), m_byteArray.size());
			file.close();
		}
	}
}

/**
 * Create image from binary data and display it in window.
 */
void TagLibBinaryOpenSave::viewData()
{
	QImage image;
	if (image.loadFromData(m_byteArray)) {
		ImageViewer iv(this, 0, &image);
		iv.exec();
	}
}


/** Base class for TagLib field controls */
class TagLibFieldControl : public FieldControl {
public:
	/**
	 * Constructor.
	 *
	 * @param fl  frame list
	 * @param id  ID of field
	 * @param fld ID3 field
	 */
	TagLibFieldControl() {}

	/**
	 * Destructor.
	 */
	virtual ~TagLibFieldControl() {}

	/**
	 * Update field from data in field control.
	 */
	virtual void updateTag() {};
};


/** Control to edit standard UTF text fields */
class TagLibTextFieldControl : public TagLibFieldControl {
public:
	/**
	 * Constructor.
	 * @param label label
	 * @param text  text
	 */
	explicit TagLibTextFieldControl(
		const QString& label, const QString& text = QString::null) :
	m_label(label), m_text(text) {}

	/**
	 * Destructor.
	 */
	virtual ~TagLibTextFieldControl() {}

	/**
	 * Create widget to edit field data.
	 *
	 * @param parent parent widget
	 *
	 * @return widget to edit field data.
	 */
	virtual QWidget* createWidget(QWidget* parent);

	/**
	 * Set text.
	 * @param text text
	 */
	void setText(const QString& text) {
		m_text = text;
		if (m_edit) {
			m_edit->setText(text);
		}
	}

	/**
	 * Get text.
	 * @return text.
	 */
	QString getText() const {
		return m_edit ? m_edit->text() : m_text;
	}

protected:
	/** Text editor widget */
	LabeledTextEdit* m_edit;
	/** Label */
	QString m_label;
	/** Text */
	QString m_text;
};

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */

QWidget* TagLibTextFieldControl::createWidget(QWidget* parent)
{
	m_edit = new LabeledTextEdit(parent);
	if (m_edit) {
		m_edit->setLabel(m_label);
		m_edit->setText(m_text);
	}
	return m_edit;
}


/** Control to edit single line text fields */
class TagLibLineFieldControl : public TagLibFieldControl {
public:
	/**
	 * Constructor.
	 * @param label label
	 * @param text  text
	 */
	explicit TagLibLineFieldControl(
		const QString& label, const QString& text = QString::null) :
	m_label(label), m_text(text) {}

	/**
	 * Destructor.
	 */
	virtual ~TagLibLineFieldControl() {}

	/**
	 * Create widget to edit field data.
	 *
	 * @param parent parent widget
	 *
	 * @return widget to edit field data.
	 */
	virtual QWidget* createWidget(QWidget* parent);

	/**
	 * Set text.
	 * @param text text
	 */
	void setText(const QString& text) {
		m_text = text;
		if (m_edit) {
			m_edit->setText(text);
		}
	}

	/**
	 * Get text.
	 * @return text.
	 */
	QString getText() const {
		return m_edit ? m_edit->text() : m_text;
	}

protected:
	/** Line editor widget */
	LabeledLineEdit* m_edit;
	/** Label */
	QString m_label;
	/** Text */
	QString m_text;
};

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* TagLibLineFieldControl::createWidget(QWidget* parent)
{
	m_edit = new LabeledLineEdit(parent);
	if (m_edit) {
		m_edit->setLabel(m_label);
		m_edit->setText(m_text);
	}
	return m_edit;
}


/** Control to edit integer fields */
class TagLibIntFieldControl : public TagLibFieldControl {
public:
	/**
	 * Constructor.
	 * @param label label
	 * @param value value
	 */
	explicit TagLibIntFieldControl(const QString& label, int value = 0) :
	m_label(label), m_value(value) {}

	/**
	 * Destructor.
	 */
	virtual ~TagLibIntFieldControl() {}

	/**
	 * Create widget to edit field data.
	 *
	 * @param parent parent widget
	 *
	 * @return widget to edit field data.
	 */
	virtual QWidget* createWidget(QWidget* parent);

	/**
	 * Set value.
	 * @param value value
	 */
	void setValue(int value) {
		m_value = value;
		if (m_numinp) {
			m_numinp->setValue(value);
		}
	}

	/**
	 * Get value.
	 * @return value.
	 */
	int getValue() const {
		return m_numinp ? m_numinp->value() : m_value;
	}

protected:
	/** Spin box widget */
	LabeledSpinBox* m_numinp;
	/** Label */
	QString m_label;
	/** Value */
	int m_value;
};

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* TagLibIntFieldControl::createWidget(QWidget* parent)
{
	m_numinp = new LabeledSpinBox(parent);
	if (m_numinp) {
		m_numinp->setLabel(m_label);
		m_numinp->setValue(m_value);
	}
	return m_numinp;
}


/** Control to edit integer fields using a combo box with given values */
class TagLibIntComboBoxControl : public TagLibFieldControl {
public:
	/**
	 * Constructor.
	 *
	 * @param strlst list of strings with possible selections, 0 terminated
	 * @param label  label
	 * @param item   item
	 */
	TagLibIntComboBoxControl(const char** strlst, const QString& label, int item = 0) :
	m_strlst(strlst), m_label(label), m_item(item) {}

	/**
	 * Destructor.
	 */
	virtual ~TagLibIntComboBoxControl() {}

	/**
	 * Create widget to edit field data.
	 *
	 * @param parent parent widget
	 *
	 * @return widget to edit field data.
	 */
	virtual QWidget* createWidget(QWidget* parent);

	/**
	 * Set current item.
	 * @param item item
	 */
	void setCurrentItem(int item) {
		m_item = item;
		if (m_ptinp) {
			m_ptinp->setCurrentItem(item);
		}
	}

	/**
	 * Get current item.
	 * @return item.
	 */
	int getCurrentItem() const {
		return m_ptinp ? m_ptinp->currentItem() : m_item;
	}

 protected:
	/** Combo box widget */
	LabeledComboBox* m_ptinp;
	/** List of strings with possible selections */
	const char** m_strlst;
	/** Label */
	QString m_label;
	/** Selected item */
	int m_item;
};

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* TagLibIntComboBoxControl::createWidget(QWidget* parent)
{
	m_ptinp = new LabeledComboBox(parent, 0, m_strlst);
	if (m_ptinp) {
		m_ptinp->setLabel(m_label);
		m_ptinp->setCurrentItem(m_item);
	}
	return m_ptinp;
}


/** Control to import, export and view data from binary fields */
class TagLibBinFieldControl : public TagLibFieldControl {
 public:
	/**
	 * Constructor.
	 * @param label label
	 */
	explicit TagLibBinFieldControl(const QString& label, bool viewButton = false) :
	m_label(label), m_viewButton(viewButton) {}

	/**
	 * Destructor.
	 */
	virtual ~TagLibBinFieldControl() {}

	/**
	 * Create widget to edit field data.
	 *
	 * @param parent parent widget
	 *
	 * @return widget to edit field data.
	 */
	virtual QWidget* createWidget(QWidget* parent);

	/**
	 * Set binary data.
	 * @param data array of bytes
	 * @param size size of array
	 */
	void setBinaryData(const char* data, unsigned size) {
		m_byteArray.duplicate(data, size);
	}

	/**
	 * Get binary data.
	 * @return binary data.
	 */
	const QByteArray& getBinaryData() const { return m_byteArray; }

protected:
	/** Import, Export, View buttons */
	TagLibBinaryOpenSave* m_bos;
	/** Label */
	QString m_label;
	/** Binary data */
	QByteArray m_byteArray;
	/** true if View button shall be displayed */
	bool m_viewButton;
};

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* TagLibBinFieldControl::createWidget(QWidget* parent)
{
	m_bos = new TagLibBinaryOpenSave(m_byteArray, parent, 0, m_viewButton);
	if (m_bos) {
		m_bos->setLabel(m_label);
	}
	return m_bos;
}


/** Control to edit relative volume adjustment */
class TagLibRelativeVolumeControl : public TagLibFieldControl {
public:
	/**
	 * Constructor.
	 * @param label    label
	 * @param adjIndex adjustment index
	 * @param peakBits number of bits representing peak
	 * @param peakVol  peak volume hex string
	 * @param header   true to display header
	 */
	explicit TagLibRelativeVolumeControl(
		const QString& label, short adjIndex, unsigned char peakBits,
		const QByteArray& peakVol, bool header = false) :
	m_adjSpinBox(0), m_peakBitsSpinBox(0), m_peakVolEdit(0),
		m_label(label), m_adjIndex(adjIndex), m_peakBits(peakBits),
		m_peakVol(peakVol), m_header(header) {}

	/**
	 * Destructor.
	 */
	virtual ~TagLibRelativeVolumeControl() {}

	/**
	 * Create widget to edit field data.
	 *
	 * @param parent parent widget
	 *
	 * @return widget to edit field data.
	 */
	virtual QWidget* createWidget(QWidget* parent);

	/**
	 * Get adjustment index.
	 * @return adjustment index.
	 */
	short getAdjIndex() const {
		return m_adjSpinBox ? m_adjSpinBox->value() : m_adjIndex;
	}

	/**
	 * Get number of peak bits.
	 * @return number of peak bits.
	 */
	unsigned char getPeakBits() const {
		return m_peakBitsSpinBox ? m_peakBitsSpinBox->value() : m_peakBits;
	}

	/**
	 * Get peak volume.
	 * @return peak volume.
	 */
	const QByteArray& getPeakVolume();

private:
	/**
	 * Get peak volume as a hex string.
	 *
	 * @return hex string representing peak volume.
	 */
	QString peakVolAsString() const;

	QSpinBox* m_adjSpinBox;
	QSpinBox* m_peakBitsSpinBox;
	QLineEdit* m_peakVolEdit;
	QString m_label;
	short m_adjIndex;
	unsigned char m_peakBits;
	QByteArray m_peakVol;
	bool m_header;
};

/**
 * Get peak volume as a hex string.
 *
 * @return hex string representing peak volume.
 */
QString TagLibRelativeVolumeControl::peakVolAsString() const
{
	QString str;
	for (QByteArray::ConstIterator it = m_peakVol.begin();
			 it != m_peakVol.end();
			 ++it) {
		uint byteVal = *it & 0xff;
		str += QString::number(byteVal, 16);
	}
	return str;
}

/**
 * Get peak volume.
 * @return peak volume.
 */
const QByteArray& TagLibRelativeVolumeControl::getPeakVolume()
{
	if (m_peakVolEdit) {
		QString str = m_peakVolEdit->text();
		uint len = str.length();
		uint numBytes = (getPeakBits() + 7) / 8;
		m_peakVol.fill(0, numBytes);
		uint i, index;
		for (i = 0, index = 0; i < numBytes; ++i, index += 2) {
			if (len < index + 2) break;
			m_peakVol.at(i) = str.mid(index, 2).toUShort(0, 16);
		}
	}
	return m_peakVol;
}

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* TagLibRelativeVolumeControl::createWidget(QWidget* parent)
{
	QHBox* hbox = new QHBox(parent);
	hbox->setSpacing(6);
	QVBox* vbox0 = new QVBox(hbox);
	if (m_header)
		new QLabel(vbox0);
	QLabel* label = new QLabel(m_label, vbox0);
	QFontMetrics fm(label->fontMetrics());
	label->setFixedWidth(fm.width(i18n("Master volume")));
	QVBox* vbox1 = new QVBox(hbox);
	QString str1(i18n("Adjustment [dB/512]"));
	if (m_header)
		new QLabel(str1, vbox1);
	m_adjSpinBox = new QSpinBox(-32768, 32767, 1, vbox1);
	m_adjSpinBox->setValue(m_adjIndex);
	vbox1->setFixedWidth(fm.width(str1));
	QVBox* vbox2 = new QVBox(hbox);
	QString str2(i18n("Bits representing peak"));
	if (m_header)
		new QLabel(str2, vbox2);
	m_peakBitsSpinBox = new QSpinBox(0, 255, 1, vbox2);
	m_peakBitsSpinBox->setValue(m_peakBits);
	vbox2->setFixedWidth(fm.width(str2));
	QVBox* vbox3 = new QVBox(hbox);
	if (m_header)
		new QLabel(i18n("Peak volume [hex]"), vbox3);
	m_peakVolEdit = new QLineEdit(peakVolAsString(), vbox3);
	QRegExp rx("[A-Fa-f0-9]+");
	QValidator* validator = new QRegExpValidator(rx, this);
	m_peakVolEdit->setValidator(validator);
	return hbox;
}


/** Field edit dialog */
class TagLibEditFrameDialog : public QDialog {
public:
	/**
	 * Constructor.
	 *
	 * @param parent  parent widget
	 * @param caption window title
	 * @param ctls    list with controls to edit fields
	 */
	TagLibEditFrameDialog(QWidget* parent, const QString& caption,
										 QPtrList<FieldControl> &ctls);

protected:
	QVBoxLayout* m_vlayout;
	QHBoxLayout* m_hlayout;
	QSpacerItem* m_hspacer;
	QPushButton* m_okButton;
	QPushButton* m_cancelButton;
};

/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param caption window title
 * @param ctls    list with controls to edit fields
 */
TagLibEditFrameDialog::TagLibEditFrameDialog(
	QWidget* parent, const QString &caption, QPtrList<FieldControl> &ctls) :
	QDialog(parent, "edit_frame", true)
{
	setCaption(caption);
	m_vlayout = new QVBoxLayout(this);
	if (m_vlayout) {
		m_vlayout->setSpacing(6);
		m_vlayout->setMargin(6);
		FieldControl* fldCtl = ctls.first();
		while (fldCtl != 0) {
			m_vlayout->addWidget(fldCtl->createWidget(this));
			fldCtl = ctls.next();
		}
	}
	m_hlayout = new QHBoxLayout(m_vlayout);
	QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
																				 QSizePolicy::Minimum);
	m_okButton = new QPushButton(i18n("&OK"), this);
	m_cancelButton = new QPushButton(i18n("&Cancel"), this);
	if (m_hlayout && m_okButton && m_cancelButton) {
		m_hlayout->addItem(hspacer);
		m_hlayout->addWidget(m_okButton);
		m_hlayout->addWidget(m_cancelButton);
		m_okButton->setDefault(true);
		connect(m_okButton, SIGNAL(clicked()), this, SLOT(accept()));
		connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	}
	resize(fontMetrics().maxWidth() * 30, -1);
}


/**
 * Constructor.
 */
TagLibFrameList::TagLibFrameList() : m_tag(0)
{
	m_fieldcontrols.setAutoDelete(true);
}

/**
 * Destructor.
 */
TagLibFrameList::~TagLibFrameList()
{
}

/** Alphabetically sorted list of frame descriptions */
static const struct {
	char* str;
	bool supported;
} id3v2FrameIdTable[] = {
	{ I18N_NOOP("AENC - Audio encryption"), false },
	{ I18N_NOOP("APIC - Attached picture"), true },
	{ I18N_NOOP("ASPI - Audio seek point index"), false },
	{ I18N_NOOP("COMM - Comments"), true },
	{ I18N_NOOP("COMR - Commercial frame"), false },
	{ I18N_NOOP("ENCR - Encryption method registration"), false },
	{ I18N_NOOP("EQU2 - Equalisation (2)"), false },
	{ I18N_NOOP("ETCO - Event timing codes"), false },
	{ I18N_NOOP("GEOB - General encapsulated object"),
#ifdef TAGLIB_SUPPORTS_GEOB_FRAMES
		true
#else
		false
#endif
	},
	{ I18N_NOOP("GRID - Group identification registration"), false },
	{ I18N_NOOP("LINK - Linked information"), false },
	{ I18N_NOOP("MCDI - Music CD identifier"), false },
	{ I18N_NOOP("MLLT - MPEG location lookup table"), false },
	{ I18N_NOOP("OWNE - Ownership frame"), false },
	{ I18N_NOOP("PRIV - Private frame"), false },
	{ I18N_NOOP("PCNT - Play counter"), false },
	{ I18N_NOOP("POPM - Popularimeter"), false },
	{ I18N_NOOP("POSS - Position synchronisation frame"), false },
	{ I18N_NOOP("RBUF - Recommended buffer size"), false },
	{ I18N_NOOP("RVA2 - Relative volume adjustment (2)"), true },
	{ I18N_NOOP("RVRB - Reverb"), false },
	{ I18N_NOOP("SEEK - Seek frame"), false },
	{ I18N_NOOP("SIGN - Signature frame"), false },
	{ I18N_NOOP("SYLT - Synchronised lyric/text"), false },
	{ I18N_NOOP("SYTC - Synchronised tempo codes"), false },
	{ I18N_NOOP("TALB - Album/Movie/Show title"), true },
	{ I18N_NOOP("TBPM - BPM (beats per minute)"), true },
	{ I18N_NOOP("TCOM - Composer"), true },
	{ I18N_NOOP("TCON - Content type"), true },
	{ I18N_NOOP("TCOP - Copyright message"), true },
	{ I18N_NOOP("TDEN - Encoding time"), true },
	{ I18N_NOOP("TDLY - Playlist delay"), true },
	{ I18N_NOOP("TDOR - Original release time"), true },
	{ I18N_NOOP("TDRC - Recording time"), true },
	{ I18N_NOOP("TDRL - Release time"), true },
	{ I18N_NOOP("TDTG - Tagging time"), true },
	{ I18N_NOOP("TENC - Encoded by"), true },
	{ I18N_NOOP("TEXT - Lyricist/Text writer"), true },
	{ I18N_NOOP("TFLT - File type"), true },
	{ I18N_NOOP("TIPL - Involved people list"), true },
	{ I18N_NOOP("TIT1 - Content group description"), true },
	{ I18N_NOOP("TIT2 - Title/songname/content description"), true },
	{ I18N_NOOP("TIT3 - Subtitle/Description refinement"), true },
	{ I18N_NOOP("TKEY - Initial key"), true },
	{ I18N_NOOP("TLAN - Language(s)"), true },
	{ I18N_NOOP("TLEN - Length"), true },
	{ I18N_NOOP("TMCL - Musician credits list"), true },
	{ I18N_NOOP("TMED - Media type"), true },
	{ I18N_NOOP("TMOO - Mood"), true },
	{ I18N_NOOP("TOAL - Original album/movie/show title"), true },
	{ I18N_NOOP("TOFN - Original filename"), true },
	{ I18N_NOOP("TOLY - Original lyricist(s)/text writer(s)"), true },
	{ I18N_NOOP("TOPE - Original artist(s)/performer(s)"), true },
	{ I18N_NOOP("TOWN - File owner/licensee"), true },
	{ I18N_NOOP("TPE1 - Lead performer(s)/Soloist(s)"), true },
	{ I18N_NOOP("TPE2 - Band/orchestra/accompaniment"), true },
	{ I18N_NOOP("TPE3 - Conductor/performer refinement"), true },
	{ I18N_NOOP("TPE4 - Interpreted, remixed, or otherwise modified by"), true },
	{ I18N_NOOP("TPOS - Part of a set"), true },
	{ I18N_NOOP("TPRO - Produced notice"), true },
	{ I18N_NOOP("TPUB - Publisher"), true },
	{ I18N_NOOP("TRCK - Track number/Position in set"), true },
	{ I18N_NOOP("TRSN - Internet radio station name"), true },
	{ I18N_NOOP("TRSO - Internet radio station owner"), true },
	{ I18N_NOOP("TSOA - Album sort order"), true },
	{ I18N_NOOP("TSOP - Performer sort order"), true },
	{ I18N_NOOP("TSOT - Title sort order"), true },
	{ I18N_NOOP("TSRC - ISRC (international standard recording code)"), true },
	{ I18N_NOOP("TSSE - Software/Hardware and settings used for encoding"), true },
	{ I18N_NOOP("TSST - Set subtitle"), true },
	{ I18N_NOOP("TXXX - User defined text information frame"), true },
	{ I18N_NOOP("UFID - Unique file identifier"), true },
	{ I18N_NOOP("USER - Terms of use"), false },
	{ I18N_NOOP("USLT - Unsynchronised lyric/text transcription"), true },
	{ I18N_NOOP("WCOM - Commercial information"), true },
	{ I18N_NOOP("WCOP - Copyright/Legal information"), true },
	{ I18N_NOOP("WOAF - Official audio file webpage"), true },
	{ I18N_NOOP("WOAR - Official artist/performer webpage"), true },
	{ I18N_NOOP("WOAS - Official audio source webpage"), true },
	{ I18N_NOOP("WORS - Official Internet radio station homepage"), true },
	{ I18N_NOOP("WPAY - Payment"), true },
	{ I18N_NOOP("WPUB - Publishers official webpage"), true },
	{ I18N_NOOP("WXXX - User defined URL link frame"), true }
};

/**
 * Get a description of an ID3v2 frame.
 *
 * @param frameId ID3v2 frame ID
 *
 * @return description.
 */
QString TagLibFrameList::getId3v2FrameDescription(
	TagLib::ByteVector frameId) const
{
	static TagLib::Map<TagLib::ByteVector, unsigned> idIndexMap;
	if (idIndexMap.isEmpty()) {
		for (unsigned i = 0;
				 i < sizeof(id3v2FrameIdTable) / sizeof(id3v2FrameIdTable[0]);
				 ++i) {
			idIndexMap.insert(TagLib::ByteVector(id3v2FrameIdTable[i].str, 4), i);
		}
	}
	return idIndexMap.contains(frameId) ?
		i18n(id3v2FrameIdTable[idIndexMap[frameId]].str) :
		TStringToQString(TagLib::String(frameId));
}

/**
 * Fill listbox with frame descriptions.
 * Before using this method, the listbox and file have to be set.
 * @see setListBox(), setTags()
 */
void TagLibFrameList::readTags()
{
	listbox->clear();
	if (m_tag) {
		TagLib::ID3v2::Tag* id3v2Tag;
		TagLib::Ogg::XiphComment* oggTag;
		TagLib::APE::Tag* apeTag;
		if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tag)) != 0) {
			const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
			for (TagLib::ID3v2::FrameList::ConstIterator it = frameList.begin();
					 it != frameList.end();
					 ++it) {
				listbox->insertItem(getId3v2FrameDescription((*it)->frameID()));
			}
		} else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tag)) != 0) {
			const TagLib::Ogg::FieldListMap& fieldListMap = oggTag->fieldListMap();
			for (TagLib::Ogg::FieldListMap::ConstIterator it = fieldListMap.begin();
					 it != fieldListMap.end();
					 ++it) {
				QString id = TStringToQString((*it).first);
				TagLib::StringList stringList = (*it).second;
				for (TagLib::StringList::ConstIterator slit = stringList.begin();
						 slit != stringList.end();
						 ++slit) {
					listbox->insertItem(id);
				}
			}
		} else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tag)) != 0) {
			const TagLib::APE::ItemListMap& itemListMap = apeTag->itemListMap();
			for (TagLib::APE::ItemListMap::ConstIterator it = itemListMap.begin();
					 it != itemListMap.end();
					 ++it) {
				listbox->insertItem(TStringToQString((*it).first));
			}
		}
	}
}

/**
 * Set file and fill the list box with its frames.
 * The listbox has to be set with setListBox() before calling this
 * function.
 *
 * @param taggedFile file
 */
void TagLibFrameList::setTags(TaggedFile* taggedFile)
{
	m_file = taggedFile;
	TagLibFile* tagLibFile = dynamic_cast<TagLibFile*>(m_file);
	if (tagLibFile && tagLibFile->isTagInformationRead()) {
		m_tag = tagLibFile->m_tagV2;
		readTags();
	}
}

/**
 * Get a Xiph comment field by index.
 *
 * @param oggTag Xiph comment
 * @param index  index of field
 * @param key    the key is returned here
 * @param value  the value is returned here
 *
 * @return true if found.
 */
bool getXiphCommentField(
	const TagLib::Ogg::XiphComment& oggTag,
	int index, TagLib::String& key, TagLib::String& value)
{
	const TagLib::Ogg::FieldListMap& fieldListMap = oggTag.fieldListMap();
	int i = 0;
	for (TagLib::Ogg::FieldListMap::ConstIterator it = fieldListMap.begin();
			 it != fieldListMap.end();
			 ++it) {
		TagLib::StringList stringList = (*it).second;
		for (TagLib::StringList::ConstIterator slit = stringList.begin();
				 slit != stringList.end();
				 ++slit) {
			if (i == index) {
				key = (*it).first;
				value = *slit;
				return true;
			}
			++i;
		}
	}
	return false;
}

/**
 * Get an APE item by index.
 *
 * @param apeTag APE tag
 * @param index  index of field
 * @param key    the key is returned here
 * @param item   the item is returned here
 *
 * @return true if found.
 */
bool getApeItem(
	const TagLib::APE::Tag& apeTag,
	int index, TagLib::String& key,
	TagLib::APE::Item& item)
{
	const TagLib::APE::ItemListMap& itemListMap = apeTag.itemListMap();
	int i = 0;
	for (TagLib::APE::ItemListMap::ConstIterator it = itemListMap.begin();
			 it != itemListMap.end();
			 ++it) {
		if (i == index) {
			key = (*it).first;
			item = (*it).second;
			return true;
		}
		++i;
	}
	return false;
}

/**
 * Create dialog to edit a field with key and value and update the fields
 * if Ok selected.
 *
 * @param key   key of field
 * @param value value, will be changed if true is returned
 *
 * @return true if Ok selected in dialog.
 */
bool TagLibFrameList::editKeyValueField(
	const TagLib::String& key, TagLib::String& value)
{
	bool result = false;
	m_fieldcontrols.clear();
	TagLibTextFieldControl* textCtl = new TagLibTextFieldControl("", TStringToQString(value));
	if (textCtl) {
		m_fieldcontrols.append(textCtl);
	}
	TagLibEditFrameDialog* dialog =
		new TagLibEditFrameDialog(0, TStringToQString(key), m_fieldcontrols);
	if (dialog && dialog->exec() == QDialog::Accepted) {
		value = QStringToTString(textCtl->getText());
		result = true;
	}
	m_fieldcontrols.clear();
	return result;
}

/**
 * Copy an ID3v2 frame.
 *
 * @param frame original frame
 *
 * @return new frame.
 */
TagLib::ID3v2::Frame* TagLibFrameList::copyId3v2Frame(
	const TagLib::ID3v2::Frame* frame) const
{
	// Setting a version other than the default 4 makes not much sense as
	// TagLib always writes ID3v2.4.0 tags.
	return TagLib::ID3v2::FrameFactory::instance()->createFrame(frame->render());
}

/**
 * Convert a string to a language code byte vector.
 *
 * @param str string containing language code.
 *
 * @return 3 byte vector with language code.
 */
static TagLib::ByteVector languageCodeByteVector(QString str)
{
	uint len = str.length();
	if (len > 3) {
		str.truncate(3);
	} else if (len < 3) {
		for (uint i = len; i < 3; ++i) {
			str += ' ';
		}
	}
	return TagLib::ByteVector(str.latin1(), str.length());
}

static const char* encodingStrLst[] = {
	I18N_NOOP("ISO-8859-1"),
	I18N_NOOP("UTF16"),
	I18N_NOOP("UTF16BE"),
	I18N_NOOP("UTF8"),
	I18N_NOOP("UTF16LE"),
	0
};

/**
 * Edit a text identificatin frame.
 *
 * @param tFrame text identification frame
 * @param id     frame ID
 *
 * @return new edited frame.
 */
TagLib::ID3v2::Frame* TagLibFrameList::editTextFrame(
	const TagLib::ID3v2::TextIdentificationFrame* tFrame, const QString& id)
{
	TagLib::ID3v2::Frame* newFrame = 0;
	const TagLib::ID3v2::UserTextIdentificationFrame* txxxFrame;
	TagLibIntComboBoxControl* encCtl =
		new TagLibIntComboBoxControl(encodingStrLst, i18n("Text Encoding"),
													 tFrame->textEncoding());
	if (encCtl) {
		m_fieldcontrols.append(encCtl);
	}
	TagLibLineFieldControl* descCtl = 0;
	TagLibTextFieldControl* textCtl = 0;
	if ((txxxFrame =
			 dynamic_cast<const TagLib::ID3v2::UserTextIdentificationFrame*>(tFrame))
			!= 0) {
		descCtl = new TagLibLineFieldControl(
			i18n("Description"), TStringToQString(txxxFrame->description()));
		if (descCtl) {
			m_fieldcontrols.append(descCtl);
		}
		TagLib::StringList slText = tFrame->fieldList();
		textCtl = new TagLibTextFieldControl(i18n("Text"), slText.size() > 1 ?
																	 TStringToQString(slText[1]) : "");
	} else {
		textCtl = new TagLibTextFieldControl(i18n("Text"),
																	 TStringToQString(tFrame->toString()));
	}
	if (textCtl) {
		m_fieldcontrols.append(textCtl);
	}
	TagLibEditFrameDialog* dialog =
		new TagLibEditFrameDialog(0, id, m_fieldcontrols);
	if (dialog && dialog->exec() == QDialog::Accepted) {
		newFrame = copyId3v2Frame(tFrame);
		TagLib::ID3v2::TextIdentificationFrame* newTFrame;
		if (newFrame &&
				(newTFrame =
				 dynamic_cast<TagLib::ID3v2::TextIdentificationFrame*>(newFrame)) !=
				0) {
			newTFrame->setTextEncoding(
				static_cast<TagLib::String::Type>(encCtl->getCurrentItem()));
			TagLib::ID3v2::UserTextIdentificationFrame* newTxxxFrame =
				dynamic_cast<TagLib::ID3v2::UserTextIdentificationFrame*>(newFrame);
			if (newTxxxFrame && descCtl) {
				newTxxxFrame->setDescription(QStringToTString(descCtl->getText()));
			}
			newTFrame->setText(QStringToTString(textCtl->getText()));
		}
	}
	return newFrame;
}

/**
 * Edit an attached picture frame.
 *
 * @param apicFrame attached picture frame
 * @param id        frame ID
 *
 * @return new edited frame.
 */
TagLib::ID3v2::Frame* TagLibFrameList::editApicFrame(
	const TagLib::ID3v2::AttachedPictureFrame* apicFrame, const QString& id)
{
	TagLib::ID3v2::Frame* newFrame = 0;
	static const char* pictureTypeStrLst[] = {
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
		0
	};
	TagLibIntComboBoxControl* encCtl =
		new TagLibIntComboBoxControl(encodingStrLst, i18n("Text Encoding"),
													 apicFrame->textEncoding());
	if (encCtl) {
		m_fieldcontrols.append(encCtl);
	}
	TagLibLineFieldControl* mimeCtl = new TagLibLineFieldControl(
		i18n("Mimetype"), TStringToQString(apicFrame->mimeType()));
	if (mimeCtl) {
		m_fieldcontrols.append(mimeCtl);
	}
	TagLibIntComboBoxControl* typeCtl =
		new TagLibIntComboBoxControl(pictureTypeStrLst, i18n("Picture Type"),
													 apicFrame->type());
	if (typeCtl) {
		m_fieldcontrols.append(typeCtl);
	}
	TagLibLineFieldControl* descCtl = new TagLibLineFieldControl(
		i18n("Description"), TStringToQString(apicFrame->description()));
	if (descCtl) {
		m_fieldcontrols.append(descCtl);
	}
	TagLibBinFieldControl* dataCtl = new TagLibBinFieldControl(i18n("Picture"), true);
	if (dataCtl) {
		TagLib::ByteVector pic = apicFrame->picture();
		dataCtl->setBinaryData(pic.data(), pic.size());
		m_fieldcontrols.append(dataCtl);
	}
	TagLibEditFrameDialog* dialog =
		new TagLibEditFrameDialog(0, id, m_fieldcontrols);
	if (dialog && dialog->exec() == QDialog::Accepted) {
		newFrame = copyId3v2Frame(apicFrame);
		TagLib::ID3v2::AttachedPictureFrame* newApicFrame;
		if (newFrame &&
				(newApicFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(
					newFrame)) != 0) {
			newApicFrame->setTextEncoding(
				static_cast<TagLib::String::Type>(encCtl->getCurrentItem()));
			newApicFrame->setMimeType(QStringToTString(mimeCtl->getText()));
			newApicFrame->setType(
				static_cast<TagLib::ID3v2::AttachedPictureFrame::Type>(
					typeCtl->getCurrentItem()));
			newApicFrame->setDescription(QStringToTString(descCtl->getText()));
			QByteArray baPicture = dataCtl->getBinaryData();
			newApicFrame->setPicture(
				TagLib::ByteVector(baPicture.data(), baPicture.size()));
		}
	}
	return newFrame;
}

/**
 * Edit a comments frame.
 *
 * @param commFrame comments frame
 * @param id        frame ID
 *
 * @return new edited frame.
 */
TagLib::ID3v2::Frame* TagLibFrameList::editCommFrame(
	const TagLib::ID3v2::CommentsFrame* commFrame, const QString& id)
{
	TagLib::ID3v2::Frame* newFrame = 0;
	TagLibIntComboBoxControl* encCtl =
		new TagLibIntComboBoxControl(
			encodingStrLst, i18n("Text Encoding"), commFrame->textEncoding());
	if (encCtl) {
		m_fieldcontrols.append(encCtl);
	}
	TagLib::ByteVector bvLang = commFrame->language();
	TagLibLineFieldControl* langCtl = new TagLibLineFieldControl(
		i18n("Language"), QCString(bvLang.data(), bvLang.size() + 1));
	if (langCtl) {
		m_fieldcontrols.append(langCtl);
	}
	TagLibLineFieldControl* descCtl = new TagLibLineFieldControl(
		i18n("Description"), TStringToQString(commFrame->description()));
	if (descCtl) {
		m_fieldcontrols.append(descCtl);
	}
	TagLibTextFieldControl* textCtl = new TagLibTextFieldControl(
		i18n("Text"), TStringToQString(commFrame->toString()));
	if (textCtl) {
		m_fieldcontrols.append(textCtl);
	}
	TagLibEditFrameDialog* dialog =
		new TagLibEditFrameDialog(0, id, m_fieldcontrols);
	if (dialog && dialog->exec() == QDialog::Accepted) {
		newFrame = copyId3v2Frame(commFrame);
		TagLib::ID3v2::CommentsFrame* newCommFrame;
		if (newFrame &&
				(newCommFrame = dynamic_cast<TagLib::ID3v2::CommentsFrame*>(
					newFrame)) != 0) {
			newCommFrame->setTextEncoding(
				static_cast<TagLib::String::Type>(encCtl->getCurrentItem()));
			newCommFrame->setLanguage(languageCodeByteVector(langCtl->getText()));
			newCommFrame->setDescription(QStringToTString(descCtl->getText()));
			newCommFrame->setText(QStringToTString(textCtl->getText()));
		}
	}
	return newFrame;
}

/**
 * Edit a relative volume frame.
 *
 * @param rva2Frame relative volume frame
 * @param id        frame ID
 *
 * @return new edited frame.
 */
TagLib::ID3v2::Frame* TagLibFrameList::editRva2Frame(
	const TagLib::ID3v2::RelativeVolumeFrame* rva2Frame, const QString& id)
{
	TagLib::ID3v2::Frame* newFrame = 0;
	static const unsigned maxNumChannels = 9;
	static const char* const channelTypeStrings[maxNumChannels] = {
		I18N_NOOP("Other"),
		I18N_NOOP("Master volume"),
		I18N_NOOP("Front right"),
		I18N_NOOP("Front left"),
		I18N_NOOP("Back right"),
		I18N_NOOP("Back left"),
		I18N_NOOP("Front centre"),
		I18N_NOOP("Back centre"),
		I18N_NOOP("Subwoofer")
	};
	TagLibRelativeVolumeControl* rvCtl[maxNumChannels];
	for (unsigned i = 0; i < maxNumChannels; ++i) {
		TagLib::ID3v2::RelativeVolumeFrame::ChannelType channelType =
			static_cast<TagLib::ID3v2::RelativeVolumeFrame::ChannelType>(i);
		QString channelName(i18n(channelTypeStrings[i]));
		TagLib::ID3v2::RelativeVolumeFrame::PeakVolume pv =
			rva2Frame->peakVolume(channelType);
		QByteArray baPeakVolume;
		baPeakVolume.duplicate(pv.peakVolume.data(), pv.peakVolume.size());
		rvCtl[i] =
			new TagLibRelativeVolumeControl(
				channelName, rva2Frame->volumeAdjustmentIndex(channelType),
				pv.bitsRepresentingPeak, baPeakVolume, i == 0);
		if (rvCtl[i]) {
			m_fieldcontrols.append(rvCtl[i]);
		}
	}
	TagLibEditFrameDialog* dialog =
		new TagLibEditFrameDialog(0, id, m_fieldcontrols);
	if (dialog && dialog->exec() == QDialog::Accepted) {
		newFrame = copyId3v2Frame(rva2Frame);
		TagLib::ID3v2::RelativeVolumeFrame* newRva2Frame;
		if (newFrame &&
				(newRva2Frame = dynamic_cast<TagLib::ID3v2::RelativeVolumeFrame*>(
					newFrame)) != 0) {
			for (unsigned i = 0; i < maxNumChannels; ++i) {
				TagLib::ID3v2::RelativeVolumeFrame::ChannelType channelType =
					static_cast<TagLib::ID3v2::RelativeVolumeFrame::ChannelType>(i);
				if (rvCtl[i] &&
						(rvCtl[i]->getPeakBits() != 0 || rvCtl[i]->getAdjIndex() != 0)) {
					TagLib::ID3v2::RelativeVolumeFrame::PeakVolume pv;
					pv.bitsRepresentingPeak = rvCtl[i]->getPeakBits();
					const QByteArray& baPeak = rvCtl[i]->getPeakVolume();
					pv.peakVolume.setData(baPeak.data(), baPeak.size());
					newRva2Frame->setPeakVolume(pv, channelType);
					newRva2Frame->setVolumeAdjustmentIndex(
						rvCtl[i]->getAdjIndex(), channelType);
				}
			}
		}
	}
	return newFrame;
}

/**
 * Edit a unique file identifier frame.
 *
 * @param ufidFrame unique file identifier frame
 * @param id        frame ID
 *
 * @return new edited frame.
 */
TagLib::ID3v2::Frame* TagLibFrameList::editUfidFrame(
	const TagLib::ID3v2::UniqueFileIdentifierFrame* ufidFrame, const QString& id)
{
	TagLib::ID3v2::Frame* newFrame = 0;
	TagLibLineFieldControl* ownerCtl = new TagLibLineFieldControl(
		i18n("Owner"), TStringToQString(ufidFrame->owner()));
	if (ownerCtl) {
		m_fieldcontrols.append(ownerCtl);
	}
	TagLibBinFieldControl* dataCtl = new TagLibBinFieldControl(i18n("Identifier"));
	if (dataCtl) {
		TagLib::ByteVector id = ufidFrame->identifier();
		dataCtl->setBinaryData(id.data(), id.size());
		m_fieldcontrols.append(dataCtl);
	}
	TagLibEditFrameDialog* dialog =
		new TagLibEditFrameDialog(0, id, m_fieldcontrols);
	if (dialog && dialog->exec() == QDialog::Accepted) {
		newFrame = copyId3v2Frame(ufidFrame);
		TagLib::ID3v2::UniqueFileIdentifierFrame* newUfidFrame;
		if (newFrame &&
				(newUfidFrame =
				 dynamic_cast<TagLib::ID3v2::UniqueFileIdentifierFrame*>(newFrame)) !=
				0) {
			newUfidFrame->setOwner(QStringToTString(ownerCtl->getText()));
			QByteArray id = dataCtl->getBinaryData();
			newUfidFrame->setIdentifier(TagLib::ByteVector(id.data(), id.size()));
		}
	}
	return newFrame;
}

#ifdef TAGLIB_SUPPORTS_GEOB_FRAMES
/**
 * Edit a general encapsulated object frame.
 *
 * @param geobFrame general encapsulated object frame
 * @param id        frame ID
 *
 * @return new edited frame.
 */
TagLib::ID3v2::Frame* TagLibFrameList::editGeobFrame(
	const TagLib::ID3v2::GeneralEncapsulatedObjectFrame* geobFrame, const QString& id)
{
	TagLib::ID3v2::Frame* newFrame = 0;
	TagLibIntComboBoxControl* encCtl =
		new TagLibIntComboBoxControl(encodingStrLst, i18n("Text Encoding"),
																 geobFrame->textEncoding());
	if (encCtl) {
		m_fieldcontrols.append(encCtl);
	}
	TagLibLineFieldControl* mimeCtl = new TagLibLineFieldControl(
		i18n("Mimetype"), TStringToQString(geobFrame->mimeType()));
	if (mimeCtl) {
		m_fieldcontrols.append(mimeCtl);
	}
	TagLibLineFieldControl* fnCtl = new TagLibLineFieldControl(
		i18n("Filename"), TStringToQString(geobFrame->fileName()));
	if (fnCtl) {
		m_fieldcontrols.append(fnCtl);
	}
	TagLibLineFieldControl* descCtl = new TagLibLineFieldControl(
		i18n("Description"), TStringToQString(geobFrame->description()));
	if (descCtl) {
		m_fieldcontrols.append(descCtl);
	}
	TagLibBinFieldControl* dataCtl = new TagLibBinFieldControl(i18n("Data"));
	if (dataCtl) {
		TagLib::ByteVector obj = geobFrame->object();
		dataCtl->setBinaryData(obj.data(), obj.size());
		m_fieldcontrols.append(dataCtl);
	}
	TagLibEditFrameDialog* dialog =
		new TagLibEditFrameDialog(0, id, m_fieldcontrols);
	if (dialog && dialog->exec() == QDialog::Accepted) {
		newFrame = copyId3v2Frame(geobFrame);
		TagLib::ID3v2::GeneralEncapsulatedObjectFrame* newGeobFrame;
		if (newFrame &&
				(newGeobFrame = dynamic_cast<TagLib::ID3v2::GeneralEncapsulatedObjectFrame*>(
					newFrame)) != 0) {
			newGeobFrame->setTextEncoding(
				static_cast<TagLib::String::Type>(encCtl->getCurrentItem()));
			newGeobFrame->setMimeType(QStringToTString(mimeCtl->getText()));
			newGeobFrame->setFileName(QStringToTString(fnCtl->getText()));
			newGeobFrame->setDescription(QStringToTString(descCtl->getText()));
			QByteArray baObj = dataCtl->getBinaryData();
			newGeobFrame->setObject(
				TagLib::ByteVector(baObj.data(), baObj.size()));
		}
	}
	return newFrame;
}
#endif // TAGLIB_SUPPORTS_GEOB_FRAMES

/**
 * Edit a URL link frame.
 *
 * @param wFrame URL link frame
 * @param id     frame ID
 *
 * @return new edited frame.
 */
TagLib::ID3v2::Frame* TagLibFrameList::editUrlFrame(
#ifdef TAGLIB_SUPPORTS_URLLINK_FRAMES
	const TagLib::ID3v2::UrlLinkFrame* wFrame
#else
	const UrlLinkFrame* wFrame
#endif
	, const QString& id)
{
	TagLib::ID3v2::Frame* newFrame = 0;
	TagLibLineFieldControl* textCtl =
		new TagLibLineFieldControl(i18n("URL"),
												 TStringToQString(wFrame->url()));
	if (textCtl) {
		m_fieldcontrols.append(textCtl);
	}
	TagLibEditFrameDialog* dialog =
		new TagLibEditFrameDialog(0, id, m_fieldcontrols);
	if (dialog && dialog->exec() == QDialog::Accepted) {
#ifdef TAGLIB_SUPPORTS_URLLINK_FRAMES
		newFrame = copyId3v2Frame(wFrame);
		TagLib::ID3v2::UrlLinkFrame* newWFrame;
		if (newFrame &&
				(newWFrame = dynamic_cast<TagLib::ID3v2::UrlLinkFrame*>(
					newFrame)) != 0) {
			if (textCtl) {
				newWFrame->setUrl(QStringToTString(textCtl->getText()));
			}
		}
#else
		// Because UrlLinkFrame is not known to the
		// TagLib frame factory, we have to create a new frame, change it
		// and then create an UnknownFrame copy using copyId3v2Frame().
		UrlLinkFrame* newWFrame = new UrlLinkFrame(wFrame->render());
		if (newWFrame) {
			if (textCtl) {
				newWFrame->setUrl(QStringToTString(textCtl->getText()));
			}
			newFrame = copyId3v2Frame(newWFrame);
			delete newWFrame;
		}
#endif
	}
	return newFrame;
}

/**
 * Edit a user URL link frame.
 *
 * @param wxxxFrame user URL link frame
 * @param id        frame ID
 *
 * @return new edited frame.
 */
TagLib::ID3v2::Frame* TagLibFrameList::editUserUrlFrame(
#ifdef TAGLIB_SUPPORTS_URLLINK_FRAMES
	const TagLib::ID3v2::UserUrlLinkFrame* wxxxFrame
#else
	const UserUrlLinkFrame* wxxxFrame
#endif
	, const QString& id)
{
	TagLib::ID3v2::Frame* newFrame = 0;
	TagLibIntComboBoxControl* encCtl =
		new TagLibIntComboBoxControl(encodingStrLst, i18n("Text Encoding"),
													 wxxxFrame->textEncoding());
	if (encCtl) {
		m_fieldcontrols.append(encCtl);
	}
	TagLibLineFieldControl* descCtl = new TagLibLineFieldControl(
		i18n("Description"), TStringToQString(wxxxFrame->description()));
	if (descCtl) {
		m_fieldcontrols.append(descCtl);
	}
	TagLibLineFieldControl* textCtl =
		new TagLibLineFieldControl(i18n("URL"),
												 TStringToQString(wxxxFrame->url()));
	if (textCtl) {
		m_fieldcontrols.append(textCtl);
	}
	TagLibEditFrameDialog* dialog =
		new TagLibEditFrameDialog(0, id, m_fieldcontrols);
	if (dialog && dialog->exec() == QDialog::Accepted) {
#ifdef TAGLIB_SUPPORTS_URLLINK_FRAMES
		newFrame = copyId3v2Frame(wxxxFrame);
		TagLib::ID3v2::UserUrlLinkFrame* newWxxxFrame;
		if (newFrame &&
				(newWxxxFrame = dynamic_cast<TagLib::ID3v2::UserUrlLinkFrame*>(
					newFrame)) != 0) {
			if (encCtl) {				
				newWxxxFrame->setTextEncoding(
					static_cast<TagLib::String::Type>(encCtl->getCurrentItem()));
			}
			if (descCtl) {
				newWxxxFrame->setDescription(QStringToTString(descCtl->getText()));
			}
			if (textCtl) {
				newWxxxFrame->setUrl(QStringToTString(textCtl->getText()));
			}
		}
#else
		// Because UserUrlLinkFrame is not known  to the
		// TagLib frame factory, we have to create a new frame, change it
		// and then create an UnknownFrame copy using copyId3v2Frame().
		UserUrlLinkFrame* newWxxxFrame= new UserUrlLinkFrame(wxxxFrame->render());
		if (newWxxxFrame) {
			if (encCtl) {				
				newWxxxFrame->setTextEncoding(
					static_cast<TagLib::String::Type>(encCtl->getCurrentItem()));
			}
			if (descCtl) {
				newWxxxFrame->setDescription(QStringToTString(descCtl->getText()));
			}
			if (textCtl) {
				newWxxxFrame->setUrl(QStringToTString(textCtl->getText()));
			}
			newFrame = copyId3v2Frame(newWxxxFrame);
			delete newWxxxFrame;
		}
#endif
	}
	return newFrame;
}

/**
 * Edit an unsynchronized lyrics frame.
 * This is copy-pasted from editCommFrame().
 *
 * @param usltFrame unsynchronized frame
 * @param id        frame ID
 *
 * @return new edited frame.
 */
TagLib::ID3v2::Frame* TagLibFrameList::editUsltFrame(
#ifdef TAGLIB_SUPPORTS_USLT_FRAMES
	const TagLib::ID3v2::UnsynchronizedLyricsFrame* usltFrame
#else
	const UnsynchronizedLyricsFrame* usltFrame
#endif
	, const QString& id)
{
	TagLib::ID3v2::Frame* newFrame = 0;
	TagLibIntComboBoxControl* encCtl =
		new TagLibIntComboBoxControl(
			encodingStrLst, i18n("Text Encoding"), usltFrame->textEncoding());
	if (encCtl) {
		m_fieldcontrols.append(encCtl);
	}
	TagLib::ByteVector bvLang = usltFrame->language();
	TagLibLineFieldControl* langCtl = new TagLibLineFieldControl(
		i18n("Language"), QCString(bvLang.data(), bvLang.size() + 1));
	if (langCtl) {
		m_fieldcontrols.append(langCtl);
	}
	TagLibLineFieldControl* descCtl = new TagLibLineFieldControl(
		i18n("Description"), TStringToQString(usltFrame->description()));
	if (descCtl) {
		m_fieldcontrols.append(descCtl);
	}
	TagLibTextFieldControl* textCtl = new TagLibTextFieldControl(
		i18n("Text"), TStringToQString(usltFrame->toString()));
	if (textCtl) {
		m_fieldcontrols.append(textCtl);
	}
	TagLibEditFrameDialog* dialog =
		new TagLibEditFrameDialog(0, id, m_fieldcontrols);
	if (dialog && dialog->exec() == QDialog::Accepted) {
#ifdef TAGLIB_SUPPORTS_USLT_FRAMES
		newFrame = copyId3v2Frame(usltFrame);
		TagLib::ID3v2::UnsynchronizedLyricsFrame* newUsltFrame;
		if (newFrame &&
				(newUsltFrame = dynamic_cast<TagLib::ID3v2::UnsynchronizedLyricsFrame*>(
					newFrame)) != 0) {
			if (encCtl) {
				newUsltFrame->setTextEncoding(
					static_cast<TagLib::String::Type>(encCtl->getCurrentItem()));
			}
			if (langCtl) {
				newUsltFrame->setLanguage(languageCodeByteVector(langCtl->getText()));
			}
			if (descCtl) {
				newUsltFrame->setDescription(QStringToTString(descCtl->getText()));
			}
			if (textCtl) {
				newUsltFrame->setText(QStringToTString(textCtl->getText()));
			}
		}
#else
		// Because UnsynchronizedLyricsFrame is not known to the
		// TagLib frame factory, we have to create a new frame, change it
		// and then create an UnknownFrame copy using copyId3v2Frame().
		UnsynchronizedLyricsFrame* newUsltFrame = new UnsynchronizedLyricsFrame(usltFrame->render());
		if (newUsltFrame) {
			if (encCtl) {
				newUsltFrame->setTextEncoding(
					static_cast<TagLib::String::Type>(encCtl->getCurrentItem()));
			}
			if (langCtl) {
				newUsltFrame->setLanguage(languageCodeByteVector(langCtl->getText()));
			}
			if (descCtl) {
				newUsltFrame->setDescription(QStringToTString(descCtl->getText()));
			}
			if (textCtl) {
				newUsltFrame->setText(QStringToTString(textCtl->getText()));
			}
			newFrame = copyId3v2Frame(newUsltFrame);
			delete newUsltFrame;
		}
#endif
	}
	return newFrame;
}

/**
 * Edit an unknown frame.
 *
 * @param unknownFrame unknown frame
 * @param id           frame ID
 *
 * @return new edited frame.
 */
TagLib::ID3v2::Frame* TagLibFrameList::editUnknownFrame(
	const TagLib::ID3v2::Frame* unknownFrame, const QString& id)
{
	TagLib::ID3v2::Frame* newFrame = 0;
	TagLibBinFieldControl* dataCtl = new TagLibBinFieldControl(i18n("Data"));
	if (dataCtl) {
		TagLib::ByteVector dat = unknownFrame->render();
		dataCtl->setBinaryData(dat.data(), dat.size());
		m_fieldcontrols.append(dataCtl);
	}
	TagLibEditFrameDialog* dialog =
		new TagLibEditFrameDialog(0, id, m_fieldcontrols);
	if (dialog && dialog->exec() == QDialog::Accepted) {
		newFrame = copyId3v2Frame(unknownFrame);
		if (newFrame) {
			QByteArray dat = dataCtl->getBinaryData();
			newFrame->setData(TagLib::ByteVector(dat.data(), dat.size()));
		}
	}
	return newFrame;
}

/**
 * Create dialog to edit an ID3v2 frame and return a modified copy
 * if Ok is selected.
 *
 * @param frame frame to edit
 *
 * @return a new frame if Ok selected in dialog, else 0.
 */
TagLib::ID3v2::Frame* TagLibFrameList::editId3v2Frame(
	const TagLib::ID3v2::Frame* frame)
{
	TagLib::ID3v2::Frame* newFrame = 0;
	QString id(getId3v2FrameDescription(frame->frameID()));
	m_fieldcontrols.clear();

	const TagLib::ID3v2::TextIdentificationFrame* tFrame;
	const TagLib::ID3v2::AttachedPictureFrame* apicFrame;
	const TagLib::ID3v2::CommentsFrame* commFrame;
	const TagLib::ID3v2::RelativeVolumeFrame* rva2Frame;
	const TagLib::ID3v2::UniqueFileIdentifierFrame* ufidFrame;
#ifdef TAGLIB_SUPPORTS_GEOB_FRAMES
	const TagLib::ID3v2::GeneralEncapsulatedObjectFrame* geobFrame;
#endif // TAGLIB_SUPPORTS_GEOB_FRAMES
#ifdef TAGLIB_SUPPORTS_URLLINK_FRAMES
	const TagLib::ID3v2::UserUrlLinkFrame* wxxxFrame;
	const TagLib::ID3v2::UrlLinkFrame* wFrame;
#else
	const UserUrlLinkFrame* wxxxFrame;
	const UrlLinkFrame* wFrame;
#endif
#ifdef TAGLIB_SUPPORTS_USLT_FRAMES
	const TagLib::ID3v2::UnsynchronizedLyricsFrame* usltFrame;
#else
	const UnsynchronizedLyricsFrame* usltFrame;
#endif
	if ((tFrame =
			 dynamic_cast<const TagLib::ID3v2::TextIdentificationFrame*>(frame)) !=
			0) {
		newFrame = editTextFrame(tFrame, id);
	} else if ((apicFrame =
							dynamic_cast<const TagLib::ID3v2::AttachedPictureFrame*>(frame))
						 != 0) {
		newFrame = editApicFrame(apicFrame, id);
	} else if ((commFrame = dynamic_cast<const TagLib::ID3v2::CommentsFrame*>(
								frame)) != 0) {
		newFrame = editCommFrame(commFrame, id);
	} else if ((rva2Frame =
							dynamic_cast<const TagLib::ID3v2::RelativeVolumeFrame*>(frame))
						 != 0) {
		newFrame = editRva2Frame(rva2Frame, id);
	} else if ((ufidFrame =
							dynamic_cast<const TagLib::ID3v2::UniqueFileIdentifierFrame*>(
								frame)) != 0) {
		newFrame = editUfidFrame(ufidFrame, id);
	}
#ifdef TAGLIB_SUPPORTS_GEOB_FRAMES
	else if ((geobFrame =
							dynamic_cast<const TagLib::ID3v2::GeneralEncapsulatedObjectFrame*>(
								frame)) != 0) {
		newFrame = editGeobFrame(geobFrame, id);
	}
#endif // TAGLIB_SUPPORTS_GEOB_FRAMES
#ifdef TAGLIB_SUPPORTS_URLLINK_FRAMES
	else if ((wxxxFrame = dynamic_cast<const TagLib::ID3v2::UserUrlLinkFrame*>(frame)) != 0) {
		newFrame = editUserUrlFrame(wxxxFrame, id);
	} else if ((wFrame = dynamic_cast<const TagLib::ID3v2::UrlLinkFrame*>(frame)) != 0) {
		newFrame = editUrlFrame(wFrame, id);
	}
#else
	else if ((wxxxFrame = dynamic_cast<const UserUrlLinkFrame*>(frame)) != 0) {
		newFrame = editUserUrlFrame(wxxxFrame, id);
	} else if ((wFrame = dynamic_cast<const UrlLinkFrame*>(frame)) != 0) {
		newFrame = editUrlFrame(wFrame, id);
	}
#endif
#ifdef TAGLIB_SUPPORTS_USLT_FRAMES
	else if ((usltFrame = dynamic_cast<const TagLib::ID3v2::UnsynchronizedLyricsFrame*>(frame)) != 0) {
		newFrame = editUsltFrame(usltFrame, id);
	}
#else
	else if ((usltFrame = dynamic_cast<const UnsynchronizedLyricsFrame*>(frame)) != 0) {
		newFrame = editUsltFrame(usltFrame, id);
	}
#endif
	else if (frame != 0) {
		// create temporary objects for frames not known by TagLib,
		// an UnknownFrame copy will be created by the edit method.
#ifndef TAGLIB_SUPPORTS_URLLINK_FRAMES
		if (id.startsWith("WXXX")) {
			UserUrlLinkFrame userUrlLinkFrame(frame->render());
			newFrame = editUserUrlFrame(&userUrlLinkFrame, id);
		} else if (id.startsWith("W")) {
			UrlLinkFrame urlLinkFrame(frame->render());
			newFrame = editUrlFrame(&urlLinkFrame, id);
		} else
#endif
#ifndef TAGLIB_SUPPORTS_USLT_FRAMES
		if (id.startsWith("USLT")) {
			UnsynchronizedLyricsFrame usltFrame(frame->render());
			newFrame = editUsltFrame(&usltFrame, id);
		} else
#endif
			newFrame = editUnknownFrame(frame, id);
	}

	m_fieldcontrols.clear();
	return newFrame;
}

/**
 * Create dialog to edit the selected frame and update the fields
 * if Ok is returned.
 *
 * @return true if Ok selected in dialog.
 */
bool TagLibFrameList::editFrame()
{
	bool edited = false;
	int selectedIndex = listbox->currentItem();
	if (selectedIndex != -1 && m_tag) {
		TagLib::ID3v2::Tag* id3v2Tag;
		TagLib::Ogg::XiphComment* oggTag;
		TagLib::APE::Tag* apeTag;
		if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tag)) != 0) {
			const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
			if (selectedIndex < static_cast<int>(frameList.size())) {
				TagLib::ID3v2::Frame* oldFrame = frameList[selectedIndex];
				TagLib::ID3v2::Frame* newFrame;
				if ((newFrame = editId3v2Frame(oldFrame)) != 0) {
					id3v2Tag->removeFrame(oldFrame);
					id3v2Tag->addFrame(newFrame);
					edited = true;
				}
			}
		} else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tag)) != 0) {
			TagLib::String key, value;
			if (getXiphCommentField(*oggTag, selectedIndex, key, value)) {
				TagLib::String oldValue = value;
				if (editKeyValueField(key, value)) {
					if (!(value == oldValue)) {
#ifdef TAGLIB_XIPHCOMMENT_REMOVEFIELD_CRASHES
						oggTag->addField(key, value, true);
#else
						// This will crash because TagLib uses an invalidated iterator
						// after calling erase(). I hope this will be fixed in the next
						// version. Until then, remove all fields with that key.
						oggTag->removeField(key, oldValue);
						oggTag->addField(key, value, false);
#endif
						edited = true;
					}
				}
			}
		} else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tag)) != 0) {
			TagLib::String key;
			TagLib::APE::Item item;
			if (getApeItem(*apeTag, selectedIndex, key, item)) {
				TagLib::StringList values = item.toStringList();
				TagLib::String value = values.size() > 0 ? values.front() : "";
				TagLib::String oldValue = value;
				if (editKeyValueField(key, value)) {
					if (!(value == oldValue)) {
						apeTag->addValue(key, value, true);
						edited = true;
					}
				}
			}
		}
	}
	if (edited) {
		readTags(); // refresh listbox
		// select the next item (or the last if it was the last)
		if (selectedIndex >= 0) {
			const int lastIndex = listbox->count() - 1;
			if (lastIndex >= 0) {
				listbox->setSelected(
					selectedIndex <= lastIndex ? selectedIndex : lastIndex, true);
			}
		}
		if (m_file) {
			m_file->changedV2 = true;
		}
	}
	return edited;
}

/**
 * Delete selected frame.
 *
 * @return false if frame not found.
 */
bool TagLibFrameList::deleteFrame()
{
	bool deleted = false;
	int selectedIndex = listbox->currentItem();
	if (selectedIndex != -1 && m_tag) {
		TagLib::ID3v2::Tag* id3v2Tag;
		TagLib::Ogg::XiphComment* oggTag;
		TagLib::APE::Tag* apeTag;
		if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tag)) != 0) {
			const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
			if (selectedIndex < static_cast<int>(frameList.size())) {
				id3v2Tag->removeFrame(frameList[selectedIndex]);
				deleted = true;
			}
		} else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tag)) != 0) {
			TagLib::String key, value;
			if (getXiphCommentField(*oggTag, selectedIndex, key, value)) {
#ifdef TAGLIB_XIPHCOMMENT_REMOVEFIELD_CRASHES
				oggTag->removeField(key);
#else
				// This will crash because TagLib uses an invalidated iterator
				// after calling erase(). I hope this will be fixed in the next
				// version. Until then, remove all fields with that key.
				oggTag->removeField(key, value);
#endif
				deleted = true;
			}
		} else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tag)) != 0) {
			TagLib::String key;
			TagLib::APE::Item item;
			if (getApeItem(*apeTag, selectedIndex, key, item)) {
				apeTag->removeItem(key);
				deleted = true;
			}
		}
		if (deleted) {
			readTags(); // refresh listbox
			// select the next item (or the last if it was the last)
			if (selectedIndex >= 0) {
				const int lastIndex = listbox->count() - 1;
				if (lastIndex >= 0) {
					listbox->setSelected(
						selectedIndex <= lastIndex ? selectedIndex : lastIndex, true);
				}
			}
			if (m_file) {
				m_file->changedV2 = true;
			}
			return true;
		}
	}
	return false;
}

/**
 * Create m_tag if it does not already exist so that it can be set.
 *
 * @return true if m_tag can be set.
 */
bool TagLibFrameList::makeTagSettable()
{
	if (!m_tag) {
		TagLibFile* tagLibFile = dynamic_cast<TagLibFile*>(m_file);
		if (tagLibFile && tagLibFile->isTagInformationRead() &&
				tagLibFile->makeTagV2Settable()) {
			m_tag = tagLibFile->m_tagV2;
		}
	}
	return (m_tag != 0);
}

/**
 * Add a new frame.
 *
 * @param frameId ID of frame to add
 * @param edit    true to edit frame after adding it
 * @return true if frame added.
 */
bool TagLibFrameList::addFrame(int frameId, bool edit)
{
	bool added = false;
	if (makeTagSettable()) {
		TagLib::ID3v2::Tag* id3v2Tag;
		TagLib::Ogg::XiphComment* oggTag;
		TagLib::APE::Tag* apeTag;
		if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tag)) != 0) {
			QString frameId = m_selectedName.left(4);
			TagLib::ID3v2::Frame* frame = 0;
			if (frameId.startsWith("T")) {
				if (frameId == "TXXX") {
					frame = new TagLib::ID3v2::UserTextIdentificationFrame;
				} else {
					frame = new TagLib::ID3v2::TextIdentificationFrame(
						TagLib::ByteVector(frameId.latin1(), frameId.length()),
						TagLib::String::Latin1);
					frame->setText(""); // is necessary for createFrame() to work
				}
			} else if (frameId == "COMM") {
				frame = new TagLib::ID3v2::CommentsFrame;
			} else if (frameId == "APIC") {
				frame = new TagLib::ID3v2::AttachedPictureFrame;
			} else if (frameId == "RVA2") {
				frame = new TagLib::ID3v2::RelativeVolumeFrame("RVA2");
			} else if (frameId == "UFID") {
				// the bytevector must not be empty
				frame = new TagLib::ID3v2::UniqueFileIdentifierFrame(
					TagLib::String(), TagLib::ByteVector(" "));
			}
#ifdef TAGLIB_SUPPORTS_GEOB_FRAMES
			else if (frameId == "GEOB") {
				frame = new TagLib::ID3v2::GeneralEncapsulatedObjectFrame;
			}
#endif // TAGLIB_SUPPORTS_GEOB_FRAMES
			else if (frameId.startsWith("W")) {
#ifdef TAGLIB_SUPPORTS_URLLINK_FRAMES
				if (frameId == "WXXX") {
					frame = new TagLib::ID3v2::UserUrlLinkFrame;
				} else {
					frame = new TagLib::ID3v2::UrlLinkFrame(
						TagLib::ByteVector(frameId.latin1(), frameId.length()));
					frame->setText("http://"); // is necessary for createFrame() to work
				}
#else
				if (frameId == "WXXX") {
					frame = new UserUrlLinkFrame;
				} else {
					frame = new UrlLinkFrame(
						TagLib::ByteVector(frameId.latin1(), frameId.length()));
					frame->setText("http://"); // is necessary for createFrame() to work
				}
#endif
			} else if (frameId == "USLT") {
#ifdef TAGLIB_SUPPORTS_USLT_FRAMES
				frame = new TagLib::ID3v2::UnsynchronizedLyricsFrame;
#else
				frame = new UnsynchronizedLyricsFrame;
#endif
			}
			if (frame) {
				TagLib::ID3v2::Frame* newFrame = edit ?
					editId3v2Frame(frame) : copyId3v2Frame(frame);
				delete frame;
				if (newFrame) {
					id3v2Tag->addFrame(newFrame);
					added = true;
					readTags(); // refresh listbox
					const int lastIndex = listbox->count() - 1;
					if (lastIndex >= 0) {
						listbox->setSelected(lastIndex, true);
					}
				}
			}
		} else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tag)) != 0) {
			if (frameId != 0) {
				return false;
			}
			TagLib::String key = QStringToTString(m_selectedName);
			TagLib::String value("");
			if (!edit || editKeyValueField(key, value)) {
				oggTag->addField(key, value);
				added = true;
				readTags(); // refresh listbox
				QListBoxItem* lbi = listbox->findItem(m_selectedName);
				if (lbi) {
					listbox->setSelected(lbi, true);
				}
			}
		} else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tag)) != 0) {
			if (frameId != 0) {
				return false;
			}
			TagLib::String key = QStringToTString(m_selectedName);
			TagLib::String value("");
			if (!edit || editKeyValueField(key, value)) {
				apeTag->addValue(key, value, true);
				added = true;
				readTags(); // refresh listbox
				QListBoxItem* lbi = listbox->findItem(m_selectedName);
				if (lbi) {
					listbox->setSelected(lbi, true);
				}
			}
		}
		if (added) {
			if (m_file) {
				m_file->changedV2 = true;
			}
		}
	}
	return added;
}

/**
 * Display a dialog to select a frame type.
 *
 * @return ID of selected frame, to be passed to addFrame(),
 *         -1 if no frame selected.
 */
int TagLibFrameList::selectFrameId()
{
	TagLibFile* tagLibFile;
	TagLib::File* file;
	if (m_file && (tagLibFile = dynamic_cast<TagLibFile*>(m_file)) != 0 &&
			tagLibFile->isTagInformationRead() &&
			!tagLibFile->m_fileRef.isNull() &&
			(file = tagLibFile->m_fileRef.file()) != 0) {
		TagLib::MPEG::File* mpegFile;
		const char* const* stringArray = 0;
		unsigned numStrings = 0;
		QStringList lst;
		if ((mpegFile = dynamic_cast<TagLib::MPEG::File*>(file)) != 0) {
			for (unsigned i = 0;
					 i < sizeof(id3v2FrameIdTable) / sizeof(id3v2FrameIdTable[0]);
					 ++i) {
				if (id3v2FrameIdTable[i].supported) {
					lst.append(id3v2FrameIdTable[i].str);
				}
			}
		} else {
			/** Alphabetically sorted list of frame descriptions */
			static const char* const fieldNames[] = {
				"ALBUM",
				"ARRANGER",
				"ARTIST",
				"AUTHOR",
				"CATALOGNUMBER",
				"COMMENT",
				"COMPOSER",
				"CONDUCTOR",
				"CONTACT",
				"COPYRIGHT",
				"DATE",
				"DESCRIPTION",
				"DISCID",
				"DISCNUMBER",
				"EAN/UPN",
				"ENCODED-BY",
				"ENCODING",
				"ENGINEER",
				"ENSEMBLE",
				"GENRE",
				"GUEST ARTIST",
				"ISRC",
				"LABEL",
				"LABELNO",
				"LICENSE",
				"LOCATION",
				"LYRICIST",
				"OPUS",
				"ORGANIZATION",
				"PART",
				"PARTNUMBER",
				"PERFORMER",
				"PRODUCER",
				"PRODUCTNUMBER",
				"PUBLISHER",
				"RELEASE DATE",
				"REMIXER",
				"SOURCE ARTIST",
				"SOURCE MEDIUM",
				"SOURCE WORK",
				"SOURCEMEDIA",
				"SPARS",
				"SUBTITLE",
				"TITLE",
				"TRACKNUMBER",
				"TRACKTOTAL",
				"VERSION",
				"VOLUME",
				"" // user comment
			};
			stringArray = fieldNames;
			numStrings = sizeof(fieldNames) / sizeof(fieldNames[0]);
		}
		if (stringArray) {
			for (unsigned i = 0; i < numStrings; ++i) {
				lst.append(stringArray[i]);
			}
		}
		if (!lst.empty()) {
			bool ok = false;
			QString res = QInputDialog::getItem(
				i18n("Add Frame"),
				i18n("Select the frame ID"), lst, 0, true, &ok);
			if (ok) {
				m_selectedName = res;
				return 0; // just used by addFrame()
			}
		}
	}
	return -1;
}

/**
 * Copy the selected frame to the copy buffer.
 *
 * @return true if frame copied.
 */
bool TagLibFrameList::copyFrame()
{
	bool copied = false;
	int selectedIndex = listbox->currentItem();
	if (selectedIndex != -1 && m_tag) {
		TagLib::ID3v2::Tag* id3v2Tag;
		TagLib::Ogg::XiphComment* oggTag;
		TagLib::APE::Tag* apeTag;
		if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tag)) != 0) {
			const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
			if (selectedIndex < static_cast<int>(frameList.size())) {
				m_copyData = frameList[selectedIndex]->render();
				copied = true;
			}
		} else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tag)) != 0) {
			TagLib::String key, value;
			if (getXiphCommentField(*oggTag, selectedIndex, key, value)) {
				m_copyKey = key;
				m_copyValue = value;
				copied = true;
			}
		} else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tag)) != 0) {
			TagLib::String key;
			TagLib::APE::Item item;
			if (getApeItem(*apeTag, selectedIndex, key, item)) {
				m_copyKey = key;
				TagLib::StringList values = item.toStringList();
				m_copyValue = values.size() > 0 ? values.front() : "";
				copied = true;
			}
		}
	}
	return copied;
}

	/**
	 * Paste the selected frame from the copy buffer.
	 *
	 * @return true if frame pasted.
	 */
bool TagLibFrameList::pasteFrame()
{
	bool pasted = false;
	if (makeTagSettable()) {
		TagLib::ID3v2::Tag* id3v2Tag;
		TagLib::Ogg::XiphComment* oggTag;
		TagLib::APE::Tag* apeTag;
		if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tag)) != 0) {
			TagLib::ID3v2::Frame* frame;
			// Setting a version other than the default 4 makes not much sense as
			// TagLib always writes ID3v2.4.0 tags.
			if (!m_copyData.isEmpty() &&
					(frame = TagLib::ID3v2::FrameFactory::instance()->createFrame(
						m_copyData)) != 0) {
				id3v2Tag->addFrame(frame);
				pasted = true;
			}
		} else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tag)) != 0) {
			if (!m_copyKey.isEmpty()) {
				oggTag->addField(m_copyKey, m_copyValue);
				pasted = true;
			}
		} else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tag)) != 0) {
			if (!m_copyKey.isEmpty()) {
				apeTag->addValue(m_copyKey, m_copyValue, true);
				pasted = true;
			}
		}
	}
	if (pasted && m_file) {
		m_file->changedV2 = true;
	}
	return pasted;
}

#else // HAVE_TAGLIB

void TagLibBinaryOpenSave::loadData() {}
void TagLibBinaryOpenSave::saveData() {}
void TagLibBinaryOpenSave::viewData() {}

#endif // HAVE_TAGLIB
