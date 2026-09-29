package TechSheet;
# ---------------------------------------------------------------------------
# Citire .xlsx fara Excel: fisierul e un ZIP cu XML inauntru.
# Folosit de gen_install_guide.pl si gen_diagram.pl, ca parsarea sa stea
# intr-un singur loc.
#
#   my $ts   = TechSheet->load($path);
#   my $rows = $ts->sheet('Install 40x ORI');   # { rand => { coloana => valoare } }
#   $ts->has_sheet('Features');
# ---------------------------------------------------------------------------
use strict;
use warnings;
use File::Temp qw(tempdir);
use File::Spec;

sub _quiet_system {
    my @cmd = @_;
    open(my $keep, '>&', \*STDERR) or return system(@cmd) == 0;
    open(STDERR, '>', File::Spec->devnull) or return system(@cmd) == 0;
    my $rc = system(@cmd);
    open(STDERR, '>&', $keep);
    close $keep;
    return $rc == 0;
}

sub _slurp {
    my $f = shift;
    open(my $h, '<:raw', $f) or return '';
    local $/;
    return <$h>;
}

sub unesc {
    my $s = shift // '';
    $s =~ s/&lt;/</g;  $s =~ s/&gt;/>/g;   $s =~ s/&quot;/"/g;
    $s =~ s/&apos;/'/g; $s =~ s/&#10;/ /g; $s =~ s/&amp;/&/g;
    $s =~ s/\s+/ /g;   $s =~ s/^ //;      $s =~ s/ $//;
    return $s;
}

sub load {
    my ($class, $xlsx) = @_;
    die "nu gasesc fisierul: $xlsx\n" unless -f $xlsx;

    my $tmp = tempdir(CLEANUP => 1);
    # "unzip" exista in Git Bash, dar nu si in PATH-ul cmd/PowerShell;
    # "tar" din Windows 10 (bsdtar) citeste si el zip, deci serveste ca rezerva.
    # Nicio unealta nu e sigura in ambele shell-uri: "unzip" exista doar in
    # Git Bash, iar "tar" din Windows (bsdtar) citeste zip, dar cel din Git Bash
    # nu. Se incearca pe rind, cu stderr inchis ca sa nu urle cea care pica.
    my $ok = _quiet_system('tar',   '-x', '-f', $xlsx, '-C', $tmp)
          || _quiet_system('unzip', '-o', '-q', $xlsx, '-d', $tmp);
    die "nu pot despacheta $xlsx (nici unzip, nici tar)\n" unless $ok;

    my $self = bless { tmp => $tmp, path => $xlsx, ss => [], sheet => {} }, $class;

    # sirurile comune
    my $s = _slurp("$tmp/xl/sharedStrings.xml");
    while ($s =~ /<si>(.*?)<\/si>/gs) {
        my ($t, $o) = ($1, '');
        $o .= $1 while $t =~ /<t[^>]*>(.*?)<\/t>/gs;
        push @{ $self->{ss} }, unesc($o);
    }

    # nume foaie -> fisier
    my %rel;
    my $r = _slurp("$tmp/xl/_rels/workbook.xml.rels");
    $rel{$1} = $2 while $r =~ /Id="([^"]*)"[^>]*Target="([^"]*)"/g;
    my $w = _slurp("$tmp/xl/workbook.xml");
    while ($w =~ /<sheet name="([^"]*)"[^>]*r:id="([^"]*)"/g) {
        my ($n, $id) = (unesc($1), $2);
        next unless $rel{$id};
        (my $t = $rel{$id}) =~ s{^/?}{};
        $self->{sheet}{$n} = "$tmp/xl/$t";
    }
    return $self;
}

sub path      { $_[0]{path} }
sub has_sheet { exists $_[0]{sheet}{ $_[1] } }

sub sheet {
    my ($self, $name) = @_;
    die "foaia '$name' lipseste din $self->{path}\n" unless $self->has_sheet($name);
    my $w = _slurp( $self->{sheet}{$name} );
    my %rows;
    while ($w =~ /<c r="([A-Z]+)(\d+)"([^>]*?)(?:\/>|>(.*?)<\/c>)/gs) {
        my ($col, $row, $attr, $body) = ($1, $2, $3, defined $4 ? $4 : '');
        my $v = '';
        $v = $1 if $body =~ /<v>(.*?)<\/v>/s;
        if ($attr =~ /t="s"/ && $v ne '') {
            $v = defined $self->{ss}[$v] ? $self->{ss}[$v] : '';
        }
        elsif ($body =~ /<is>.*?<t[^>]*>(.*?)<\/t>/s) { $v = unesc($1) }
        else { $v = unesc($v) }
        $rows{$row}{$col} = $v if $v ne '';
    }
    return \%rows;
}

1;
