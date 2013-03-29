#!/usr/bin/perl -W
package po2ts;

use strict;
use Cwd;
use File::Find;

use vars qw(@ISA @EXPORT);
require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(generateTs);

# Read all translations from a .po file, fill them into an associative
# array.
sub getPoTranslations($)
{
  my $fn = shift;
  my $msgid;
  my $msgstr;
  my $in_msgid = 0;
  my $in_msgstr = 0;
  my %trans;
  open IF, $fn or die "Could not open $fn: $!\n";
  while (<IF>) {
    s/\r\n/\n/;
    if (/^msgid "(.*)"$/) {
      if ($msgid) {
        $trans{$msgid} = $msgstr;
      }
      $msgid = $1;
      $msgstr = "";
      $in_msgid = 1;
      $in_msgstr = 0;
    }
    if (/^msgstr "(.*)"$/) {
      $msgstr = $1;
      $in_msgid = 0;
      $in_msgstr = 1;
    }
    if (/^"(.+)"$/) {
      if ($in_msgid) {
        $msgid .= $1;
      } elsif ($in_msgstr) {
        $msgstr .= $1;
      }
    }
  }
  close IF;
  if ($msgid) {
    $trans{$msgid} = $msgstr;
  }
  return %trans;
}

# Set the translations in a .ts file replacing & by &amp;, < by &lt;,
# > by &gt; and ' by &apos;.
sub setTsTranslations($$%)
{
  my $infn = shift;
  my $outfn = shift;
  my %trans = @_;
  my $source;
  my $translation;
  my $in_source = 0;
  open IF, $infn or die "Could not open $infn: $!\n";
  open OF, ">$outfn" or die "Could not create $outfn: $!\n";
  while (<IF>) {
    s/\r\n/\n/;
    if (/<source>(.*)<\/source>/) {
      $source = $1;
      $in_source = 0;
    } elsif (/<source>(.*)$/) {
      $source = $1;
      $in_source = 1;
    } elsif ($in_source) {
      if (/^(.*)<\/source>/) {
        $source .= "\n$1";
        $in_source = 0;
      } else {
        my $line = $_;
        chomp $line;
        $source .= "\n$line";
      }
    } elsif (/<translation/) {
      $source =~ s/&amp;/&/g;
      $source =~ s/&lt;/</g;
      $source =~ s/&gt;/>/g;
      $source =~ s/&apos;/'/g;
      $source =~ s/\n/\\n/g;
      if (exists $trans{$source}) {
        $translation = $trans{$source};
        $translation =~ s/&/&amp;/g;
        $translation =~ s/</&lt;/g;
        $translation =~ s/>/&gt;/g;
        $translation =~ s/'/&apos;/g;
        $translation =~ s/\\"/&quot;/g;
        $translation =~ s/\\n/\n/g;
        s/ type="unfinished"//;
        s/<\/translation>/$translation<\/translation>/;
      } else {
        print "Could not find translation for \"$source\"\n";
      }
    }
    print OF $_;
  }
  close OF;
  close IF;
}

my @sources;

# Push .cpp file names into @sources.
sub wanted
{
  /\.cpp$/ && push @sources, $File::Find::name;
}

# Generate .ts files from .po files.
# parameters: path to lupdate command, directory with po-files,
# directory with source files
sub generateTs
{
  my ($lupdate_cmd, $podir, $srcdir) = @_;
  my @pofiles = glob "$podir/*.po";
  my @languages = map { /^.*\/([\w@]+)\.po$/ } @pofiles;
  my $curdir = cwd();
  find(\&wanted, $srcdir);
  chdir $srcdir or die "Could not change to $srcdir: $!\n";
  system "$lupdate_cmd -recursive . -ts " . join ' ', map { "$curdir/tmp_". $_ . ".ts" } @languages;
  chdir $curdir;
  foreach my $lang (@languages) {
    setTsTranslations("tmp_$lang.ts", "kid3_$lang.ts",
                      getPoTranslations("$podir/$lang.po"));
  }
  unlink map { "tmp_". $_ . ".ts" } @languages;
}

if (!caller()) {
  generateTs(@ARGV);
}
1;
