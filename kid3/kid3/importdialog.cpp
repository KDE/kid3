/**
 * \file importdialog.cpp
 * Import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 */

#include "config.h"
#ifdef CONFIG_USE_KDE
#include <klocale.h>
#include <kconfig.h>
#else
#define i18n(s) tr(s)
#define I18N_NOOP(s) QT_TR_NOOP(s)
#include <qhbox.h>
#endif

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qstring.h>
#include "importselector.h"
#include "importdialog.h"

/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param caption dialog title
 */
ImportDialog::ImportDialog(QWidget *parent, QString &caption)
#ifdef CONFIG_USE_KDE
	: KDialogBase(parent, "import", true, caption, Ok|Cancel, Ok)
#else
	: QDialog(parent, "import", true)
#endif
{

#ifdef CONFIG_USE_KDE
	QWidget *page = new QWidget(this);
	if (!page) {
		return;
	}
	setMainWidget(page);
#else
#define page this
	setCaption(caption);
#endif

	QVBoxLayout *vlayout = new QVBoxLayout(page);
	if (!vlayout) {
		return ;
	}
	vlayout->setSpacing(6);
	vlayout->setMargin(6);
	impsel = new ImportSelector(page);
	vlayout->addWidget(impsel);

	QHBoxLayout* checkLayout = new QHBoxLayout(vlayout);
	if (checkLayout) {
		mismatchCheckBox = new QCheckBox(page);
		mismatchCheckBox->setText(
			i18n("Check maximum allowable time difference (sec):"));
		maxDiffSpinBox = new QSpinBox(page);
		maxDiffSpinBox->setMaxValue(9999);
		if (mismatchCheckBox && maxDiffSpinBox) {
			checkLayout->addSpacing(vlayout->margin() * 2);
			checkLayout->addWidget(mismatchCheckBox);
			checkLayout->addWidget(maxDiffSpinBox);
			checkLayout->addStretch();
		}
	}

#ifndef CONFIG_USE_KDE
	QHBoxLayout *hlayout = new QHBoxLayout(vlayout);
	QSpacerItem *hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
	                                       QSizePolicy::Minimum);
	QPushButton *okButton = new QPushButton(i18n("OK"), page);
	QPushButton *cancelButton = new QPushButton(i18n("Cancel"), page);
	if (hlayout && okButton && cancelButton) {
		hlayout->addItem(hspacer);
		hlayout->addWidget(okButton);
		hlayout->addWidget(cancelButton);
		okButton->setDefault(TRUE);
		connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	}
#undef page
#endif
}

/**
 * Destructor.
 */
ImportDialog::~ImportDialog()
{}

/**
 * Look for album specific information (artist, album, year, genre) in
 * a header (e.g. in a freedb header).
 *
 * @param st standard tags to put resulting values in,
 *           fields which are not found are not touched.
 *
 * @return true if one or more field were found.
 */
bool ImportDialog::parseHeader(StandardTags &st)
{
	return impsel->parseHeader(st);
}

/**
 * Get next line as standardtags from imported file or clipboard.
 *
 * @param st standard tags
 * @param start true to start with the first line, false for all
 *              other lines
 *
 * @return true if ok (result in st),
 *         false if end of file reached.
 */
bool ImportDialog::getNextTags(StandardTags &st, bool start)
{
	return impsel->getNextTags(st, start);
}

/**
 * Set ID3v1 or ID3v2 tags as import destination.
 *
 * @param v1 true to set ID3v1, false for ID3v2
 */
void ImportDialog::setDestV1(bool v1)
{
	impsel->setDestination(v1 ? ImportSelector::DestV1 : ImportSelector::DestV2);
}

/**
 * Get import destination.
 *
 * @return true if ID3v1 is destination,
 *         false if ID3v2.
 */
bool ImportDialog::getDestV1() const
{
	return (impsel->getDestination() == ImportSelector::DestV1);
}

/**
 * Set import format regexp.
 *
 * @param names   import format names list
 * @param headers import format header regexps
 * @param tracks  import format track regexps
 * @param idx     selected index
 */
void ImportDialog::setImportFormat(const QStringList &names,
								   const QStringList &headers,
								   const QStringList &tracks,
								   int idx)
{
	impsel->setImportFormat(names, headers, tracks, idx);
}

/**
 * Set time difference check configuration.
 *
 * @param enable  true to enable check
 * @param maxDiff maximum allowable time difference
 */ 
void ImportDialog::setTimeDifferenceCheck(bool enable, int maxDiff)
{
	mismatchCheckBox->setChecked(enable);
	maxDiffSpinBox->setValue(maxDiff);
}

/**
 * Get time difference check configuration.
 *
 * @param enable  true if check is enabled
 * @param maxDiff maximum allowed time difference
 */ 
void ImportDialog::getTimeDifferenceCheck(bool& enable, int& maxDiff) const
{
	enable = mismatchCheckBox->isChecked();
	maxDiff = maxDiffSpinBox->value();
}

/**
 * Get import format regexp.
 *
 * @param name   import format name
 * @param header import format header regexp
 * @param track  import format track regexp
 *
 * @return index of current selection.
 */
int ImportDialog::getImportFormat(QString &name,
								  QString &header,
								  QString &track) const
{
	return impsel->getImportFormat(name, header, track);
}

/**
 * Set freedb.org configuration.
 *
 * @param cfg freedb configuration.
 */
void ImportDialog::setFreedbConfig(const FreedbConfig *cfg)
{
	impsel->setFreedbConfig(cfg);
}

/**
 * Get freedb.org configuration.
 *
 * @param cfg freedb configuration.
 */
void ImportDialog::getFreedbConfig(FreedbConfig *cfg) const
{
	impsel->getFreedbConfig(cfg);
}

/**
 * Get list with track durations.
 *
 * @return list with track durations,
 *         0 if no track durations found.
 */
QValueList<int>* ImportDialog::getTrackDurations()
{
	return impsel->getTrackDurations();
}
