/**
 * \file expressionparser.h
 * Simple parser for expressions.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 23 Jan 2008
 *
 * Copyright (C) 2008  Urs Fleisch
 *
 * This file is part of Kid3.
 *
 * Kid3 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Kid3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include <qstring.h>
#include <qstringlist.h>

/**
 * Simple parser for expressions with boolean not, and, or and
 * other binary operations.
 */
class ExpressionParser {
public:
	/**
	 * Constructor.
	 *
	 * @param operators additional operators (besides not, and, or),
	 *                  highest priority first
	 */
	ExpressionParser(QStringList operators);

	/**
	 * Destructor.
	 */
	~ExpressionParser();

	/**
	 * Tokenize an expression in reverse polish notation.
	 *
	 * @param expr with strings, operators, not, and, or, (, ).
	 */
	void tokenizeRpn(const QString& expr);

	/**
	 * Clear the variable stack before restarting an evaluation.
	 */
	void clearEvaluation();

	/**
	 * Evaluate the RPN stack.
	 * Boolean operations and, or, not are performed automatically. If another
	 * operation has to be performed, the method stops and returns operator
	 * and variables. The result can then be pushed onto the stack using
	 * pushBool() and then the method can be called again.
	 *
	 * @param op the operator is returned here
	 * @param var1 the first variable is returned here
	 * @param var2 the second variable is returned here
	 *
	 * @return true if the RPN stack has more to evaluate,
	 *         if false, the evaluation is finished.
	 */
	bool evaluate(QString& op, QString& var1, QString& var2);

	/**
	 * Push a boolean to the variable stack.
	 * Can be used to push the result of the operation returned by
	 * evaluate() back onto the variable stack.
	 *
	 * @param var boolean to  push
	 */
	void pushBool(bool var);

	/**
	 * Check if an error occurred.
	 * @return true if an error occurred.
	 */
	bool hasError() const {	return m_error; }

	/**
	 * Pop a boolean from the variable stack.
	 * Can be used to get the result after evaluate() returns false and
	 * no error occurred.
	 *
	 * @param var the boolean is returned here
	 *
	 * @return true if ok.
	 */
	bool popBool(bool& var);

private:
	/**
	 * Get the next token from the RPN stack.
	 *
	 * @return token, QString::null if stack is empty.
	 */
	QString getToken();

	/**
	 * Compare operator priority.
	 *
	 * @return true if op1 has less priority than op2.
	 */
	bool lessPriority(const QString& op1, const QString& op2) const;

	/**
	 * Pop two booleans from the variable stack.
	 *
	 * @param var1 first boolean
	 * @param var2 second boolean
	 *
	 * @return true if ok.
	 */
	bool popTwoBools(bool& var1, bool& var2);

	QStringList m_rpnStack;
	QStringList m_varStack;
	const QStringList m_operators;
	QStringList::const_iterator m_rpnIterator;
	bool m_error;
};

#endif
