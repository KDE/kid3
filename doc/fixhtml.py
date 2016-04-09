#!/usr/bin/env python

import fileinput
import sys
import os


def fix_html():
    for line in fileinput.input(mode='rb'):
        if line.lower() == b'><title\n':
            line = b'><meta http-equiv="content-type" content="text/html; charset=UTF-8"\n><title\n'
        line = line.replace(b'</TITLE', b'</title').replace(b'</title',
b"""</title>
<style type="text/css">
body { font-family: Arial, Helvetica, sans-serif; color: #000000; background: #ffffff; }
h1, h2, h3, h4 { text-align: left; font-weight: bold; color: #f7800a; background: transparent; }
a:link { color: #0057ae; }
pre { display: block; color: #000000; background: #f9f9f9; border: #2f6fab dashed; border-width: 1px; overflow: auto; line-height: 1.1em; }
dt { font-weight: bold; color: #0057ae; }
p { text-align: justify; }
li { text-align: left; }
.guibutton, .guilabel, .guimenu, .guimenuitem { font-family: Arial, Helvetica, sans-serif; color: #000000; background: #dcdcdc; }
.application { font-weight: bold; }
.command { font-family: "Courier New", Courier, monospace; }
.filename { font-style: italic; }
</style""")
        os.write(sys.stdout.fileno(), line)

if __name__ == '__main__':
    fix_html()
