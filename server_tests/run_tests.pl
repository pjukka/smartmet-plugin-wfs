#! /usr/bin/perl

use strict;
use File::Basename;
use File::Find;
use File::Slurp;
use LWP::UserAgent;
use URI::Escape;

die "\n### WFS server URL must be provided as the first parameter ###\n\n" unless $#ARGV >= 0;
my $URL = $ARGV[0];
my @test_files = FindTests();

my $ua = LWP::UserAgent->new;
$ua->agent("FMI_WFS_tester/0.1");

foreach my $file (@test_files) { RunTest($file); }

sub FindTests
{
    my @found_files;
    find(
        sub
        {
            push @found_files, $File::Find::name
                if /\.xml$/ || /\.kvp/;
        },
        ("in"));
    return @found_files;
}

sub RunTest
{
    my $fn = $_[0];
    if ($fn =~ m/\.xml$/)
    {
        run_xml_post_test($fn);
    }
    elsif ($fn =~ m/\.kvp$/)
    {
        run_kvp_get_test($fn);
        run_kvp_post_test($fn);
    }
    else
    {
        die "Do not know how to handle file $fn\n";
    }
}

sub run_xml_post_test()
{
    my $type = "text/xml";
    my $fn = $_[0];
    my $out = create_output_fn($fn, "post");
    my $exp = create_exp_fn($fn, "post");
    print "request=POST input=$fn content_type=$type\n";
    my $content = read_file($fn);
    my $req = HTTP::Request->new(POST => $URL);
    $req->content_type($type);
    $req->content($content);
    my $response = $ua->request($req);
    if ($response->is_success)
    {
        my $data = $response->content;
        process_result($fn, $data, $out, $exp);
    }
    else
    {
        unlink $out;
        print $fn, " : ", $response->status_line, "\n";
        print $response->content, "\n";
    }
}

sub run_kvp_get_test()
{
    my $fn = $_[0];
    my $out = create_output_fn($fn, "get");
    my $exp = create_exp_fn($fn, "get");
    print "request=GET input=$fn\n";
    my $input;
    open $input, "<$fn" or die "Failed to open $fn for reading: $!\n";
    my $url = $URL;
    my $sep = "?";
    while (<$input>)
    {
        if (m/([^=]+)=(.*)/)
        {
            my ($name, $value) = ($1, $2);
            $url = $url.$sep.$name."=".uri_escape($value);
            $sep = "&";
        }
    }
    close $input;
    my $req = HTTP::Request->new(GET => $url);
    my $response = $ua->request($req);
    if ($response->is_success)
    {
        my $data = $response->content;
        process_result($fn, $data, $out, $exp);
    }
    else
    {
        unlink $out;
        print $fn, " : ", $response->status_line, "\n";
        print $response->content, "\n";
    }
}

sub run_kvp_post_test()
{
    my $type = "application/x-www-form-urlencoded";
    my $fn = $_[0];
    my $out = create_output_fn($fn, "post");
    my $exp = create_exp_fn($fn, "post");
    print "request=POST input=$fn content_type=$type\n";
    my $input;
    open $input, "<$fn" or die "Failed to open $fn for reading: $!\n";
    my $content = "";
    my $sep = "";
    while (<$input>)
    {
        if (m/([^=]+)=(.*)/)
        {
            my ($name, $value) = ($1, $2);
            $content = $content.$sep.$name."=".uri_escape($value);
            $sep = "&";
        }
    }
    close $input;
    my $req = HTTP::Request->new(POST => $URL);
    $req->content_type("application/x-www-form-urlencoded");
    $req->content($content);
    my $response = $ua->request($req);
    if ($response->is_success)
    {
        my $data = $response->content;
        process_result($fn, $data, $out, $exp);
    }
    else
    {
        unlink $out;
        print $fn, " : ", $response->status_line, "\n";
        print $response->content, "\n";
    }
}

sub process_result()
{
    my $fn = $_[0];
    my $data = $_[1];
    my $out = $_[2];
    my $exp = $_[3];

    my $xslt_fn = get_xslt_conv_fn($fn, $out);
    if ($xslt_fn ne "")
    {
        write_file($out.".new", $data);
        system("xsltproc $xslt_fn $out.new >$out") == 0 or die "Failed to run xsltproc: $!\n";
        unlink($out.".new");
    }
    else
    {
        write_file($out, $data);
    }

    if (-f $exp)
    {
        system("diff -u $exp $out | head -50");
    }
}

sub create_output_fn
{
    my $fn = $_[0];
    my $suffix = $_[1];
    my $dir = dirname($fn);
    my $base = basename($fn);
    $base =~ s/\./_/;
    return $dir."/".$base."_".$suffix.".out";
}

sub create_exp_fn
{
    my $fn = $_[0];
    my $suffix = $_[1];
    my $dir = dirname($fn);
    my $base = basename($fn);
    $base =~ s/\./_/;
    return $dir."/".$base."_".$suffix.".exp";
}

sub get_xslt_conv_fn
{
    my $fn = $_[0];
    my $suffix = $_[1];
    my $dir = dirname($fn);
    my $base = basename($fn);
    $base =~ s/\./_/;
    my $xf1 = $dir."/".$base."_".$suffix.".xslt";
    my $xf2 = $dir."/conv.xslt";
    if (-f $xf1)
    {
        return $xf1;
    }
    elsif (-f $xf2)
    {
        return $xf2;
    }
    else
    {
        return "";
    }
}
