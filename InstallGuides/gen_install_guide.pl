#!/usr/bin/perl
# ---------------------------------------------------------------------------
# gen_install_guide.pl - genereaza Install Guide (Markdown) din tech sheet-ul
#                        .xlsx, dupa modelul ghidurilor generate automat
#                        (referinta: 403-FRD25EU-1.00-ORI ... _IG_EN.pdf)
#
# Sursa de adevar este techsheet-ul. Nimic nu se scrie de mana aici: tabelele
# de conexiuni ies din foile "Install 40x ORI" (403 = DB3) si "Install 933"
# (DS4), iar lista de functii de pe coperta din foaia "Features".
#
# Utilizare:
#   perl gen_install_guide.pl <techsheet.xlsx> <403|933> [> iesire.md]
#
# Nu are nevoie de Excel instalat: .xlsx e un ZIP cu XML, se citeste direct.
# Cere doar "unzip" in PATH (exista in Git for Windows).
# ---------------------------------------------------------------------------
use strict;
use warnings;
use File::Spec;
use FindBin;
use lib $FindBin::Bin;
use TechSheet;

my ($xlsx, $platform) = @ARGV;
die "utilizare: perl $0 <techsheet.xlsx> <403|933>\n"
    unless defined $xlsx && defined $platform;
die "nu gasesc fisierul: $xlsx\n" unless -f $xlsx;
die "platforma trebuie sa fie 403 sau 933\n" unless $platform =~ /^(403|933)$/;

my $ts = TechSheet->load($xlsx);

# --- asezarea coloanelor difera intre cele doua foi ------------------------
# 403: A pin | B nume | C polaritate | D culoare | E conn | F functie | G catre | H tag | I nota
# 933: A pin | B nume | C functie implicita | D polaritate | E culoare | F conn | G functie | H catre | I tag | J nota
my %LAYOUT = (
    '403' => { sheet => 'Install 40x ORI',
               pin=>'A', name=>'B', pol=>'C', color=>'D',
               conn=>'E', func=>'F', to=>'G', tag=>'H', note=>'I', dflt=>undef },
    '933' => { sheet => 'Install 933',
               pin=>'A', name=>'B', dflt=>'C', pol=>'D', color=>'E',
               conn=>'F', func=>'G', to=>'H', tag=>'I', note=>'J' },
);

# numele conectoarelor din ghid, in ordinea din ghid (nu cea din techsheet)
my %GUIDE_NAME = (
    '403' => [
        ['14 PINS CONNECTOR', 'Data 14-pin blue connector (H1)',   'H1'],
        ['12 PINS CONNECTOR', 'Output 12-pin red connector (H2)',  'H2'],
        ['10 PINS CONNECTOR', 'Input 10-pin black connector (H3)', 'H3'],
        ['2 PINS CONNECTOR',  'RF 2-pin white connector (H5)',     'H5'],
    ],
    '933' => [
        ['10 PINS CONNECTOR',  '10-pin main ignition harness', 'IGN'],
        ['12 PINS CONNECTOR',  '12-pin relay harness',         'RLY'],
        ['22 PINS CONNECTOR',  '22-pin input/output harness',  'IO'],
        ['8 PINS CONNECTOR',   '8-pin data harness',           'DATA'],
        ['FUSE PANEL',         'Fuse panel',                   'FUSE'],
    ],
);

# conectorul D2D nu apare in techsheet: e constant pe modul, nu depinde de firmware
my @H4 = (
    ['H4/1', 'Blue',  '(Data) TX'],
    ['H4/2', 'Black', '(-) Ground'],
    ['H4/3', 'Green', '(Data) RX'],
    ['H4/4', 'Red',   '(+) 12 Volt'],
);

my $L = $LAYOUT{$platform};
my $rows = $ts->sheet($L->{sheet});

