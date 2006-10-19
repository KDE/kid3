/**
 * \file numbertracksdialog.cpp
 * Number tracks dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 May 2006
 */

#include "numbertracksdialog.h"
#include "config.h"
#ifdef CONFIG_USE_KDE
#include <klocale.h>
#include <kconfig.h>
#else
#define i18n(s) tr(s)
#define I18N_NOOP(s) QT_TR_NOOP(s)
#endif

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qstring.h>
#include <qcombobox.h>
#include "kid3.h"

/**
 * Constructor.
 *
 * @param parent parent widget
 */
NumberTracksDialog::NumberTracksDialog(QWidget* parent) :
	QDialog(parent, "numbertracks", true)
{
	setCaption(i18n("Number Tracks"));

	QVBoxLayout* vlayout = new QVBoxLayout(this, 6, 6, "vlayout");
	if (vlayout) {
		QHBoxLayout* trackLayout = new QHBoxLayout(vlayout, 6, "trackLayout");
		if (trackLayout) {
			QLabel* trackLabel = new QLabel(i18n("&Start number:"), this, "trackLabel");
			m_trackSpinBox = new QSpinBox(0, 999, 1, this, "trackSpinBox");
			if (trackLabel && m_trackSpinBox) {
				m_trackSpinBox->setValue(1);
				trackLayout->addWidget(trackLabel);
				trackLayout->addWidget(m_trackSpinBox);
				trackLabel->setBuddy(m_trackSpinBox);
			}
			QSpacerItem* trackSpacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
																								 QSizePolicy::Minimum);
			trackLayout->addItem(trackSpacer);

			QLabel* destLabel = new QLabel(i18n("&Destination:"), this, "destLabel");
			m_destComboBox = new QComboBox(false, this, "destComboBox");
			if (destLabel && m_destComboBox) {
				m_destComboBox->insertItem(i18n("Tag 1"), DestV1);
				m_destComboBox->insertItem(i18n("Tag 2"), DestV2);
				trackLayout->addWidget(destLabel);
				trackLayout->addWidget(m_destComboBox);
				destLabel->setBuddy(m_destComboBox);
			}
		}

		QHBoxLayout* hlayout = new QHBoxLayout(vlayout, 6, "hlayout");
		if (hlayout) {
			QPushButton* helpButton = new QPushButton(i18n("&Help"), this);
			if (helpButton) {
				hlayout->addWidget(helpButton);
				connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
			}
			QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
																						 QSizePolicy::Minimum);
			hlayout->addItem(hspacer);

			QPushButton* okButton = new QPushButton(i18n("&OK"), this);
			if (okButton) {
				hlayout->addWidget(okButton);
				connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
			}
			QPushButton* cancelButton = new QPushButton(i18n("&Cancel"), this);
			if (cancelButton) {
				hlayout->addWidget(cancelButton);
				connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
			}
		}
	}
}

/**
 * Destructor.
 */
NumberTracksDialog::~NumberTracksDialog()
{}

/**
 * Get start number.
 */
int NumberTracksDialog::getStartNumber() const
{
	return m_trackSpinBox->value();
}

/**
 * Get destination.
 *
 * @return true if ID3v1 is destination,
 *         false if ID3v2.
 */
bool NumberTracksDialog::getDestV1() const
{
	return (m_destComboBox->currentItem() == DestV1);
}

/**
 * Show help.
 */
void NumberTracksDialog::showHelp()
{
	Kid3App::displayHelp("number-tracks");
}
