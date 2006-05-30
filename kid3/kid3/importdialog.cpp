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
 * @param parent        parent widget
 * @param caption       dialog title
 * @param trackDataList track data to be filled with imported values,
 *                      is passed with durations of files set
 */
ImportDialog::ImportDialog(QWidget *parent, QString &caption,
													 ImportTrackDataVector& trackDataList) :
	QDialog(parent, "import", true),
	m_autoStartSubDialog(ASD_None),
	m_trackDataVector(trackDataList)
{
	setCaption(caption);

	QVBoxLayout *vlayout = new QVBoxLayout(this);
	if (!vlayout) {
		return ;
	}
	vlayout->setSpacing(6);
	vlayout->setMargin(6);
	impsel = new ImportSelector(this, m_trackDataVector);
	vlayout->addWidget(impsel);

	QHBoxLayout *hlayout = new QHBoxLayout(vlayout);
	QSpacerItem *hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
	                                       QSizePolicy::Minimum);
	QPushButton *okButton = new QPushButton(i18n("&OK"), this);
	QPushButton *cancelButton = new QPushButton(i18n("&Cancel"), this);
	if (hlayout && okButton && cancelButton) {
		hlayout->addItem(hspacer);
		hlayout->addWidget(okButton);
		hlayout->addWidget(cancelButton);
		okButton->setDefault(TRUE);
		connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	}
}

/**
 * Destructor.
 */
ImportDialog::~ImportDialog()
{}

/**
 * Shows the dialog as a modal dialog.
 */
int ImportDialog::exec()
{
	switch (m_autoStartSubDialog) {
		case ASD_None:
			return QDialog::exec();

		case ASD_Freedb:
		case ASD_MusicBrainz:
			setModal(true);
			show();
			if (m_autoStartSubDialog == ASD_Freedb) {
				impsel->fromFreedb();
			} else if (m_autoStartSubDialog == ASD_MusicBrainz) {
				impsel->fromMusicBrainz();
			}
			return result();
	}
	return QDialog::Rejected;
}

/**
 * Clear dialog data.
 */
void ImportDialog::clear()
{
	impsel->clear();
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
	impsel->setTimeDifferenceCheck(enable, maxDiff);
}

/**
 * Get time difference check configuration.
 *
 * @param enable  true if check is enabled
 * @param maxDiff maximum allowed time difference
 */ 
void ImportDialog::getTimeDifferenceCheck(bool& enable, int& maxDiff) const
{
	impsel->getTimeDifferenceCheck(enable, maxDiff);
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

#ifdef HAVE_TUNEPIMP
/**
 * Set MusicBrainz configuration.
 *
 * @param cfg MusicBrainz configuration.
 */
void ImportDialog::setMusicBrainzConfig(const MusicBrainzConfig* cfg)
{
	impsel->setMusicBrainzConfig(cfg);
}

/**
 * Get MusicBrainz configuration.
 *
 * @param cfg MusicBrainz configuration.
 */
void ImportDialog::getMusicBrainzConfig(MusicBrainzConfig* cfg) const
{
	impsel->getMusicBrainzConfig(cfg);
}
#endif
