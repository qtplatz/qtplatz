#!/usr/bin/perl -w

# Copyright (C) 2016 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

=head1 NAME

purify2tasks.pl - Convert Rational Purify logs into Qt Creator task files.

=head1 SYNOPSIS

    purify2tasks.pl < logfile > taskfile

=cut

use strict;

my $lastMessage = '';

while (my $line = <STDIN> ) {
    chomp($line);
    # --- extract file name based matching:
    # Classname::function [c:\path\file.cpp:389]
    if ($line =~ / \[(.*:\d+)\]$/) {
        my $capture = $1;
        my $lastColon = rindex($capture, ':');
        my $fileName = substr($capture, 0, $lastColon);
        my $lineNumber = substr($capture, $lastColon + 1);
        $fileName =~ s|\\|/|g;
        print $fileName, "\t", $lineNumber, "\tpurify\t", $lastMessage,"\n";
    # match a warning/error report
    } elsif ($line =~ /^\[[W|E|I]\] /) {
        $lastMessage = substr($line, 4);
        $lastMessage =~ s/\\/\\\\/g;
    }
}
