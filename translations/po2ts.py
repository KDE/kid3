#!/usr/bin/env python

import re
import glob
import os
import sys


def get_po_translations(fn):
    """
    Read all translations from a .po file, fill them into an associative
    array.
    """
    msgctxt = b''
    msgid = b''
    msgstr = b''
    msgstrs = []
    in_msgctxt = False
    in_msgid = False
    in_msgstr = False
    in_msgstrs = False
    trans = {}

    def add_trans():
        if msgid:
            context = msgctxt
            pipepos = context.find(b'|')
            if pipepos != -1:
                context = context[:pipepos]
            if in_msgstrs:
                if msgstr:
                    msgstrs.append(msgstr)
                trans.setdefault(msgid, {})[context] = msgstrs
            else:
                trans.setdefault(msgid, {})[context] = msgstr

    msgctxtre = re.compile(br'^msgctxt "(.*)"$')
    msgidre = re.compile(br'^msgid "(.*)"$')
    msgstrre = re.compile(br'^msgstr "(.*)"$')
    msgstrsre = re.compile(br'^msgstr\[\d\] "(.*)"$')
    strcontre = re.compile(br'^"(.+)"$')
    with open(fn, 'rb') as fh:
        for line in fh:
            line = line.replace(b'\r\n', b'\n')
            m = msgctxtre.match(line)
            if m:
                add_trans()
                msgid = b'' # do not add it again in add_trans() call below
                msgctxt = m.group(1)
                in_msgctxt = True
                in_msgid = False
                in_msgstr = False
                in_msgstrs = False
            m = msgidre.match(line)
            if m:
                add_trans()
                msgid = m.group(1)
                msgstr = b''
                msgstrs = []
                in_msgctxt = False
                in_msgid = True
                in_msgstr = False
                in_msgstrs = False
            m = msgstrre.match(line)
            if m:
                msgstr = m.group(1)
                in_msgctxt = False
                in_msgid = False
                in_msgstr = True
                in_msgstrs = False
            m = msgstrsre.match(line)
            if m:
                if msgstr:
                    msgstrs.append(msgstr)
                msgstr = m.group(1)
                in_msgctxt = False
                in_msgid = False
                in_msgstr = True
                in_msgstrs = True
            m = strcontre.match(line)
            if m:
                if in_msgctxt:
                    msgctxt += m.group(1)
                elif in_msgid:
                    msgid += m.group(1)
                elif in_msgstr:
                    msgstr += m.group(1)
    add_trans()
    return trans


def set_ts_translations(infn, outfn, trans):
    """
    Set the translations in a .ts file replacing & by &amp;, < by &lt;,
    > by &gt; and ' by &apos;.
    """
    def decode_entities(s):
        return s \
            .replace(b'&amp;', b'&') \
            .replace(b'&lt;', b'<') \
            .replace(b'&gt;', b'>') \
            .replace(b'&apos;', b"'") \
            .replace(b'&quot;', br'\"') \
            .replace(b'\n', br'\n')

    def encode_entities(s):
        return s \
            .replace(b'&', b'&amp;') \
            .replace(b'<', b'&lt;') \
            .replace(b'>', b'&gt;') \
            .replace(b"'", b'&apos;') \
            .replace(br'\"', b'&quot;') \
            .replace(br'\n', b'\n')

    name = b''
    source = b''
    in_source = False
    numerusforms = []
    namere = re.compile(br'<name>(.*)</name>')
    sourcere = re.compile(br'<source>(.*)</source>')
    sourcebeginre = re.compile(br'<source>(.*)$')
    sourceendre = re.compile(br'^(.*)</source>')
    with open(infn, 'rb') as infh:
        with open(outfn, 'wb') as outfh:
            for line in infh:
                line = line.replace(b'\r\n', b'\n')
                m = namere.search(line)
                if m:
                    name = m.group(1)
                m = sourcere.search(line)
                if m:
                    source = m.group(1)
                    in_source = False
                else:
                    m = sourcebeginre.search(line)
                    if m:
                        source = m.group(1)
                        in_source = True
                    elif in_source:
                        source += b'\n'
                        m = sourceendre.match(line)
                        if m:
                            source += m.group(1)
                            in_source = False
                        else:
                            source += line.strip()
                    elif b'<translation' in line:
                        source = decode_entities(source)
                        if source in trans:
                            translations_for_context = trans[source]
                            translation = translations_for_context.get(name, b'')
                            if not translation:
                                for translation in translations_for_context.values():
                                    if translation:
                                        break
                            line = line.replace(b' type="unfinished"', b'')
                            if type(translation) == list:
                                numerusforms = translation
                            else:
                                translation = encode_entities(translation)
                                line = line.replace(b'</translation>',
                                             translation + b'</translation>')
                        else:
                            print('%s: Could not find translation for "%s"' %
                                  (outfn, source.decode()))
                    elif b'<numerusform' in line:
                        if numerusforms:
                            translation = encode_entities(numerusforms.pop(0))
                            line = line.replace(b'</numerusform>',
                                                translation + b'</numerusform>')
                        else:
                            print('%s: Could not find translation for "%s"' %
                                  (outfn, source.decode()))

                outfh.write(line)


def generate_ts(lupdate_cmd, podir, srcdir):
    """
    Generate .ts files from .po files.
    parameters: path to lupdate command, directory with po-files,
    directory with source files
    """
    pofiles = glob.glob(os.path.join(podir, '*/*.po'))
    pofnre = re.compile(r'^.*[\\/]([\w@]+)[\\/][^\\/]+\.po$')
    languages = [pofnre.sub(r'\1', f) for f in pofiles]
    curdir = os.getcwd()
    sources = []
    for root, dirs, files in os.walk(srcdir):
        for fn in files:
            if fn.endswith('.cpp'):
                sources.append(os.path.join(root, fn))
    os.chdir(srcdir)
    os.system(lupdate_cmd + ' -recursive -locations none . -ts ' +
              ' '.join([os.path.join(curdir, 'tmp_' + l + '.ts')
                        for l in languages]))
    if 'en' in languages:
        os.system(lupdate_cmd + ' -pluralonly -recursive -locations none . -ts ' +
                  os.path.join(curdir, 'tmp_en.ts'))
    os.chdir(curdir)
    for pofile, lang in zip(pofiles, languages):
        tmptsfn = 'tmp_' + lang + '.ts'
        set_ts_translations(tmptsfn, 'kid3_' + lang + '.ts',
                            get_po_translations(pofile))
        os.remove(tmptsfn)


if __name__ == '__main__':
    generate_ts(*sys.argv[1:4])
