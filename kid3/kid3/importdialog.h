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
		ASD_Discogs,
		ASD_MusicBrainzRelease,
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
	 * Get import destination.
	 *
	 * @return true if ID3v1 is destination,
	 *         false if ID3v2.
	 */
	bool getDestV1() const;

public slots:
	/**
	 * Shows the dialog as a modal dialog.
	 */
	int exec();

private slots:
	/**
	 * Show help.
	 */
	void showHelp();

private:
	AutoStartSubDialog m_autoStartSubDialog;
	/** import selector widget */
	ImportSelector *impsel;
	ImportTrackDataVector& m_trackDataVector;
};

#endif
