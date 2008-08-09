/**
 * \file formatreplacer.h
 * Replaces format codes in a string.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 06 Jul 2008
 */

#ifndef FORMATREPLACER_H
#define FORMATREPLACER_H

#include <qstring.h>

/**
 * Replaces format codes in a string.
 */
class FormatReplacer {
public:
	/** Flags for replacePercentCodes(). */
	enum FormatStringFlags {
		FSF_SupportUrlEncode  = (1 << 0),
		FSF_ReplaceSeparators = (1 << 1)
	};

	/**
	 * Constructor.
	 *
	 * @param str string with format codes
	 */
	explicit FormatReplacer(const QString& str = QString());

	/**
	 * Destructor.
	 */
	virtual ~FormatReplacer();

	/**
	 * Set string with format codes.
	 * @param str string with format codes
	 */
	void setString(const QString& str) { m_str = str; }

	/**
	 * Get string.
	 * The string set with setString() can be modified using
	 * replaceEscapedChars() and replacePercentCodes().
	 * @return string.
	 */
	QString getString() const { return m_str; }

	/**
	 * Replace escaped characters.
	 * Replaces the escaped characters ("\n", "\t", "\r", "\\", "\a", "\b",
	 * "\f", "\v") with the corresponding characters.
	 */
	void replaceEscapedChars();

	/**
	 * Replace percent codes.
	 *
	 * @param flags flags: FSF_SupportUrlEncode to support modifier u
	 *              (with code c "%uc") to URL encode,
	 *              FSF_ReplaceSeparators to replace directory separators
	 *              ('/', '\\', ':') in tags.
	 */
	void replacePercentCodes(unsigned flags = 0);

protected:
	/**
	 * Replace a format code (one character %c or multiple characters %{chars}).
	 *
	 * @param code format code
	 *
	 * @return replacement string,
	 *         QString::null if code not found.
	 */
	virtual QString getReplacement(const QString& code) const = 0;

private:
	QString m_str;
};

#endif // FORMATREPLACER_H
