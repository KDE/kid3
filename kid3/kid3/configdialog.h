/**
 * \file configdialog.h
 * Configuration dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
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

#ifdef KID3_USE_KCONFIGDIALOG
#include <kconfigdialog.h>
#else
#include <qtabdialog.h>
#endif
#include "formatconfig.h"

class FormatBox;
class QCheckBox;
class FormatConfig;
class MiscConfig;
class CommandsTable;
class QString;
class QWidget;
class QComboBox;
class QLineEdit;

#ifdef KID3_USE_KCONFIGDIALOG
typedef KConfigDialog ConfigDialogBaseClass;
#else
typedef QTabDialog ConfigDialogBaseClass;
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
	void setConfig(const FormatConfig *fnCfg, const FormatConfig *id3Cfg,
				   const MiscConfig *miscCfg);
	/**
	 * Get values from dialog and store them in the current configuration.
	 *
	 * @param fnCfg   filename format configuration
	 * @param id3Cfg  ID3 format configuration
	 * @param miscCfg misc. configuration
	 */
	void getConfig(FormatConfig *fnCfg, FormatConfig *id3Cfg,
				   MiscConfig *miscCfg) const;

protected slots:
	/**
	 * Show help.
	 */
	virtual void slotHelp();

private:
	/** Preserve timestamp checkbox */
	QCheckBox* m_preserveTimeCheckBox;
	/** Use track/total number of tracks format checkbox */
	QCheckBox* m_totalNumTracksCheckBox;
	/** Comment field name combo box */
	QComboBox* m_commentNameComboBox;
	/** ID3v2 version combo box */
	QComboBox* m_id3v2VersionComboBox;
	/** Filename Format box */
	FormatBox *fnFormatBox;
	/** ID3 Format box */
	FormatBox *id3FormatBox;
	/** Commands table */
	CommandsTable* m_commandsTable;
	/** Use proxy check box */
	QCheckBox* m_proxyCheckBox;
	/** Proxy line edit */
	QLineEdit* m_proxyLineEdit;
};

#endif
