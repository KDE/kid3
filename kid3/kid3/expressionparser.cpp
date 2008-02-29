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
 *
 * The RPN tokenizer is based on ExprEvaluator,
 * Copyright (C) 2004 the VideoLAN team, under the same license.
 */

#include "expressionparser.h"
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param operators additional operators (besides not, and, or),
 *                  highest priority first
 */
ExpressionParser::ExpressionParser(QStringList operators) :
	m_operators(operators << "not" << "and" << "or"),
	m_error(false)
{
}

/**
 * Destructor.
 */
ExpressionParser::~ExpressionParser()
{
}

/**
 * Compare operator priority.
 *
 * @return true if op1 has less priority than op2.
 */
bool ExpressionParser::lessPriority(const QString& op1,
																		const QString& op2) const
{
#if QT_VERSION >= 0x040000
	int index1 = m_operators.indexOf(op1);
	int index2 = m_operators.indexOf(op2);
#else
	int index1 = m_operators.findIndex(op1);
	int index2 = m_operators.findIndex(op2);
#endif
	if (op1 == "(") return true;
	if (index1 >= 0 && index2 >= 0) return index1 >= index2;
	return false;
}

/**
 * Tokenize an expression in reverse polish notation.
 *
 * @param expr with strings, operators, not, and, or, (, ).
 */
void ExpressionParser::tokenizeRpn(const QString& expr)
{
	m_rpnStack.clear();

	QStringList operatorStack;
	QString token;
	int begin = 0, end = 0, len = expr.length();
	while (begin < len) {
		// skip spaces
		while (expr[begin] == ' ') {
			++begin;
		}

		if (expr[begin] == '(') {
			// push '(' on operator stack and continue
			operatorStack.push_back("(");
			++begin;
		} else if (expr[begin] == ')') {
			// after ')', pop operator stack until '(' is found
			while (!operatorStack.empty()) {
				QString lastOp = operatorStack.back();
				operatorStack.pop_back();
				if (lastOp == "(") {
					break;
				}
				m_rpnStack.push_back(lastOp);
			}
			++begin;
		} else {
			if (expr[begin] == '"') {
				// skip quoted string
				end = begin + 1;
				while (end < len &&
							 !(expr[end] == '"' && (end <= 0 || expr[end - 1] != '\\'))) {
					++end;
				}
				token = expr.mid(begin + 1, end - begin - 1);
				token.replace("\\\"", "\"");
				begin = end + 1;
			} else {
				// skip spaces
				end = begin;
				while (end < len && expr[end] != ' ' && expr[end] != ')') {
					++end;
				}
				token = expr.mid(begin, end - begin);
				begin = end;
			}
			if (m_operators.contains(token)) {
				// pop the operator stack while the token has lower priority
				while (!operatorStack.empty() &&
							 lessPriority(token, operatorStack.back())) {
					QString lastOp = operatorStack.back();
					operatorStack.pop_back();
					m_rpnStack.push_back(lastOp);
				}
				operatorStack.push_back(token);
			} else {
				m_rpnStack.push_back(token);
			}
		}
	}
	// pop operator stack
	while (!operatorStack.empty()) {
		QString lastOp = operatorStack.back();
		operatorStack.pop_back();
		m_rpnStack.push_back(lastOp);
	}
	m_rpnIterator = m_rpnStack.begin();
}

/**
 * Get the next token from the RPN stack.
 *
 * @return token, QString::null if stack is empty.
 */
QString ExpressionParser::getToken()
{
	if (!m_rpnStack.empty()) {
		QString token = m_rpnStack.front();
		m_rpnStack.pop_front();
		return token;
	}
	return QString::null;
}


/**
 * Convert a string to a boolean.
 *
 * @param str string
 * @param b   the boolean is returned here
 *
 * @return true if ok.
 */
static bool stringToBool(const QString& str, bool& b)
{
	if (str == "1" || str == "true" || str == "on" || str == "yes") {
		b = true;
		return true;
	} else if (str == "0" || str == "false" || str == "off" || str == "no") {
		b = false;
		return true;
	}
	return false;
}

/**
 * Convert a boolean to a string.
 *
 * @param b boolean to convert
 *
 * @return "1" or "0".
 */
static QString boolToString(bool b)
{
	return b ? "1" : "0";
}

/**
 * Clear the variable stack before restarting an evaluation.
 */
void ExpressionParser::clearEvaluation()
{
	m_rpnIterator = m_rpnStack.begin();
	m_varStack.clear();
	m_error = false;
}

/**
 * Pop a boolean from the variable stack.
 * Can be used to get the result after evaluate() returns false and
 * no error occurred.
 *
 * @param var the boolean is returned here
 *
 * @return true if ok.
 */
bool ExpressionParser::popBool(bool& var)
{
	if (m_varStack.empty() || !stringToBool(m_varStack.back(), var)) {
		return false;
	}
	m_varStack.pop_back();
	return true;
}

/**
 * Push a boolean to the variable stack.
 * Can be used to push the result of the operation returned by
 * evaluate() back onto the variable stack.
 *
 * @param var boolean to  push
 */
void ExpressionParser::pushBool(bool var)
{
	m_varStack.push_back(boolToString(var));
}

/**
 * Pop two booleans from the variable stack.
 *
 * @param var1 first boolean
 * @param var2 second boolean
 *
 * @return true if ok.
 */
bool ExpressionParser::popTwoBools(bool& var1, bool& var2)
{
	if (m_varStack.empty() || !stringToBool(m_varStack.back(), var1)) {
		return false;
	}
	m_varStack.pop_back();
	if (m_varStack.empty() || !stringToBool(m_varStack.back(), var2)) {
		return false;
	}
	m_varStack.pop_back();
	return true;
}

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
bool ExpressionParser::evaluate(QString& op, QString& var1, QString& var2)
{
	while (m_rpnIterator != m_rpnStack.end()) {
		QString token = *m_rpnIterator++;
		if (token == "and") {
			bool var1, var2;
			if (!popTwoBools(var1, var2)) {
				m_error = true;
				break;
			}
			pushBool(var1 && var2);
		} else if (token == "or") {
			bool var1, var2;
			if (!popTwoBools(var1, var2)) {
				m_error = true;
				break;
			}
			pushBool(var1 || var2);
		} else if (token == "not") {
			bool var;
			if (!popBool(var)) {
				m_error = true;
				break;
			}
			pushBool(!var);
		} else if (m_operators.contains(token)) {
			if (m_varStack.empty()) {
				m_error = true;
				break;
			}
			var1 = m_varStack.back();
			m_varStack.pop_back();
			if (m_varStack.empty()) {
				m_error = true;
				break;
			}
			var2 = m_varStack.back();
			m_varStack.pop_back();
			op = token;
			return true;
		} else {
			m_varStack.push_back(token);
		}
	}
	return false;
}
