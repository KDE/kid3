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
#include <qstring.h>

#if !defined CONFIG_USE_KDE && (QT_VERSION < 300)
#include <qmap.h>
#include <qstringlist.h>

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
 * Abstract base class for configurations.
 */
class GeneralConfig {
public:
	/**
	 * Constructor.
	 * Set default configuration.
	 *
	 * @param grp configuration group
	 */
	GeneralConfig(const QString &grp);
	/**
	 * Destructor.
	 */
	virtual ~GeneralConfig();
	/**
	 * Persist configuration.
	 *
	 * @param config KDE configuration
	 */
	virtual void writeToConfig(
#ifdef CONFIG_USE_KDE
		KConfig *config
#else
		QSettings *config
#endif
		) const = 0;
	/**
	 * Read persisted configuration.
	 *
	 * @param config KDE configuration
	 */
	virtual void readFromConfig(
#ifdef CONFIG_USE_KDE
		KConfig *config
#else
		QSettings *config
#endif
		) = 0;
protected:
	QString group;
};

#endif
