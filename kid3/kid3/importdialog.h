/**
 * \file importdialog.h
 * Import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 */

#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include "config.h"
#include "importtrackdata.h"
#include <qdialog.h>

class ImportSelector;
class FreedbConfig;
class QCheckBox;
class QSpinBox;
#ifdef HAVE_TUNEPIMP
class MusicBrainzConfig;
#endif

/**
 * Import dialog.
 */
class ImportDialog : public QDialog {
Q_OBJECT

public:
	/**
	 * Sub-Dialog to be started automatically.
	 */
	enum AutoStartSubDialog {
		ASD_None,
		ASD_Freedb,
		ASD_MusicBrainz
	};

	/**
	 * Constructor.
	 *
	 * @param parent        parent widget
	 * @param caption       dialog title
	 * @param trackDataList track data to be filled with imported values,
	 *                      is passed with durations of files set
	 */
	ImportDialog(QWidget *parent, QString &caption,
							 ImportTrackDataVector& trackDataList);
	/**
	 * Destructor.
	 */
	~ImportDialog();

	/**
	 * Set dialog to be started automatically.
	 *
	 * @param asd dialog to be started
	 */
	void setAutoStartSubDialog(AutoStartSubDialog asd) {
		m_autoStartSubDialog = asd;
	}

	/**
	 * Clear dialog data.
	 */
	void clear();
	/**
	 * Set ID3v1 or ID3v2 tags as destination.
	 *
	 * @param v1 true to set ID3v1, false for ID3v2
	 */
	void setDestV1(bool v1);
	/**
	 * Get import destination.
	 *
	 * @return true if ID3v1 is destination,
	 *         false if ID3v2.
	 */
	bool getDestV1() const;
	/**
	 * Set import format regexp.
	 *
	 * @param names   import format names list
	 * @param headers import format header regexps
	 * @param tracks  import format track regexps
	 * @param idx     selected index
	 */
	void setImportFormat(const QStringList &names,
						 const QStringList &headers,
						 const QStringList &tracks,
						 int idx);
	/**
	 * Get import format regexp.
	 *
	 * @param name   import format name
	 * @param header import format header regexp
	 * @param track  import format track regexp
	 *
	 * @return index of current selection.
	 */
	int getImportFormat(QString &name,
						QString &header,
						QString &track) const;
	/**
	 * Set time difference check configuration.
	 *
	 * @param enable  true to enable check
	 * @param maxDiff maximum allowable time difference
	 */ 
	void setTimeDifferenceCheck(bool enable, int maxDiff);
	/**
	 * Get time difference check configuration.
	 *
	 * @param enable  true if check is enabled
	 * @param maxDiff maximum allowed time difference
	 */ 
	void getTimeDifferenceCheck(bool& enable, int& maxDiff) const;
	/**
	 * Set freedb.org configuration.
	 *
	 * @param cfg freedb configuration.
	 */
	void setFreedbConfig(const FreedbConfig *cfg);
	/**
	 * Get freedb.org configuration.
	 *
	 * @param cfg freedb configuration.
	 */
	void getFreedbConfig(FreedbConfig *cfg) const;
#ifdef HAVE_TUNEPIMP
	/**
	 * Set MusicBrainz configuration.
	 *
	 * @param cfg musicBrainz configuration.
	 */
	void setMusicBrainzConfig(const MusicBrainzConfig* cfg);
	/**
	 * Get MusicBrainz configuration.
	 *
	 * @param cfg MusicBrainz configuration.
	 */
	void getMusicBrainzConfig(MusicBrainzConfig* cfg) const;
#endif

public slots:
	/**
	 * Shows the dialog as a modal dialog.
	 */
	int exec();

private:
	AutoStartSubDialog m_autoStartSubDialog;
	/** import selector widget */
	ImportSelector *impsel;
	ImportTrackDataVector& m_trackDataVector;
};

#endif
