#!/usr/bin/perl -w
#
# qmlpp - QML and JavaScript preprocessor
# Based on https://katastrophos.net/andre/blog/2013/09/20/qml-preprocessor-the-qnd-and-kiss-way/
# Converted to perl because it is already required to build Kid3.
#
# This library is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version
# 2.1 as published by the Free Software Foundation.

use strict;
use Getopt::Std;
use File::Find;

my %opts=();
if (!getopts("hiq:d:", \%opts) or $opts{h}) {
  usage();
  exit 1;
}

my $rewriteQtQuickVersion = $opts{q};
my $defines = $opts{d} // '@NODEFINESET';
my $processInline = $opts{i};
my $input = shift;

# Shortcuts for Kid3: 4, 5 and U for Qt4, Qt5 and Ubuntu.
if ($input eq "4") {
  $rewriteQtQuickVersion = "1.1";
  $defines = '@QtQuick1|@!Ubuntu';
  $processInline = 1;
  $input = ".";
} elsif ($input eq "5") {
  $rewriteQtQuickVersion = "2.2";
  $defines = '@QtQuick2|@!Ubuntu';
  $processInline = 1;
  $input = ".";
} elsif ($input eq "U") {
  $rewriteQtQuickVersion = "2.2";
  $defines = '@QtQuick2|@Ubuntu';
  $processInline = 1;
  $input = ".";
}

if (-f $input or $input eq "-") {
  preprocessFile($input);
} elsif (-d $input) {
  if (!$processInline) {
    print "Please specify -i when trying to preprocess a whole directory recursively.\n";
    usage();
    exit 1;
  }
  preprocessDirectory($input);
} else {
    print "Please specify a valid file or directory.\n";
    usage();
    exit 1;
}
exit 0;

sub usage {
  print <<EOF;
usage: $0 [options] <filename JS or QML or directoryname>

OPTIONS:
   -h                Show this message.
   -q <major.minor>  The version of QtQuick to rewrite to. Example: -q 2.1
   -d <defines>      The defines to set, separated by |. Example: -d "\@QtQuick1|\@Meego|\@Debug"
   -i                Modify file in-place instead of dumping to stdout.
EOF
}

sub preprocessFile {
  my $fn = shift;
  my @lines;
  open IF, $fn or die "Could not open $fn: $!\n";
  while (<IF>) {
    if (!/\/\/!noRewrite/) {
      s/import QtQuick\s*\d.\d/import QtQuick $rewriteQtQuickVersion/;
    }
    s/^(\s*)(\/\/){0,1}(.*)\/\/@(.*)/$1$3\/\/\@$4/;
    if (!/$defines/) {
      s/^(\s*)(.*)\/\/(@.*)/$1\/\/$2\/\/$3/;
    }
    if ($processInline) {
      push @lines, $_;
    } else {
      print;
    }
  }
  close IF;

  if ($processInline) {
    open OF, ">$fn" or die "Could not create $fn: $!\n";
    print OF @lines;
    close OF;
  }
}

sub wantedfunc {
  if (-f $_) {
    preprocessFile($_);
  }
}

sub preprocessfunc {
  my @filtered;
  foreach (@_) {
    if ((-d $_ and not /(?:\.svn|\.git)$/) or
        (-f $_ and /(?:\.qml|\.js)$/)) {
      push @filtered, $_;
    }
  }
  return @filtered;
}

sub preprocessDirectory {
  find({ wanted => \&wantedfunc, preprocess => \&preprocessfunc }, @_);
}
