/**
 * \file configdialog.h
 * Configuration dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2007  Urs Fleisch
 *
 * This file is part of Kid3.
 *
 * Kid3 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Kid3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kdeversion.h>
#if KDE_VERSION >= 0x30200
#define KID3_USE_KCONFIGDIALOG
#endif
#endif

#include <qstringlist.h>
#include "qtcompatmac.h"
#ifdef KID3_USE_KCONFIGDIALOG
#include <kconfigdialog.h>
#else
#include <qdialog.h>
#endif
#include "formatconfig.h"

class FormatBox;
class QCheckBox;
class FormatConfig;
class MiscConfig;
class StringListEdit;
class CommandsTable;
class QString;
class QWidget;
class QComboBox;
class QLineEdit;
class QSpinBox;

/** Base class for configuration dialog. */
#ifdef KID3_USE_KCONFIGDIALOG
typedef KConfigDialog ConfigDialogBaseClass;
#else
typedef QDialog ConfigDialogBaseClass;
#endif

/**
 * Configuration dialog.
 */
class ConfigDialog : public ConfigDialogBaseClass
{
Q_OBJECT
public:
	/**
	 * Constructor.
	 *
	 * @param parent  parent widget
	 * @param caption dialog title
	 * @param configSkeleton KDE config skeleton
	 */
#ifdef KID3_USE_KCONFIGDIALOG
	ConfigDialog(QWidget* parent, QString& caption,
							 KConfigSkeleton* configSkeleton);
#else
	ConfigDialog(QWidget* parent, QString& caption);
#endif
	/**
	 * Destructor.
	 */
	~ConfigDialog();
	/**
	 * Set values in dialog from current configuration.
	 *
	 * @param fnCfg   filename format configuration
	 * @param id3Cfg  ID3 format configuration
	 * @param miscCfg misc. configuration
	 */
	void setConfig(const FormatConfig* fnCfg, const FormatConfig* id3Cfg,
				   const MiscConfig* miscCfg);
	/**
	 * Get values from dialog and store them in the current configuration.
	 *
	 * @param fnCfg   filename format configuration
	 * @param id3Cfg  ID3 format configuration
	 * @param miscCfg misc. configuration
	 */
	void getConfig(FormatConfig* fnCfg, FormatConfig* id3Cfg,
				   MiscConfig* miscCfg) const;

protected slots:
	/**
	 * Show help.
	 */
	virtual void slotHelp();

	/**
	 * Select custom application font.
	 */
	void slotSelectFont();

	/**
	 * Select custom application style.
	 *
	 * @param key style key
	 */
	void slotSelectStyle(const QString& key);

	/**
	 * Revert the font and style to the values in the settings.
	 */
	void slotRevertFontAndStyle();

private:
	/** Preserve timestamp checkbox */
	QCheckBox* m_preserveTimeCheckBox;
	/** Mark changes checkbox */
	QCheckBox* m_markChangesCheckBox;
	/** Mark truncated fields checkbox */
	QCheckBox* m_markTruncationsCheckBox;
	/** ID3v1 text encodings */
	QStringList m_textEncodingV1List;
	/** ID3v1 text encoding combo box */
	QComboBox* m_textEncodingV1ComboBox;
	/** Use track/total number of tracks format checkbox */
	QCheckBox* m_totalNumTracksCheckBox;
	/** Comment field name combo box */
	QComboBox* m_commentNameComboBox;
	/** Picture field name combo box */
	QComboBox* m_pictureNameComboBox;
#if defined HAVE_ID3LIB || defined HAVE_TAGLIB
	/** Genre as text instead of numeric string checkbox */
	QCheckBox* m_genreNotNumericCheckBox;
	/** ID3v2 text encoding combo box */
	QComboBox* m_textEncodingComboBox;
#endif
#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
	/** ID3v2 version combo box */
	QComboBox* m_id3v2VersionComboBox;
#endif
	/** Number of digits in track number spin box */
	QSpinBox* m_trackNumberDigitsSpinBox;
	/** Filename Format box */
	FormatBox* m_fnFormatBox;
	/** ID3 Format box */
	FormatBox* m_id3FormatBox;
	/** Only custom genres checkbox */
	QCheckBox* m_onlyCustomGenresCheckBox;
	/** List of custom genres */
	StringListEdit* m_genresEdit;
	/** Commands table */
	CommandsTable* m_commandsTable;
	/** Browser line edit */
	QLineEdit* m_browserLineEdit;
	/** Use proxy check box */
	QCheckBox* m_proxyCheckBox;
	/** Proxy line edit */
	QLineEdit* m_proxyLineEdit;
#if QT_VERSION >= 0x040000
	/** Use proxy authentication check box */
	QCheckBox* m_proxyAuthenticationCheckBox;
	/** Proxy user name line edit */
	QLineEdit* m_proxyUserNameLineEdit;
	/** Proxy password line edit */
	QLineEdit* m_proxyPasswordLineEdit;
#endif
#ifndef CONFIG_USE_KDE
	QCheckBox* m_useApplicationFontCheckBox;
	QPushButton* m_applicationFontButton;
	QCheckBox* m_useApplicationStyleCheckBox;
	QComboBox* m_applicationStyleComboBox;
	QFont m_font;
	QString m_style;
	bool m_fontChanged;
	bool m_styleChanged;
#endif
};

#endif
