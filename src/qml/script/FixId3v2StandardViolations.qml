/**
 * \file FixId3v2StandardViolations.qml
 * Fix standard violations in ID3v2 frames to conform to the specification.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 01 Oct 2022
 *
 * Copyright (C) 2022  Urs Fleisch
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import Kid3 1.1

Kid3Script {
  onRun: {
    function frameModelRowToString(model, row) {
      var values = []
      var roleNames = ["name", "value", "internalName", "fieldIds", "fieldValues", "notice"]
      for (var i = 0; i < roleNames.length; ++i) {
        var roleName = roleNames[i]
        values.push(roleName + "=" + script.getRoleData(model, row, roleName))
      }
      return "" + row + ": " + values.join(", ")
    }

    function doNothing(model, row) {
      return {
        fixed: false,
        message: ""
      }
    }

    // ISO-639-2 language codes as found on
    // https://www.loc.gov/standards/iso639-2/php/code_list.php
    var languageCodes = ["aar", "abk", "ace", "ach", "ada", "ady", "afa",
    "afh", "afr", "ain", "aka", "akk", "alb", "sqi", "ale", "alg", "alt",
    "amh", "ang", "anp", "apa", "ara", "arc", "arg", "arm", "hye", "arn",
    "arp", "art", "arw", "asm", "ast", "ath", "aus", "ava", "ave", "awa",
    "aym", "aze", "bad", "bai", "bak", "bal", "bam", "ban", "baq", "eus",
    "bas", "bat", "bej", "bel", "bem", "ben", "ber", "bho", "bih", "bik",
    "bin", "bis", "bla", "bnt", "tib", "bod", "bos", "bra", "bre", "btk",
    "bua", "bug", "bul", "bur", "mya", "byn", "cad", "cai", "car", "cat",
    "cau", "ceb", "cel", "cze", "ces", "cha", "chb", "che", "chg", "chi",
    "zho", "chk", "chm", "chn", "cho", "chp", "chr", "chu", "chv", "chy",
    "cmc", "cnr", "cop", "cor", "cos", "cpe", "cpf", "cpp", "cre", "crh",
    "crp", "csb", "cus", "wel", "cym", "cze", "ces", "dak", "dan", "dar",
    "day", "del", "den", "ger", "deu", "dgr", "din", "div", "doi", "dra",
    "dsb", "dua", "dum", "dut", "nld", "dyu", "dzo", "efi", "egy", "eka",
    "gre", "ell", "elx", "eng", "enm", "epo", "est", "baq", "eus", "ewe",
    "ewo", "fan", "fao", "per", "fas", "fat", "fij", "fil", "fin", "fiu",
    "fon", "fre", "fra", "fre", "fra", "frm", "fro", "frr", "frs", "fry",
    "ful", "fur", "gaa", "gay", "gba", "gem", "geo", "kat", "ger", "deu",
    "gez", "gil", "gla", "gle", "glg", "glv", "gmh", "goh", "gon", "gor",
    "got", "grb", "grc", "gre", "ell", "grn", "gsw", "guj", "gwi", "hai",
    "hat", "hau", "haw", "heb", "her", "hil", "him", "hin", "hit", "hmn",
    "hmo", "hrv", "hsb", "hun", "hup", "arm", "hye", "iba", "ibo", "ice",
    "isl", "ido", "iii", "ijo", "iku", "ile", "ilo", "ina", "inc", "ind",
    "ine", "inh", "ipk", "ira", "iro", "ice", "isl", "ita", "jav", "jbo",
    "jpn", "jpr", "jrb", "kaa", "kab", "kac", "kal", "kam", "kan", "kar",
    "kas", "geo", "kat", "kau", "kaw", "kaz", "kbd", "kha", "khi", "khm",
    "kho", "kik", "kin", "kir", "kmb", "kok", "kom", "kon", "kor", "kos",
    "kpe", "krc", "krl", "kro", "kru", "kua", "kum", "kur", "kut", "lad",
    "lah", "lam", "lao", "lat", "lav", "lez", "lim", "lin", "lit", "lol",
    "loz", "ltz", "lua", "lub", "lug", "lui", "lun", "luo", "lus", "mac",
    "mkd", "mad", "mag", "mah", "mai", "mak", "mal", "man", "mao", "mri",
    "map", "mar", "mas", "may", "msa", "mdf", "mdr", "men", "mga", "mic",
    "min", "mis", "mac", "mkd", "mkh", "mlg", "mlt", "mnc", "mni", "mno",
    "moh", "mon", "mos", "mao", "mri", "may", "msa", "mul", "mun", "mus",
    "mwl", "mwr", "bur", "mya", "myn", "myv", "nah", "nai", "nap", "nau",
    "nav", "nbl", "nde", "ndo", "nds", "nep", "new", "nia", "nic", "niu",
    "dut", "nld", "nno", "nob", "nog", "non", "nor", "nqo", "nso", "nub",
    "nwc", "nya", "nym", "nyn", "nyo", "nzi", "oci", "oji", "ori", "orm",
    "osa", "oss", "ota", "oto", "paa", "pag", "pal", "pam", "pan", "pap",
    "pau", "peo", "per", "fas", "phi", "phn", "pli", "pol", "pon", "por",
    "pra", "pro", "pus", "que", "raj", "rap", "rar", "roa", "roh", "rom",
    "rum", "ron", "rum", "ron", "run", "rup", "rus", "sad", "sag", "sah",
    "sai", "sal", "sam", "san", "sas", "sat", "scn", "sco", "sel", "sem",
    "sga", "sgn", "shn", "sid", "sin", "sio", "sit", "sla", "slo", "slk",
    "slo", "slk", "slv", "sma", "sme", "smi", "smj", "smn", "smo", "sms",
    "sna", "snd", "snk", "sog", "som", "son", "sot", "spa", "alb", "sqi",
    "srd", "srn", "srp", "srr", "ssa", "ssw", "suk", "sun", "sus", "sux",
    "swa", "swe", "syc", "syr", "tah", "tai", "tam", "tat", "tel", "tem",
    "ter", "tet", "tgk", "tgl", "tha", "tib", "bod", "tig", "tir", "tiv",
    "tkl", "tlh", "tli", "tmh", "tog", "ton", "tpi", "tsi", "tsn", "tso",
    "tuk", "tum", "tup", "tur", "tut", "tvl", "twi", "tyv", "udm", "uga",
    "uig", "ukr", "umb", "und", "urd", "uzb", "vai", "ven", "vie", "vol",
    "vot", "wak", "wal", "war", "was", "wel", "cym", "wen", "wln", "wol",
    "xal", "xho", "yao", "yap", "yid", "yor", "ypk", "zap", "zbl", "zen",
    "zgh", "zha", "chi", "zho", "znd", "zul", "zun", "zxx", "zza"]
    // ID3v2 frames containing a language field
    var consideredFrames = ["COMM", "USLT", "SYLT", "USER"]

    /**
     * Convert to valid ISO-639-2 language code.
     * The conversion just transforms to lower case. A more sophisticated
     * conversion could convert ISO-639-1 to ISO-639-2.
     * @param lang existing language code
     * @return language code, "XXX" if invalid or unknown, "" if not existing.
     */
    function convertToValidLanguageCode(lang) {
      // If lang is empty the frame probably does not exist. An empty language
      // field would be "   " and not "".
      if (!lang) {
        return ""
      }
      if (lang.length === 3 && lang !== "XXX") {
        lang = lang.toLowerCase()
        // qaa-qtz is a range reserved for local use
        if (languageCodes.indexOf(lang) !== -1 ||
            (lang >= "qaa" && lang <= "qtz")) {
          return lang
        }
      }
      return "XXX"
    }

    /**
     * Replace invalid language codes by "XXX".
     * The three byte language field, present in several frames, is used to
     * describe the language of the frame's content, according to ISO-639-2
     * [ISO-639-2]. The language should be represented in lower case. If the
     * language is not known the string "XXX" should be used.
     */
    function fixLanguageCode(model, row) {
      var oldText, newText
      var message = ""
      var fixed = false
      var name = script.getRoleData(model, row, "internalName")
      if (name.startsWith("TLAN")) {
        oldText = script.getRoleData(model, row, "value")
        newText = convertToValidLanguageCode(oldText)
        if (newText !== oldText) {
          app.setFrame(tagv2, name, newText)
          message = "Changed '" + oldText + "' to '" + newText + "'"
          fixed = true
        }
      } else {
        var fieldIds = script.getRoleData(model, row, "fieldIds")
        var fieldIdx = fieldIds.indexOf(Frame.ID_Language)
        if (fieldIdx !== -1) {
          var fieldValues = script.getRoleData(model, row, "fieldValues")
          oldText = fieldValues[fieldIdx]
          newText = convertToValidLanguageCode(oldText)
          if (newText !== oldText) {
            app.setFrame(tagv2, name + ".language", newText)
            message = "Changed '" + oldText + "' to '" + newText + "'"
            fixed = true
          }
        }
      }
      return { fixed, message }
    }

    /**
     * Replace new lines ("\n" or "\r\n") by spaces.
     */
    function fixNlForbidden(model, row) {
      var message = ""
      var fixed = false
      var name = script.getRoleData(model, row, "internalName")
      var oldText = script.getRoleData(model, row, "value")
      var newText = oldText.replace(/[\r\n]+/g, " ")
      if (newText !== oldText) {
        app.setFrame(tagv2, name, newText)
        message = "Changed '" +
            oldText.replace(/\r/g, "\\r").replace(/\n/g, "\\n") +
            "' to '" + newText + "'"
        fixed = true
      }
      return { fixed, message }
    }

    /**
     * Replace "\r\n" by "\n".
     */
    function fixCrForbidden(model, row) {
      var message = ""
      var fixed = false
      var name = script.getRoleData(model, row, "internalName")
      var oldText = script.getRoleData(model, row, "value")
      var newText = oldText.replace(/\r\n/g, "\n")
      if (newText !== oldText) {
        app.setFrame(tagv2, name, newText)
        message = "Changed '" +
            oldText.replace(/\r/g, "\\r").replace(/\n/g, "\\n") +
            "' to '" + newText.replace(/\n/g, "\\n") + "'"
        fixed = true
      }
      return { fixed, message }
    }

    /**
     * Replace empty owner field by "http://www.id3.org/dummy/ufid.html".
     */
    function fixOwnerEmpty(model, row) {
      var message = ""
      var fixed = false
      var fieldIds = script.getRoleData(model, row, "fieldIds")
      var fieldIdx = fieldIds.indexOf(Frame.ID_Owner)
      if (fieldIdx !== -1) {
        var name = script.getRoleData(model, row, "internalName")
        var fieldValues = script.getRoleData(model, row, "fieldValues")
        var oldText = fieldValues[fieldIdx]
        var newText = "http://www.id3.org/dummy/ufid.html"
        if (oldText === "") {
          app.setFrame(tagv2, name + ".owner", newText)
          message = "Changed '" + oldText + "' to '" + newText + "'"
          fixed = true
        }
      }
      return { fixed, message }
    }

    /**
     * Replace with first number found in string.
     */
    function fixNumeric(model, row) {
      var message = ""
      var fixed = false
      var name = script.getRoleData(model, row, "internalName")
      var oldText = script.getRoleData(model, row, "value")
      var newText = oldText.replace(/^[^\d]*(\d+).*$/, "$1")
      if (newText !== oldText) {
        app.setFrame(tagv2, name, newText)
        message = "Changed '" + oldText + "' to '" + newText + "'"
        fixed = true
      }
      return { fixed, message }
    }

    /**
     * Replace with "n1/n2" or just "n1" if numbers found in string.
     */
    function fixNrTotal(model, row) {
      var message = ""
      var fixed = false
      var name = script.getRoleData(model, row, "internalName")
      var oldText = script.getRoleData(model, row, "value")
      var twoNumbersRe = /^[^\d]*(\d+)[^\d]+(\d+).*$/
      var match = twoNumbersRe.exec(oldText)
      var newText = match && match.length === 3 && +match[1] <= +match[2]
          ? match[1] + "/" + match[2]
          : oldText.replace(/^[^\d]*(\d+).*$/, "$1")
      if (newText !== oldText) {
        app.setFrame(tagv2, name, newText)
        message = "Changed '" + oldText + "' to '" + newText + "'"
        fixed = true
      }
      return { fixed, message }
    }

    /**
     * Replace with "ddMM" if valid day and month numbers found in string.
     */
    function fixDayMonth(model, row) {
      var message = ""
      var fixed = false
      var name = script.getRoleData(model, row, "internalName")
      var oldText = script.getRoleData(model, row, "value")
      var twoNumbersRe = /^[^\d]*(\d+)[^\d]+(\d+).*$/
      var match = twoNumbersRe.exec(oldText)
      var newText = oldText
      if (match && match.length === 3) {
        var n1 = +match[1]
        var n2 = +match[2]
        if (n1 >= 1 && n1 <= 31 && n2 >= 1 && n2 <= 12) {
          newText = ("0" + n1).slice(-2) + ("0" + n2).slice(-2)
        } else if (n2 >= 1 && n2 <= 31 && n1 >= 1 && n1 <= 12) {
          newText = ("0" + n2).slice(-2) + ("0" + n1).slice(-2)
        }
      }
      if (newText !== oldText) {
        app.setFrame(tagv2, name, newText)
        message = "Changed '" + oldText + "' to '" + newText + "'"
        fixed = true
      }
      return { fixed, message }
    }

    /**
     * Replace with "hhmm" if valid hour and minute numbers found in string.
     */
    function fixHourMinute(model, row) {
      var message = ""
      var fixed = false
      var name = script.getRoleData(model, row, "internalName")
      var oldText = script.getRoleData(model, row, "value")
      var twoNumbersRe = /^[^\d]*(\d+)[^\d]+(\d+).*$/
      var match = twoNumbersRe.exec(oldText)
      var newText = oldText
      if (match && match.length === 3) {
        var n1 = +match[1]
        var n2 = +match[2]
        if (n1 >= 0 && n1 <= 23 && n2 >= 0 && n2 <= 59) {
          newText = ("0" + n1).slice(-2) + ("0" + n2).slice(-2)
        } else if (n2 >= 0 && n2 <= 23 && n1 >= 0 && n1 <= 59) {
          newText = ("0" + n2).slice(-2) + ("0" + n1).slice(-2)
        }
      }
      if (newText !== oldText) {
        app.setFrame(tagv2, name, newText)
        message = "Changed '" + oldText + "' to '" + newText + "'"
        fixed = true
      }
      return { fixed, message }
    }

    /**
     * Replace with "yyyy" if valid year number found in string.
     */
    function fixYear(model, row) {
      var message = ""
      var fixed = false
      var name = script.getRoleData(model, row, "internalName")
      var oldText = script.getRoleData(model, row, "value")
      var oneNumberRe = /^[^\d]*(\d+).*$/
      var match = oneNumberRe.exec(oldText)
      var newText = oldText
      if (match && match.length === 2) {
        var n = +match[1]
        if (n >= 1000 && n < 2100) {
          newText = "" + n
        }
      }
      if (newText !== oldText) {
        app.setFrame(tagv2, name, newText)
        message = "Changed '" + oldText + "' to '" + newText + "'"
        fixed = true
      }
      return { fixed, message }
    }

    /**
     * Replace strings like "(C) yyyy " with "yyyy ".
     */
    function fixYearSpace(model, row) {
      var message = ""
      var fixed = false
      var name = script.getRoleData(model, row, "internalName")
      var oldText = script.getRoleData(model, row, "value")
      var newText =
          oldText.replace(/^.*(?:\(C\)|\(c\)|\xa9)\s*(\d{4} .*)$/, "$1")
      if (newText !== oldText) {
        app.setFrame(tagv2, name, newText)
        message = "Changed '" + oldText + "' to '" + newText + "'"
        fixed = true
      }
      return { fixed, message }
    }

    /**
     * Replace strings like "yyyyMMdd", "yyyy/MM/dd" or "dd.MM.yyyy" with
     * "yyyy-MM-dd", try to fix swapped month/day number or keep at least year.
     */
    function fixIsoDate(model, row) {
      var message = ""
      var fixed = false
      var name = script.getRoleData(model, row, "internalName")
      var oldText = script.getRoleData(model, row, "value")
      var startsWithYearRe = /^[^\d]*(\d{4})[-/.]?(\d{1,2})[-/.]?(\d{1,2}).*$/
      var endsWithYearRe = /^[^\d]*(\d{1,2})[-/.]?(\d{1,2})[-/.]?(\d{4}).*$/
      var newText = oldText
      var year = 0
      var month = 0
      var day = 0
      var match
      if ((match = startsWithYearRe.exec(oldText)) && match.length === 4) {
        year = +match[1]
        month = +match[2]
        day = +match[3]
      } else if ((match = endsWithYearRe.exec(oldText)) && match.length === 4) {
        year = +match[3]
        month = +match[2]
        day = +match[1]
      }
      if (year >= 1000 && year < 2100) {
        if (day >= 1 && month >= 1) {
          if ((day > 31 || month > 12) && day <= 12 && month <= 31) {
            var tmp = day
            day = month
            month = tmp
          }
          newText = year + "-" +
              ("0" + month).slice(-2) + "-" + ("0" + day).slice(-2)
        } else {
          newText = "" + year
        }
      }
      if (newText !== oldText) {
        app.setFrame(tagv2, name, newText)
        message = "Changed '" + oldText + "' to '" + newText + "'"
        fixed = true
      }
      return { fixed, message }
    }

    /**
     * Try to generate a list with "involvement 1|involvee 1|..." from
     * "involvement 1:involvee 1;..." or
     * "involvee 1 (involvement 1);...".
     */
    function fixStringList(model, row) {
      var message = ""
      var fixed = false
      var name = script.getRoleData(model, row, "internalName")
      var oldText = script.getRoleData(model, row, "value")
      var invRe = oldText.indexOf(":") !== -1 ? /([^:]+):\s*([^;]+);?/
                                              : /([^(]+)\s*\([^)]+\)\s*[;,/]?/
      var newText = oldText
      var strs = []
      var involvRe, match
      if (oldText.indexOf(":") !== -1) {
        involvRe = /([^:]+):\s*([^;]+);?/g
        while ((match = involvRe.exec(oldText))) {
          strs.push(match[1])
          strs.push(match[2])
        }
      } else {
        involvRe = /([^(]+)\s*\([^)]+\)\s*[;,/]?/g
        while ((match = involvRe.exec(oldText))) {
          strs.push(match[2])
          strs.push(match[1])
        }
      }
      if (strs.length) {
        newText = strs.join("|")
      }
      if (newText !== oldText) {
        app.setFrame(tagv2, name, newText)
        message = "Changed '" + oldText + "' to '" + newText + "'"
        fixed = true
      }
      return { fixed, message }
    }

    /**
     * Remove space at begin or end of string.
     */
    function fixExcessSpace(model, row) {
      var message = ""
      var fixed = false
      var name = script.getRoleData(model, row, "internalName")
      var oldText = script.getRoleData(model, row, "value")
      var newText = oldText.replace(/^\s+/, "").replace(/\s+$/, "")
      if (newText !== oldText) {
        app.setFrame(tagv2, name, newText)
        message = "Changed '" + oldText + "' to '" + newText + "'"
        fixed = true
      }
      return { fixed, message }
    }

    var functionForWarning = {
      [FrameNotice.LanguageCode]: fixLanguageCode,
      [FrameNotice.NlForbidden]: fixNlForbidden,
      [FrameNotice.CrForbidden]: fixCrForbidden,
      [FrameNotice.OwnerEmpty]: fixOwnerEmpty,
      [FrameNotice.Numeric]: fixNumeric,
      [FrameNotice.NrTotal]: fixNrTotal,
      [FrameNotice.DayMonth]: fixDayMonth,
      [FrameNotice.HourMinute]: fixHourMinute,
      [FrameNotice.Year]: fixYear,
      [FrameNotice.YearSpace]: fixYearSpace,
      [FrameNotice.IsoDate]: fixIsoDate,
      [FrameNotice.StringList]: fixStringList,
      [FrameNotice.ExcessSpace]: fixExcessSpace
    }

    function doWork() {
      ///console.debug("Checking file " + app.selectionInfo.filePath)
      var tagFormat = app.selectionInfo.tag(Frame.Tag_2).tagFormat
      if (tagFormat && tagFormat.startsWith("ID3v2")) {
        var frameModel = app.tag(Frame.Tag_2).frameModel
        for (var i = 0; i < frameModel.rowCount(); ++i) {
          ///console.debug(frameModelRowToString(frameModel, i))
          var warning = script.getRoleData(frameModel, i, "noticeWarning")
          if (warning) {
            var notice = script.getRoleData(frameModel, i, "notice")
            var name = script.getRoleData(frameModel, i, "name")
            var func = functionForWarning[warning] || doNothing
            var result = func(frameModel, i)
            if (result) {
              console.log((result.fixed ? "MODIFIED " : "UNCHANGED") +
                          (result.message ? " " + result.message + ": " : " ") +
                          notice.replace(/\n/g, " ") + " in '" + name +
                          "' of '" + app.selectionInfo.filePath + "'")
            }
          }
        }
      }
      if (!nextFile()) {
        if (isStandalone()) {
          // Save the changes if the script is started stand-alone, not from Kid3.
          app.saveDirectory()
        }
        Qt.quit()
      } else {
        setTimeout(doWork, 1)
      }
    }

    firstFile()
    doWork()
  }
}
