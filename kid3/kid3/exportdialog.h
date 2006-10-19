/**
 * \file exportdialog.h
 * Export dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 May 2006
 */

#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <qdialog.h>
#include <qstringlist.h>
#include "importtrackdata.h"

class QTextEdit;
class QLineEdit;
class QPushButton;
class QComboBox;

/**
 * Export dialog.
 */
class ExportDialog : public QDialog {
Q_OBJECT

public:
	/** Export source */
	enum Source { SrcV1, SrcV2 };

	/**
	 * Constructor.
	 *
	 * @param parent  parent widget
	 */
	ExportDialog(QWidget* parent);

	/**
	 * Destructor.
	 */
	~ExportDialog();

	/**
	 * Read the local settings from the configuration.
	 */
	void readConfig();

	/**
	 * Set data to be exported.
	 *
	 * @param trackDataVector data to export
	 */
	void setExportData(const ImportTrackDataVector& trackDataVector);

signals:
	/**
	 * Emitted when new export data has to be provided
	 * Parameter: SrcV1 for ID3v1 data, SrcV2 for ID3v2 data
	 */
	void exportDataRequested(int);

private slots:
	/**
	 * Export to a file.
	 */
	void slotToFile();

	/**
	 * Export to clipboard.
	 */
	void slotToClipboard();

	/**
	 * Set the format lineedits to the format selected in the combo box.
	 *
	 * @param index current index of the combo box
	 */
	void setFormatLineEdit(int index);

	/**
	 * Show exported text as preview in editor.
	 */
	void showPreview();

	/**
	 * Save the local settings to the configuration.
	 */
	void saveConfig();

	/**
	 * Show help.
	 */
	void showHelp();

private:
  /** Text editor */
  QTextEdit* m_edit;
	/** cobobox with formats */
	QComboBox* m_formatComboBox;
	/** LineEdit for header */
	QLineEdit* m_headerLineEdit;
	/** LineEdit for track */
	QLineEdit* m_trackLineEdit;
	/** LineEdit for trailer */
	QLineEdit* m_trailerLineEdit;
	/** To File button */
	QPushButton* m_fileButton;
	/** To Clipboard button */
	QPushButton* m_clipButton;
	/** combobox with export sources */
	QComboBox* m_srcComboBox;
	/** header formats */
	QStringList m_formatHeaders;
	/** track formats */
	QStringList m_formatTracks;
	/** trailer formats */
	QStringList m_formatTrailers;

	/** data to export */
	ImportTrackDataVector m_trackDataVector;
};

#endif
