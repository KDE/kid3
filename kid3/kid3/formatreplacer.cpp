/**
 * \file formatreplacer.cpp
 * Replaces format codes in a string.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 06 Jul 2008
 */

#include "formatreplacer.h"
#include <qurl.h>
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param str string with format codes
 */
FormatReplacer::FormatReplacer(const QString& str) : m_str(str) {}

/**
 * Destructor.
 */
FormatReplacer::~FormatReplacer() {}

/**
 * Replace escaped characters.
 * Replaces the escaped characters ("\n", "\t", "\r", "\\", "\a", "\b",
 * "\f", "\v") with the corresponding characters.
 */
void FormatReplacer::replaceEscapedChars()
{
	if (!m_str.isEmpty()) {
		const int numEscCodes = 8;
		const QChar escCode[numEscCodes] = {
			'n', 't', 'r', '\\', 'a', 'b', 'f', 'v'};
		const char escChar[numEscCodes] = {
			'\n', '\t', '\r', '\\', '\a', '\b', '\f', '\v'};

		for (int pos = 0; pos < static_cast<int>(m_str.length());) {
			pos = m_str.QCM_indexOf('\\', pos);
			if (pos == -1) break;
			++pos;
			for (int k = 0;; ++k) {
				if (k >= numEscCodes) {
					// invalid code at pos
					++pos;
					break;
				}
				if (m_str[pos] == escCode[k]) {
					// code found, replace it
					m_str.replace(pos - 1, 2, escChar[k]);
					break;
				}
			}
		}
	}
}

/**
 * Replace percent codes.
 *
 * @param flags flags: FSF_SupportUrlEncode to support modifier u
 *              (with code c "%uc") to URL encode,
 *              FSF_ReplaceSeparators to replace directory separators
 *              ('/', '\\', ':') in tags.
 */
void FormatReplacer::replacePercentCodes(unsigned flags)
{
	if (!m_str.isEmpty()) {
		for (int pos = 0; pos < static_cast<int>(m_str.length());) {
			pos = m_str.QCM_indexOf('%', pos);
			if (pos == -1) break;

			int codePos = pos + 1;
			int codeLen = 0;
			bool urlEncode = false;
			QString repl;
			if ((flags & FSF_SupportUrlEncode) && m_str[codePos] == 'u') {
				++codePos;
				urlEncode = true;
			}
			if (m_str[codePos] == '{') {
				int closingBracePos = m_str.QCM_indexOf('}', codePos + 1);
				if (closingBracePos > codePos + 1) {
					QString longCode =
						m_str.mid(codePos + 1, closingBracePos - codePos - 1).QCM_toLower();
					repl = getReplacement(longCode);
					codeLen = closingBracePos - pos + 1;
				}
			} else {
				repl = getReplacement(QString(m_str[codePos]));
				codeLen = codePos - pos + 1;
			}

			if (codeLen > 0) {
				if (flags & FSF_ReplaceSeparators) {
					repl.replace('/', '-');
					repl.replace('\\', '-');
					repl.replace(':', '-');
				}
				if (urlEncode) {
					QCM_QUrl_encode(repl);
				}
				if (!repl.isNull() || codeLen > 2) {
					m_str.replace(pos, codeLen, repl);
					pos += repl.length();
				} else {
					++pos;
				}
			} else {
				++pos;
			}
		}
	}
}
