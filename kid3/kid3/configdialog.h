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
#include <kdialogbase.h>
#else
#include <qdialog.h>
#endif
#include "formatconfig.h"

class FormatBox;
class QCheckBox;
class FormatConfig;
class GeneralConfig;
class QString;
class QWidget;

/**
 * Configuration dialog.
 */
#ifdef CONFIG_USE_KDE
class ConfigDialog : public KDialogBase
#else
class ConfigDialog : public QDialog
#endif
{
public:
	/**
	 * Constructor.
	 *
	 * @param parent  parent widget
	 * @param caption dialog title
	 */
	ConfigDialog(QWidget *parent, QString &caption);
	/**
	 * Destructor.
	 */
	~ConfigDialog();
	/**
	 * Set values in dialog from current configuration.
	 *
	 * @param fnCfg  filename format configuration
	 * @param fnCfg  ID3 format configuration
	 * @param genCfg general configuration
	 */
	void setConfig(const FormatConfig *fnCfg, const FormatConfig *id3Cfg,
				   const GeneralConfig *genCfg);
	/**
	 * Get values from dialog and store them in the current configuration.
	 *
	 * @param fnCfg  filename format configuration
	 * @param fnCfg  ID3 format configuration
	 * @param genCfg general configuration
	 */
	void getConfig(FormatConfig *fnCfg, FormatConfig *id3Cfg,
				   GeneralConfig *genCfg) const;
private:
	/** Format while editing checkbox */
	QCheckBox *formatEditingCheckBox;
	/** Filename Format box */
	FormatBox *fnFormatBox;
	/** ID3 Format box */
	FormatBox *id3FormatBox;
};

#endif
