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
#include <kconfig.h>
#endif

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qstring.h>
#if QT_VERSION >= 0x040000
#include <QVBoxLayout>
#include <QHBoxLayout>
#else
#include <qhbox.h>
#endif
#include "importselector.h"
#include "importdialog.h"
#include "kid3.h"

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
	QPushButton *helpButton = new QPushButton(i18n("&Help"), this);
	QPushButton *saveButton = new QPushButton(i18n("&Save Settings"), this);
	QPushButton *okButton = new QPushButton(i18n("&OK"), this);
	QPushButton *cancelButton = new QPushButton(i18n("&Cancel"), this);
	if (hlayout && helpButton && okButton && saveButton && cancelButton) {
		hlayout->addWidget(helpButton);
		hlayout->addWidget(saveButton);
		hlayout->addItem(hspacer);
		hlayout->addWidget(okButton);
		hlayout->addWidget(cancelButton);
		okButton->setDefault(true);
		connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
		connect(saveButton, SIGNAL(clicked()), impsel, SLOT(saveConfig()));
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
		case ASD_Freedb:
			show();
			impsel->fromFreedb();
			break;
		case ASD_Discogs:
			show();
			impsel->fromDiscogs();
			break;
		case ASD_MusicBrainzRelease:
			show();
			impsel->fromMusicBrainzRelease();
			break;
		case ASD_MusicBrainz:
			show();
			impsel->fromMusicBrainz();
			break;
		case ASD_None:
			break;
	}
	return QDialog::exec();
}

/**
 * Clear dialog data.
 */
void ImportDialog::clear()
{
	impsel->clear();
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
 * Show help.
 */
void ImportDialog::showHelp()
{
	Kid3App::displayHelp("import");
}
