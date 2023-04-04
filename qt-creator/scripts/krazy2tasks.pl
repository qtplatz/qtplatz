#!/usr/bin/perl -w

# Copyright (C) 2016 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

use strict;

my $file = shift;

if (!defined $file)
{
   my $usage=<<EOF;
Usage: krazy2tasks.pl outputfile

Runs KDE's Krazy2 code scanning tool on .cpp/.h files it finds below the
the working directory, filters and converts the output into a .tasks file
that can be loaded into Qt Creator's build issues pane via 'Open'.
EOF
   print $usage;
   exit(0);
}

open(PIPE, 'find . -name "*.cpp" -o -name "*.h" | grep -v /tests/ | grep -v /3rdparty/ | xargs krazy2 --check-sets qt4 --exclude captruefalse --export textedit |') or
    die 'Could not start krazy2, please make sure it is in your PATH.';
open(FILE, ">$file") or die ('Failed to open ' . $file . ' for writing.');

while (<PIPE>) {
    my $line = $_;
    chomp $line;
    next unless $line =~ /^(.*):(\d+):(.*)$/;

    my $file = $1;
    my $lineno = $2;
    my $description = $3;
    next if $file =~ /\/3rdparty\//;
    next if $file =~ /\/tests\//;

    print FILE "$file\t$lineno\tWARN\tKrazy: $description\n";
}
