/**
 * \file generalconfig.h
 * General configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 */

#ifndef GENERALCONFIG_H
#define GENERALCONFIG_H

#include "config.h"
#ifdef CONFIG_USE_KDE
class KConfig;
#else
#if QT_VERSION >= 300
class QSettings;
#endif
#endif
#include <qstringlist.h>

#if !defined CONFIG_USE_KDE && (QT_VERSION < 300)
#include <qmap.h>

/** Quick and dirty replacement for QSettings */
class QSettings {
public:
	enum Scope { User, Global };
	QSettings();
	~QSettings();
	void setPath(const QString &, const QString &, Scope = Global);
	void beginGroup(const QString &grp);
	void endGroup();
	void writeEntry(const QString &key, int val);
	void writeEntry(const QString &key, bool val);
	void writeEntry(const QString &key, const QString &val);
	void writeEntry(const QString &key, const QStringList &val);
	void writeEntry(const QString &key, const QMap<QString, QString> &val);
	QString readEntry(const QString &key, const QString &dflt = QString::null);
	int readNumEntry(const QString &key, int dflt = 0);
	bool readBoolEntry(const QString &key, bool dflt = 0);
	QStringList readListEntry(const QString &key);
	QMap<QString, QString> readMapEntry(const QString &key, const QMap<QString, QString> &dflt);
private:
	QMap<QString, QString> map;
	QString group;
};
#endif

/**
 * General configuration.
 */
class GeneralConfig {
public:
	/**
	 * Constructor.
	 * Set default configuration.
	 */
	GeneralConfig();
	/**
	 * Persist configuration.
	 *
	 * @param config KDE configuration
	 */
	void writeToConfig(
#ifdef CONFIG_USE_KDE
		KConfig *config
#else
		QSettings *config
#endif
		) const;
	/**
	 * Read persisted configuration.
	 *
	 * @param config KDE configuration
	 */
	void readFromConfig(
#ifdef CONFIG_USE_KDE
		KConfig *config
#else
		QSettings *config
#endif
		);
	/** true to enable formating in line edits */
	bool formatWhileEditing;
	/** true to import into ID3v1 tags, else into ID3v2 tags */
	bool importDestV1;
	/** Names of import formats */
	QStringList importFormatNames;
	/** regexp describing header import format */
	QStringList importFormatHeaders;
	/** regexp describing track import format */
	QStringList importFormatTracks;
	/** selected import format */
	int importFormatIdx;
};

#endif