# --- imparte foaia in blocuri de conector ---------------------------------
my @blocks;
my $cur;
for my $r (sort { $a <=> $b } keys %$rows) {
    my $a = $rows->{$r}{ $L->{pin} } // '';
    if ($a =~ /PINS CONNECTOR|FUSE PANEL/i) {
        push @blocks, $cur if $cur;
        $cur = { title => $a, rows => [] };
        next;
    }
    next unless $cur;
    next if uc($a) eq 'PIN' || uc($a) eq 'NAME';   # randul de antet
    push @{ $cur->{rows} }, $r;
}
push @blocks, $cur if $cur;

# ordinea din ghid difera de cea din techsheet (ghidul incepe cu H1 = 14-pin)
{
    my $i = 0;
    my %ord = map { $_->[0] => $i++ } @{ $GUIDE_NAME{$platform} };
    my $rank = sub {
        my $t = shift;
        for my $k (keys %ord) { return $ord{$k} if $t =~ /^\Q$k\E/i }
        return 999;
    };
    @blocks = sort { $rank->($a->{title}) <=> $rank->($b->{title}) } @blocks;
}

# --- compune coloana "Description" ca in ghidul generat -------------------
sub description {
    my $c = shift;
    my $conn = $c->{ $L->{conn} } // '';
    my $func = $c->{ $L->{func} } // '';
    my $pol  = $c->{ $L->{pol}  } // '';

    return 'No Connection' if $conn eq '' || $conn eq 'No Connection' || $func eq '';

    my $prefix = ($conn eq 'W2W Only') ? 'W2W Only - ' : '';
    # la releu, "polaritatea" e de fapt rolul contactului (Relay N.O./COM/N.C.)
    my $p = ($pol =~ /^Relay/) ? "$pol " : ($pol ne '' ? "$pol " : '');
    return $prefix . $p . $func;
}

