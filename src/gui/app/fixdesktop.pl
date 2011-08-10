#!/usr/bin/perl -n
s/^Name=Kid3$/Name=Kid3-qt/;
s/^Exec=kid3/Exec=kid3-qt/;
s/^Icon=kid3$/Icon=kid3-qt/;
s/^Categories=Qt;KDE/Categories=Qt/;
s/^StartupWMClass=kid3$/StartupWMClass=kid3-qt/;
print unless /^X-DocPath/;
