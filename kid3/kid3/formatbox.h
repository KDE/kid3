/**
 * \file formatbox.h
 * Group box containing format options.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 */

#ifndef FORMATBOX_H
#define FORMATBOX_H

#include <qgroupbox.h>

class QComboBox;
class QCheckBox;
class QTable;
class QString;
class FormatConfig;

/**
 * Group box containing format options.
 */
class FormatBox : public QGroupBox
{
Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * @param title  title
	 * @param parent parent widget
	 * @param name   Qt object name
	 */
	FormatBox(const QString & title, QWidget *parent = 0, const char *name = 0);
	/**
	 * Destructor.
	 */
	~FormatBox();
	/**
	 * Set the values from a format configuration.
	 *
	 * @param cfg format configuration
	 */
	void fromFormatConfig(const FormatConfig *cfg);
	/**
	 * Store the values in a format configuration.
	 *
	 * @param cfg format configuration
	 */
	void toFormatConfig(FormatConfig *cfg) const;
public slots:
	/**
	 * Called when a value in the string replacement table is changed.
	 * If the first cell in the last row is changed to a non-empty
	 * value, a new row is added. If it is changed to an empty value,
	 * the row is deleted.
	 *
	 * @param row table row of changed item
	 * @param col table column of changed item
	 */
	void valueChanged(int row, int col);
private:
	QComboBox *caseConvComboBox;
	QCheckBox *strRepCheckBox;
	QTable *strReplTable;
};

#endif