sub md_escape { my $s = shift // ''; $s =~ s/\|/\\|/g; return $s; }

# --- lista de functii de pe coperta, din foaia "Features" -----------------
sub cover_functions {
    return () unless $ts->has_sheet('Features');
    my $f = $ts->sheet('Features');
    # antetul: coloanele B..E sint 40x D2D, 40x W2W, 800 D2D, 933 D2D
    my @cols = ($platform eq '403') ? ('B', 'C') : ('E');
    my @out;
    for my $r (sort { $a <=> $b } keys %$f) {
        next if $r <= 2;
        my $name = $f->{$r}{A} // '';
        next if $name eq '';
        my $marked = 0;
        for my $c (@cols) { $marked = 1 if ($f->{$r}{$c} // '') =~ /x/i }
        push @out, $name if $marked;
    }
    return @out;
}

# ===========================  IESIRE  ======================================
my $fw   = 'VW08EU';
my $ver  = '1.00';
my $veh  = 'Audi / Volkswagen / Seat / Skoda (PTS and Regular Key) EU';
my $plat = ($platform eq '403') ? 'DB3' : 'DS4';

print "<!-- GENERAT AUTOMAT de gen_install_guide.pl -->\n";
print "<!-- sursa: $xlsx, foaia '$L->{sheet}' -->\n";
print "<!-- NU edita acest fisier: modifica techsheet-ul si regenereaza -->\n\n";

print "# $platform.$fw v$ver\n\n";
print "**$veh**\n\n";
print "Platforma $plat.\n\n";

my @fn = cover_functions();
if (@fn) {
    print "## Functions compatible with this vehicle\n\n";
    print "> Lista vine din foaia `Features` a techsheet-ului, coloanele ",
          ($platform eq '403' ? '`40x D2D` / `40x W2W`' : '`933 D2D`'), ".\n";
    print "> **ATENTIE:** foaia `Features` nu a fost inca adusa la zi pentru ",
          "$fw - bifele de mai jos sint inca cele mostenite din sablon.\n\n";
    print "- $_\n" for @fn;
    print "\n";
}

print "## Vehicle connections\n\n";
print "Coloanele `Connects To` si `Wire Location` sint specifice vehiculului ",
      "si nu exista in techsheet; se completeaza din schema de cablare.\n\n";

for my $b (@blocks) {
    my ($label) = grep { $b->{title} =~ /^\Q$_->[0]\E/i } @{ $GUIDE_NAME{$platform} };
    my $title = $label ? $label->[1] : $b->{title};
    my $tagpfx = $label ? $label->[2] : '';

    print "### $title\n\n";

    # panoul de sigurante nu are fir propriu: A = numele sigurantei, fara culoare
    my $fuse = ($b->{title} =~ /FUSE PANEL/i);
    if ($fuse) {
        print "| Fuse | Description | Connects To | Note |\n|---|---|---|---|\n";
        for my $r (@{ $b->{rows} }) {
            my $c = $rows->{$r};
            next if ($c->{ $L->{pin} } // '') =~ /^NAME/i;
            my $conn = $c->{ $L->{conn} } // '';
            my $func = $c->{ $L->{func} } // '';
            printf "| %s | %s | %s | %s |\n", map { md_escape($_) } (
                $c->{ $L->{pin} } // '',
                ($conn eq '' || $conn eq 'No Connection' || $func eq '') ? 'No Connection' : $func,
                $conn, $c->{ $L->{tag} } // '');
        }
        print "\n";
        next;
    }

    print "| Conn./Pin | Module wire | Description | Connects To | Tag | Wire Location |\n";
    print "|---|---|---|---|---|---|\n";

    for my $r (@{ $b->{rows} }) {
        my $c    = $rows->{$r};
        my $pin  = $c->{ $L->{pin} } // '';
        my $col  = $c->{ $L->{color} } // '';
        my $desc = description($c);
        my $to   = $c->{ $L->{to} } // '';
        my $tag  = $c->{ $L->{tag} } // '';
        my $note = $c->{ $L->{note} } // '';

        my $ref  = ($tagpfx && $pin =~ /^\d+$/) ? "$tagpfx/$pin" : $pin;
        $to      = 'No Connection' if $to eq '';
        my $loc  = ($to eq 'No Connection') ? '' : '_TBD_';
        $desc   .= " (${note})" if $note ne '';

        printf "| %s | %s | %s | %s | %s | %s |\n",
            map { md_escape($_) } ($ref, $col, $desc, $to, $tag, $loc);
    }
    print "\n";
}

if ($platform eq '403') {
    print "### D2D 4-pin white connector (H4)\n\n";
    print "> Constant pe modul, nu depinde de firmware; nu apare in techsheet.\n\n";
    print "| Conn./Pin | Module wire | Description |\n|---|---|---|\n";
    printf("| %s | %s | %s |\n", @$_) for @H4;
    print "\n";
}

# --- schema modulului, desenata din aceleasi date ca tabelele ---------------
my $diagram = "module_diagram_$platform.svg";
if (-f $diagram) {
    print "## Module connection diagram\n\n";
    print "> Desen generat din acelasi tech sheet ca tabelele de mai sus, deci nu poate ",
          "ajunge sa le contrazica. Contine **numai firele folosite de $fw**.\n";
    print "> **Partea de vehicul lipseste intentionat:** conectorii si pinii din masina ",
          "nu sint inca stabiliti. Capetele libere ale firelor se completeaza cand se face ",
          "schema de cablare.\n\n";
    print "![Conexiunile $plat folosite de $fw]($diagram)\n\n";
}

print "## Feature and option list\n\n";
if ($ts->has_sheet('PF')) {
    my $pf = $ts->sheet('PF');
    my @lines = grep { $_ ne '' } map { $pf->{$_}{A} // '' }
                sort { $a <=> $b } keys %$pf;
    shift @lines if @lines && $lines[0] =~ /Product Features/i;
    print "$_\n\n" for @lines;
}

print "---\n\n";
printf "Generat din `%s`, foaia `%s`.\n",
    (File::Spec->splitpath($xlsx))[2], $L->{sheet};
