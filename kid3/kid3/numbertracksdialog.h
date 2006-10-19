/**
 * \file numbertracksdialog.h
 * Number tracks dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 May 2006
 */

#ifndef NUMBERTRACKSDIALOG_H
#define NUMBERTRACKSDIALOG_H

#include <qdialog.h>

class QSpinBox;
class QComboBox;

/**
 * Number tracks dialog.
 */
class NumberTracksDialog : public QDialog {
Q_OBJECT
public:
	/**
	 * Constructor.
	 *
	 * @param parent parent widget
	 */
	NumberTracksDialog(QWidget* parent);

	/**
	 * Destructor.
	 */
	~NumberTracksDialog();

	/**
	 * Get start number.
	 */
	int getStartNumber() const;

	/**
	 * Get destination.
	 *
	 * @return true if ID3v1 is destination,
	 *         false if ID3v2.
	 */
	bool getDestV1() const;

private slots:
	/**
	 * Show help.
	 */
	void showHelp();

private:
	/** Destinations */
	enum Destination { DestV1, DestV2 };

	/** spinbox with starting track number */
	QSpinBox* m_trackSpinBox;
	/** combobox with destination */
	QComboBox* m_destComboBox;
};

#endif
