/**
 * \file clierror.h
 * Error codes for CLI.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 28 Jul 2019
 *
 * Copyright (C) 2019  Urs Fleisch
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

#pragma once

/**
 * Error codes for CLI.
 */
enum class CliError : int {
  Ok = 0,           /**< No error */
  ApplicationError, /**< Application specific error */
  ParseError,       /**< Request could not be parsed */
  InvalidRequest,   /**< Request is invalid */
  MethodNotFound,   /**< Method does not exist */
  InvalidParams,    /**< Parameters are invalid */
  InternalError,    /**< Internal error */
  Usage             /**< Usage message */
};
