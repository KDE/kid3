/**
 * \file configdialog.cpp
 * Configuration dialog.
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
#endif

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qstring.h>
#include <qcheckbox.h>

#include "formatconfig.h"
#include "formatbox.h"
#include "miscconfig.h"
#include "configdialog.h"

/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param caption dialog title
 */
ConfigDialog::ConfigDialog(QWidget *parent, QString &caption)
#ifdef CONFIG_USE_KDE
	: KDialogBase(parent, "configure", true, caption, Ok|Cancel, Ok)
#else
	: QDialog(parent, "configure", true)
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

	formatEditingCheckBox = new QCheckBox(page, "formatEditingCheckBox");
	formatEditingCheckBox->setText(i18n("Format while &editing:"));
	vlayout->addWidget(formatEditingCheckBox);

	QHBoxLayout *fmtLayout = new QHBoxLayout(vlayout);
	QString fnFormatTitle(i18n("&Filename Format"));
	fnFormatBox = new FormatBox(fnFormatTitle, page, "fnFormatBox");
	QString id3FormatTitle(i18n("&ID3 Format"));
	id3FormatBox = new FormatBox(id3FormatTitle, page, "id3FormatBox");
	if (fmtLayout && fnFormatBox && id3FormatBox) {
		fmtLayout->addWidget(fnFormatBox);
		fmtLayout->addWidget(id3FormatBox);
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
ConfigDialog::~ConfigDialog()
{}

/**
 * Set values in dialog from current configuration.
 *
 * @param fnCfg   filename format configuration
 * @param fnCfg   ID3 format configuration
 * @param miscCfg misc. configuration
 */
void ConfigDialog::setConfig(const FormatConfig *fnCfg,
							 const FormatConfig *id3Cfg,
							 const MiscConfig *miscCfg)
{
	fnFormatBox->fromFormatConfig(fnCfg);
	id3FormatBox->fromFormatConfig(id3Cfg);
	formatEditingCheckBox->setChecked(miscCfg->formatWhileEditing);
}

/**
 * Get values from dialog and store them in the current configuration.
 *
 * @param fnCfg   filename format configuration
 * @param fnCfg   ID3 format configuration
 * @param miscCfg misc. configuration
 */
void ConfigDialog::getConfig(FormatConfig *fnCfg,
							 FormatConfig *id3Cfg,
							 MiscConfig *miscCfg) const
{
	fnFormatBox->toFormatConfig(fnCfg);
	id3FormatBox->toFormatConfig(id3Cfg);
	miscCfg->formatWhileEditing = formatEditingCheckBox->isChecked();
}
