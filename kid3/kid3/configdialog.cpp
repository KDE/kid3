/**
 * \file configdialog.cpp
 * Configuration dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 */

#include "configdialog.h"
#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kconfig.h>
#endif

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <Q3GroupBox>
#else
#include <qgroupbox.h>
#endif

#include "formatconfig.h"
#include "formatbox.h"
#include "miscconfig.h"
#include "stringlistedit.h"
#include "commandstable.h"
#include "kid3.h"

/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param caption dialog title
 */
#ifdef KID3_USE_KCONFIGDIALOG
ConfigDialog::ConfigDialog(QWidget* parent, QString& caption,
													 KConfigSkeleton* configSkeleton) :
	KConfigDialog(parent, "configure", configSkeleton,
								IconList, Ok | Cancel | Help, Ok, true)
#else
ConfigDialog::ConfigDialog(QWidget* parent, QString& caption) :
	Q3TabDialog(parent, "configure", true)
#endif
{
	setCaption(caption);

	QWidget* tagsPage = new QWidget;
	if (tagsPage) {
		QVBoxLayout* vlayout = new QVBoxLayout(tagsPage, 6, 6);
		if (vlayout) {
			Q3GroupBox* v2GroupBox = new Q3GroupBox(1, Qt::Horizontal, i18n("ID3v2"), tagsPage);
			if (v2GroupBox) {
#if QT_VERSION >= 0x040000
				v2GroupBox->setInsideMargin(5);
#endif
				m_totalNumTracksCheckBox = new QCheckBox(i18n("Use &track/total number of tracks format"), v2GroupBox);
#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
				QLabel* id3v2VersionLabel = new QLabel(i18n("&Version used for new tags:"), v2GroupBox);
				m_id3v2VersionComboBox = new QComboBox(v2GroupBox);
				if (id3v2VersionLabel && m_id3v2VersionComboBox) {
					m_id3v2VersionComboBox->insertItem(i18n("ID3v2.3.0 (id3lib)"), MiscConfig::ID3v2_3_0);
					m_id3v2VersionComboBox->insertItem(i18n("ID3v2.4.0 (TagLib)"), MiscConfig::ID3v2_4_0);
					m_id3v2VersionComboBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
					id3v2VersionLabel->setBuddy(m_id3v2VersionComboBox);
				}
#endif
				vlayout->addWidget(v2GroupBox);
			}
#ifdef HAVE_VORBIS
			Q3GroupBox* vorbisGroupBox = new Q3GroupBox(2, Qt::Horizontal, i18n("Ogg/Vorbis"), tagsPage);
			if (vorbisGroupBox) {
#if QT_VERSION >= 0x040000
				vorbisGroupBox->setInsideMargin(5);
#endif
				QLabel* commentNameLabel = new QLabel(i18n("Comment Field &Name:"), vorbisGroupBox);
				m_commentNameComboBox = new QComboBox(true, vorbisGroupBox);
				if (commentNameLabel && m_commentNameComboBox) {
					QStringList items;
					items += "COMMENT";
					items += "DESCRIPTION";
					m_commentNameComboBox->QCM_addItems(items);
					m_commentNameComboBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
					commentNameLabel->setBuddy(m_commentNameComboBox);
				}
				vlayout->addWidget(vorbisGroupBox);
			}
#endif
			QHBoxLayout* hlayout = new QHBoxLayout(vlayout);
			if (hlayout) {
				Q3GroupBox* genresGroupBox = new Q3GroupBox(1, Qt::Horizontal, i18n("Custom &Genres"), tagsPage);
				if (genresGroupBox) {
#if QT_VERSION >= 0x040000
					genresGroupBox->setInsideMargin(5);
#endif
					m_onlyCustomGenresCheckBox = new QCheckBox(i18n("&Show only custom genres"), genresGroupBox);
					m_genresEdit = new StringListEdit(genresGroupBox, "genresList");
					hlayout->addWidget(genresGroupBox);
				}
				QString id3FormatTitle(i18n("&Tag Format"));
				m_id3FormatBox = new FormatBox(id3FormatTitle, tagsPage, "id3FormatBox");
				if (m_id3FormatBox) {
					hlayout->addWidget(m_id3FormatBox);
				}
			}
		}
#ifdef KID3_USE_KCONFIGDIALOG
		addPage(tagsPage, i18n("Tags"), "package_multimedia");
#else
		addTab(tagsPage, i18n("&Tags"));
#endif
	}

	QWidget* filesPage = new QWidget;
	if (filesPage) {
		QVBoxLayout* vlayout = new QVBoxLayout(filesPage, 6, 6);
		if (vlayout) {
			Q3GroupBox* saveGroupBox = new Q3GroupBox(1, Qt::Horizontal, i18n("Save"), filesPage);
			if (saveGroupBox) {
#if QT_VERSION >= 0x040000
				saveGroupBox->setInsideMargin(5);
#endif
				m_preserveTimeCheckBox = new QCheckBox(i18n("&Preserve file timestamp"), saveGroupBox);
				vlayout->addWidget(saveGroupBox);
			}
			QString fnFormatTitle(i18n("&Filename Format"));
			m_fnFormatBox = new FormatBox(fnFormatTitle, filesPage, "fnFormatBox");
			if (m_fnFormatBox) {
				vlayout->addWidget(m_fnFormatBox);
			}
		}
#ifdef KID3_USE_KCONFIGDIALOG
		addPage(filesPage, i18n("Files"), "package_system");
#else
		addTab(filesPage, i18n("&Files"));
#endif
	}

	QWidget* actionsPage = new QWidget;
	if (actionsPage) {
		QVBoxLayout* vlayout = new QVBoxLayout(actionsPage, 6, 6);
		if (vlayout) {
			Q3GroupBox* browserGroupBox = new Q3GroupBox(2, Qt::Horizontal, i18n("Browser"), actionsPage);
			if (browserGroupBox) {
#if QT_VERSION >= 0x040000
				browserGroupBox->setInsideMargin(5);
#endif
				QLabel* browserLabel = new QLabel(i18n("Web &Browser:"), browserGroupBox);
				m_browserLineEdit = new QLineEdit(browserGroupBox);
				if (browserLabel && m_browserLineEdit) {
					browserLabel->setBuddy(m_browserLineEdit);
				}
				vlayout->addWidget(browserGroupBox);
			}

			Q3GroupBox* commandsGroupBox = new Q3GroupBox(1, Qt::Horizontal, i18n("Context &Menu Commands"), actionsPage);
			if (commandsGroupBox) {
#if QT_VERSION >= 0x040000
				commandsGroupBox->setInsideMargin(5);
#endif
				m_commandsTable = new CommandsTable(commandsGroupBox, "commandsTable");
				vlayout->addWidget(commandsGroupBox);
			}
			QSpacerItem* vspacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
			vlayout->addItem(vspacer);
		}
#ifdef KID3_USE_KCONFIGDIALOG
		addPage(actionsPage, i18n("User Actions"), "package_utilities");
#else
		addTab(actionsPage, i18n("&User Actions"));
#endif
	}

	QWidget* networkPage = new QWidget;
	if (networkPage) {
		QVBoxLayout* vlayout = new QVBoxLayout(networkPage, 6, 6);
		if (vlayout) {
			Q3GroupBox* proxyGroupBox = new Q3GroupBox(2, Qt::Horizontal, i18n("Proxy"), networkPage);
			if (proxyGroupBox) {
#if QT_VERSION >= 0x040000
				proxyGroupBox->setInsideMargin(5);
#endif
				m_proxyCheckBox = new QCheckBox(i18n("&Proxy:"), proxyGroupBox);
				m_proxyLineEdit = new QLineEdit(proxyGroupBox);
				vlayout->addWidget(proxyGroupBox);
			}

			QSpacerItem* vspacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
			vlayout->addItem(vspacer);
		}
#ifdef KID3_USE_KCONFIGDIALOG
		addPage(networkPage, i18n("Network"), "package_network");
#else
		addTab(networkPage, i18n("&Network"));
#endif
	}

#ifndef KID3_USE_KCONFIGDIALOG
	setHelpButton(i18n("&Help"));
	connect(this, SIGNAL(helpButtonPressed()), this, SLOT(slotHelp()));

	setOkButton(i18n("&OK"));
	setCancelButton(i18n("&Cancel"));
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
void ConfigDialog::setConfig(const FormatConfig* fnCfg,
							 const FormatConfig* id3Cfg,
							 const MiscConfig* miscCfg)
{
	m_fnFormatBox->fromFormatConfig(fnCfg);
	m_id3FormatBox->fromFormatConfig(id3Cfg);
	m_totalNumTracksCheckBox->setChecked(miscCfg->m_enableTotalNumberOfTracks);
	m_preserveTimeCheckBox->setChecked(miscCfg->m_preserveTime);
	m_onlyCustomGenresCheckBox->setChecked(miscCfg->m_onlyCustomGenres);
	m_genresEdit->setStrings(miscCfg->m_customGenres);
	m_commandsTable->setCommandList(miscCfg->m_contextMenuCommands);
#ifdef HAVE_VORBIS
	m_commentNameComboBox->setCurrentText(miscCfg->m_commentName);
#endif
#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
	m_id3v2VersionComboBox->setCurrentItem(miscCfg->m_id3v2Version);
#endif
	m_browserLineEdit->setText(miscCfg->m_browser);
	m_proxyCheckBox->setChecked(miscCfg->m_useProxy);
	m_proxyLineEdit->setText(miscCfg->m_proxy);
}

/**
 * Get values from dialog and store them in the current configuration.
 *
 * @param fnCfg   filename format configuration
 * @param fnCfg   ID3 format configuration
 * @param miscCfg misc. configuration
 */
void ConfigDialog::getConfig(FormatConfig* fnCfg,
							 FormatConfig* id3Cfg,
							 MiscConfig* miscCfg) const
{
	m_fnFormatBox->toFormatConfig(fnCfg);
	m_id3FormatBox->toFormatConfig(id3Cfg);
	miscCfg->m_enableTotalNumberOfTracks = m_totalNumTracksCheckBox->isChecked();
	miscCfg->m_preserveTime = m_preserveTimeCheckBox->isChecked();
	miscCfg->m_onlyCustomGenres = m_onlyCustomGenresCheckBox->isChecked();
	m_genresEdit->getStrings(miscCfg->m_customGenres);
	m_commandsTable->getCommandList(miscCfg->m_contextMenuCommands);
#ifdef HAVE_VORBIS
	miscCfg->m_commentName = m_commentNameComboBox->currentText();
#endif
#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
	miscCfg->m_id3v2Version = m_id3v2VersionComboBox->currentItem();
#endif
	miscCfg->m_browser = m_browserLineEdit->text();
	miscCfg->m_useProxy = m_proxyCheckBox->isChecked();
	miscCfg->m_proxy = m_proxyLineEdit->text();
}

/**
 * Show help.
 */
void ConfigDialog::slotHelp()
{
	Kid3App::displayHelp("configure-kid3");
}
