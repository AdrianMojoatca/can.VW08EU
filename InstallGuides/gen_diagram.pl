#!/usr/bin/perl
# ---------------------------------------------------------------------------
# gen_diagram.pl - schema de instalare (SVG), dupa modelul ghidurilor DB3.
#
# Asezare, ca in schema originala:
#   sus       - conectoarele din masina (desenele reale, IGs/Connectors/)
#   jos       - modulul (stinga-centru) si remote starter-ul (dreapta)
#   la mijloc - firele, fiecare plecind din pinul lui, din conectorul lui
#
# Sensul conexiunilor:
#   - firele catre REMOTE STARTER sint trase pina la el
#   - firele catre VEHICUL urca la borna din conectorul respectiv
#   - restul raman cu capat liber (cerc gol): nu se stie inca unde merg
#
# Datele modulului vin din tech sheet; partea de vehicul din
# vehicle_side_<platforma>.txt (vezi comentariile de acolo).
#
# Utilizare:  perl gen_diagram.pl <techsheet.xlsx> <403|933> [> iesire.svg]
# ---------------------------------------------------------------------------
use strict;
use warnings;
use FindBin;
use lib $FindBin::Bin;
use TechSheet;
use MIME::Base64 qw(encode_base64);

my ($xlsx, $platform) = @ARGV;
die "utilizare: perl $0 <techsheet.xlsx> <403|933>\n"
    unless defined $xlsx && defined $platform;
die "platforma trebuie sa fie 403 sau 933\n" unless $platform =~ /^(403|933)$/;

my %LAYOUT = (
    '403' => { sheet=>'Install 40x ORI',
               pin=>'A', pol=>'C', color=>'D', conn=>'E', func=>'F', to=>'G' },
    '933' => { sheet=>'Install 933',
               pin=>'A', pol=>'D', color=>'E', conn=>'F', func=>'G', to=>'H' },
);

# Geometria conectoarelor pe imaginea modulului, ca fractii din inaltime.
# Masurata pe IGs/part_db3_module.png (752 x 835). side: L = stinga, R = dreapta.
my %GEOM = (
    '403' => {
        'H1' => { match=>'14 PINS CONNECTOR', pins=>14, side=>'L', ord=>0, y0=>0.030, y1=>0.453, fill=>'#2f5fa8' },
        'H2' => { match=>'12 PINS CONNECTOR', pins=>12, side=>'L', ord=>1, y0=>0.467, y1=>0.994, fill=>'#b8352b' },
        'H4' => { match=>undef,               pins=>4,  side=>'R', ord=>0, y0=>0.057, y1=>0.309, fill=>'#dcdcdc' },
        'H5' => { match=>'2 PINS CONNECTOR',  pins=>2,  side=>'R', ord=>1, y0=>0.317, y1=>0.449, fill=>'#dcdcdc' },
        'H3' => { match=>'10 PINS CONNECTOR', pins=>10, side=>'R', ord=>2, y0=>0.461, y1=>0.946, fill=>'#2b2b2b' },
    },
    # Pe DS4 nu am grafica modulului, deci nici asezarea fizica reala a
    # conectoarelor. Barele se deseneaza schematic, pe stinga.
    '933' => {
        'IGN'  => { match=>'10 PINS CONNECTOR', pins=>10, side=>'L', ord=>0, fill=>'#2b2b2b' },
        'RLY'  => { match=>'12 PINS CONNECTOR', pins=>12, side=>'L', ord=>1, fill=>'#b8352b' },
        'IO'   => { match=>'22 PINS CONNECTOR', pins=>22, side=>'L', ord=>2, fill=>'#2f5fa8' },
        'DATA' => { match=>'8 PINS CONNECTOR',  pins=>8,  side=>'L', ord=>3, fill=>'#1f7a4d' },
    },
);
my $G = $GEOM{$platform};
my $L = $LAYOUT{$platform};

sub esc { my $s = shift // ''; $s =~ s/&/&amp;/g; $s =~ s/</&lt;/g; $s =~ s/>/&gt;/g; return $s }
sub twidth { my ($s, $fs) = @_; return length($s) * $fs * 0.52 }
sub mirror {
    my $f = shift; $f =~ s/\s+$//;
    return $f =~ s/\bOutput\b/Input/r  if $f =~ /\bOutput\b/;
    return $f =~ s/\bInput\b/Output/r  if $f =~ /\bInput\b/;
    return $f;
}
sub b64 {
    my $f = shift;
    open(my $h, '<:raw', $f) or die "nu pot citi $f\n";
    local $/; my $bin = <$h>; close $h;
    return 'data:image/png;base64,' . encode_base64($bin, '');
}
sub png_size {                       # latime/inaltime din antetul IHDR
    my $f = shift;
    open(my $h, '<:raw', $f) or return (0, 0);
    read($h, my $buf, 24); close $h;
    return (unpack('N', substr($buf, 16, 4)), unpack('N', substr($buf, 20, 4)));
}

# --- firele folosite, cu conectorul si pinul lor --------------------------
my $ts   = TechSheet->load($xlsx);
my $rows = $ts->sheet( $L->{sheet} );
my (@wires, $tag);
for my $r (sort { $a <=> $b } keys %$rows) {
    my $a = $rows->{$r}{ $L->{pin} } // '';
    if ($a =~ /PINS CONNECTOR|FUSE PANEL/i) {
        $tag = undef;
        for my $k (keys %$G) {
            next unless defined $G->{$k}{match};
            $tag = $k if $a =~ /^\Q$G->{$k}{match}\E/i;
        }
        next;
    }
    next unless $tag;
    next if uc($a) eq 'PIN' || $a !~ /^\d+$/;

    my $c    = $rows->{$r};
    my $conn = $c->{ $L->{conn} } // '';
    my $func = $c->{ $L->{func} } // '';
    next if $conn eq '' || $conn eq 'No Connection' || $func eq '';
    my $pol = $c->{ $L->{pol} } // '';
    push @wires, {
        tag => $tag, pin => $a + 0,
        color => $c->{ $L->{color} } // '',
        desc  => ($pol ne '' ? "$pol " : '') . $func,
        func  => $func,
        rs    => (($c->{ $L->{to} } // '') =~ /Remote Starter/i) ? 1 : 0,
        # W2W Only = necesar doar cind nu se lucreaza in D2D; in schema
        # originala aceste fire sint desenate cu linie intrerupta
        w2w   => ($conn eq 'W2W Only') ? 1 : 0,
    };
}
my $has_rs = grep { $_->{rs} } @wires;

# --- partea de vehicul -----------------------------------------------------
my (@blocks, %blk_img, %blk_note, @veh, %veh_of, %rs_term);
{
    my $f = "$FindBin::Bin/vehicle_side_$platform.txt";
    if (open(my $h, '<', $f)) {
        while (my $l = <$h>) {
            chomp $l; $l =~ s/#.*//; next if $l !~ /\S/;
            if ($l =~ /^\s*\@\s*(.*)$/) {
                my ($name, $img, $note) = map { s/^\s+|\s+$//gr } split /\|/, $1, 3;
                push @blocks, $name;
                $blk_img{$name} = ($img && -f "$FindBin::Bin/$img") ? "$FindBin::Bin/$img" : undef;
                $blk_note{$name} = $note if defined $note && $note ne '';
                next;
            }
            if ($l =~ /^\s*\$\s*(.*)$/) {      # numele bornei de pe remote starter
                my ($ref, $name) = map { s/^\s+|\s+$//gr } split /\|/, $1, 2;
                $rs_term{$ref} = $name if defined $name;
                next;
            }
            my ($blk, $pin, $desc, $ref, $anc) = map { s/^\s+|\s+$//gr } split /\|/, $l, 5;
            next unless defined $desc;
            $ref = '' unless defined $ref;
            my $t = { blk => $blk, pin => $pin // '', desc => $desc, ref => $ref };
            # ancora: unde intra firul pe desenul conectorului
            if (defined $anc && $anc =~ /\d/) {
                $t->{anchor} = [ map { [ split /\s*,\s*/ ] } split /\s*;\s*/, $anc ];
            }
            # "RS:..." = borna vine direct de la remote starter, nu prin modul
            if ($ref =~ /^RS:\s*(.*)$/) { $t->{from_rs} = $1; $t->{ref} = ''; $ref = '' }
            push @veh, $t;
            $veh_of{$ref} = $t if $ref ne '';
        }
        close $h;
    }
    @blocks = do { my %s; grep { !$s{$_}++ } map { $_->{blk} } @veh } unless @blocks;
}

my $IMG_MOD = "$FindBin::Bin/IGs/part_db3_module.png";
my $IMG_RS  = "$FindBin::Bin/IGs/part_remote_starter.png";
my $artwork = ($platform eq '403' && -f $IMG_MOD && -f $IMG_RS);

# ===========================  GEOMETRIE  ===================================
my $FS     = 12.5;
my $FSB    = 11;       # text in blocurile de vehicul
my $STEP   = 26;
my $BAND_Y = 118;      # banda de sus, cu conectoarele din masina
my $IMG_H  = 230;      # inaltimea la care se scaleaza desenele conectoarelor
my $MOD_W  = 430;
my $MOD_H  = $artwork ? int($MOD_W * 835 / 752) : 520;

my ($band_bot, $bx) = ($BAND_Y, 60);
my %blk;
# pasul 1: dimensiunile desenelor si latimea blocurilor
for my $b (@blocks) {
    my @t = grep { $_->{blk} eq $b } @veh;
    next unless @t;
    my ($iw, $ih) = (120, 96);
    if ($blk_img{$b}) {
        my ($w0, $h0) = png_size($blk_img{$b});
        if ($h0) {
            $ih = $IMG_H; $iw = int($w0 * $IMG_H / $h0);
            if ($iw > 250) { $iw = 250; $ih = int($h0 * 250 / $w0) }
        }
    }
    my $lw = 0;
    for (@t) {
        my $s = ($_->{pin} ne '' ? "pin $_->{pin}: " : '') . $_->{desc};
        my $w = twidth($s, $FSB) + 24;
        $lw = $w if $w > $lw;
    }
    $blk{$b} = { x => $bx, w => ($iw > $lw ? $iw : $lw), iw => $iw, ih => $ih,
                 iy => $BAND_Y + 20,
                 used => scalar(grep { $_->{ref} ne '' || $_->{from_rs} } @t) };
    $bx += $blk{$b}{w} + 46;
}
my $band_right = $bx;

# pasul 2: legendele tuturor blocurilor pornesc de la aceeasi linie, altfel
# blocurile cu desen scund isi ridica lista si banda arata neingrijit
my $img_bot = $BAND_Y;
for (values %blk) { my $y = $_->{iy} + $_->{ih}; $img_bot = $y if $y > $img_bot }
for my $b (@blocks) {
    next unless $blk{$b};
    my @t = grep { $_->{blk} eq $b } @veh;
    my $ty = $img_bot + 14;
    $_->{by} = ($ty += 16) for @t;
    $blk{$b}{bot} = $ty + 12;
    $band_bot = $blk{$b}{bot} if $blk{$b}{bot} > $band_bot;
}

my $MOD_X = @veh ? ($band_right > 1290 ? $band_right - 400 : 1290) : 680;
my $MOD_Y = @veh ? $band_bot + 118 : 150;
my $LBL_L = $MOD_X - 60;
my $LBL_R = $MOD_X + $MOD_W + 62;
my $VEH_X = @veh ? $MOD_X - 660 : 140;
my $RS_X  = $LBL_R + 430;
my $RS_W  = 250;
my $RS_H  = int($RS_W * 715 / 376);
my $W     = $has_rs ? $RS_X + $RS_W + 40 : $MOD_X + $MOD_W + 70;

# barele: pentru 933 se calculeaza proportional cu numarul de pini
if (!$artwork) {
    my @order = sort { $G->{$a}{ord} <=> $G->{$b}{ord} } keys %$G;
    @order = grep { my $k = $_; grep { $_->{tag} eq $k } @wires } @order;
    my $tot = 0; $tot += $G->{$_}{pins} for @order;
    my $avail = $MOD_H - 20 - 8 * (@order - 1);
    my $cy = 10;
    for my $k (@order) {
        my $h = $avail * $G->{$k}{pins} / $tot;
        $G->{$k}{y0} = $cy / $MOD_H;
        $G->{$k}{y1} = ($cy + $h) / $MOD_H;
        $cy += $h + 8;
    }
}

sub bar {
    my $k = shift; my $g = $G->{$k};
    return ( ($g->{side} eq 'L') ? $MOD_X : $MOD_X + $MOD_W,
             $MOD_Y + $g->{y0} * $MOD_H, $MOD_Y + $g->{y1} * $MOD_H );
}
sub pin_y {
    my ($k, $n) = @_;
    my (undef, $y0, $y1) = bar($k);
    return $y0 + ($n - 0.5) * ($y1 - $y0) / $G->{$k}{pins};
}

my $by_conn = sub {
    $G->{ $_[0]{tag} }{ord} <=> $G->{ $_[1]{tag} }{ord} || $_[0]{pin} <=> $_[1]{pin}
};
my @left  = sort { $by_conn->($a,$b) } grep { $G->{ $_->{tag} }{side} eq 'L' } @wires;
my @right = sort { $by_conn->($a,$b) } grep { $G->{ $_->{tag} }{side} eq 'R' } @wires;

my $GRP_GAP = 10;
sub assign_rows {
    my @ws = @_; return unless @ws;
    my $span = 0;
    $span += $STEP + ($ws[$_]{tag} ne $ws[$_-1]{tag} ? $GRP_GAP : 0) for 1 .. $#ws;
    my $y = $MOD_Y + ($MOD_H - $span) / 2;
    for my $i (0 .. $#ws) {
        $y += $STEP + ($ws[$i]{tag} ne $ws[$i-1]{tag} ? $GRP_GAP : 0) if $i;
        $ws[$i]{row} = $y;
    }
}
assign_rows(@left);
assign_rows(@right);

my $maxrow = 0;
for (@left, @right) { $maxrow = $_->{row} if $_->{row} > $maxrow }
my $BUS_Y = $maxrow + 46;
my $H = $BUS_Y + 5 * 10 + 88;

# ===========================  DESEN  =======================================
print qq{<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" }
    . qq{viewBox="0 0 $W $H" width="100%" font-family="Segoe UI, Arial, sans-serif">\n};
print qq{<rect width="$W" height="$H" fill="#ffffff"/>\n};

my $plat = ($platform eq '403') ? 'DB3' : 'DS4';
print qq{<text x="24" y="40" font-size="20" font-weight="600" fill="#0b5394">$plat - schema de instalare VW08EU</text>\n};
printf qq{<text x="24" y="64" font-size="12.5" fill="#666">Sus: conectoarele din masina. Jos: modulul%s. Fiecare fir pleaca din pinul lui; cifra de pe bara = numarul de pini al conectorului.</text>\n},
       ($has_rs ? ' si remote starter-ul' : '');
print qq{<text x="24" y="84" font-size="12.5" fill="#8a6d00">Firele cu cerc gol nu sint conectate: nu se stie inca la ce pin din masina merg.</text>\n};

my @legend;   # legendele conectoarelor, tiparite dupa fire
# --- blocurile de vehicul --------------------------------------------------
for my $b (@blocks) {
    next unless $blk{$b};
    my $B  = $blk{$b};
    my @t  = grep { $_->{blk} eq $b } @veh;
    my $st = $B->{used} ? '#0b5394' : '#999';

    printf qq{<text x="%d" y="%d" font-size="13" font-weight="600" fill="%s">%s</text>\n},
           $B->{x}, $BAND_Y + 6, $st, esc($b);
    if ($blk_img{$b}) {
        my $d = b64($blk_img{$b});
        printf qq{<image x="%d" y="%d" width="%d" height="%d" xlink:href="%s" href="%s"%s/>\n},
               $B->{x}, $B->{iy}, $B->{iw}, $B->{ih}, $d, $d,
               ($B->{used} ? '' : ' opacity="0.45"');
    } else {
        # alimentarea nu are desen: "+12V" si simbolul de masa
        my ($x, $y) = ($B->{x} + 6, $B->{iy});
        printf qq{<text x="%d" y="%d" font-size="24" font-weight="700" fill="#b8352b">+12V</text>\n}, $x, $y + 34;
        printf qq{<line x1="%d" y1="%d" x2="%d" y2="%d" stroke="#3c4043" stroke-width="2"/>\n},
               $x + 44, $y + 48, $x + 44, $y + 62;
        for my $i (0 .. 2) {
            printf qq{<line x1="%d" y1="%d" x2="%d" y2="%d" stroke="#3c4043" stroke-width="3"/>\n},
                   $x + 44 - 26 + $i * 8, $y + 62 + $i * 9, $x + 44 + 26 - $i * 8, $y + 62 + $i * 9;
        }
    }
    for my $t (@t) {
        my $c = $t->{ref} ne '' ? '#1a1a1a' : '#aaa';
        my $s = ($t->{pin} ne '' ? "pin $t->{pin}: " : '') . $t->{desc};
        $c = '#6a3d9a' if $t->{from_rs};
        # legendele se tiparesc la sfirsit, ca traseele sa treaca pe sub ele
        push @legend, sprintf(
            qq{<rect x="%.1f" y="%.1f" width="%.1f" height="15" fill="#ffffff"/>\n}
          . qq{<text x="%d" y="%.1f" font-size="%s" fill="%s">%s</text>\n},
            $B->{x} + 10, $t->{by} - 7, twidth($s, $FSB) + 10,
            $B->{x} + 14, $t->{by} + 4, $FSB, $c, esc($s));
        next if $t->{ref} eq '' && !$t->{from_rs};
        my $dotc = $t->{from_rs} ? '#6a3d9a' : '#0b5394';
        if ($t->{anchor}) {
            my @p = map { [ $B->{x} + $_->[0] * $B->{iw}, $B->{iy} + $_->[1] * $B->{ih} ] }
                    @{ $t->{anchor} };
            # doua ancore = aceeasi borna pe doua cai (diodele PTS): se unesc
            printf qq{<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="%s" stroke-width="1.6"/>\n},
                   $p[0][0], $p[0][1], $p[1][0], $p[1][1], $dotc if @p > 1;
            printf qq{<circle cx="%.1f" cy="%.1f" r="3.2" fill="%s"/>\n}, @$_, $dotc for @p;
        } else {
            printf qq{<circle cx="%d" cy="%.1f" r="3.4" fill="%s"/>\n},
                   $B->{x} + 4, $t->{by}, $dotc;
        }
    }
    unless ($B->{used}) {
        printf qq{<text x="%d" y="%.1f" font-size="10" fill="#c0392b">%s</text>\n},
               $B->{x}, $B->{bot} + 2, esc($blk_note{$b} // "nefolosit de VW08EU pe acest firmware");
    }
}

# --- modulul ---------------------------------------------------------------
if ($artwork) {
    my $d = b64($IMG_MOD);
    printf qq{<image x="%d" y="%d" width="%d" height="%d" xlink:href="%s" href="%s"/>\n},
           $MOD_X, $MOD_Y, $MOD_W, $MOD_H, $d, $d;
} else {
    printf qq{<rect x="%d" y="%d" width="%d" height="%d" rx="10" fill="#9aa0a6" stroke="#5f6368" stroke-width="2"/>\n},
           $MOD_X, $MOD_Y, $MOD_W, $MOD_H;
    printf qq{<text x="%d" y="%d" font-size="30" font-weight="700" fill="#fff" text-anchor="middle">%s</text>\n},
           $MOD_X + $MOD_W/2, $MOD_Y + $MOD_H/2, $plat;
    for my $k (grep { defined $G->{$_}{y0} } keys %$G) {
        my ($x, $y0, $y1) = bar($k);
        printf qq{<rect x="%.1f" y="%.1f" width="24" height="%.1f" rx="3" fill="%s" stroke="#3c4043"/>\n},
               $x - 12, $y0, $y1 - $y0, $G->{$k}{fill};
        printf qq{<text x="%.1f" y="%.1f" font-size="12" font-weight="700" fill="#fff" text-anchor="middle" transform="rotate(-90 %.1f %.1f)">%d</text>\n},
               $x, ($y0+$y1)/2 + 4, $x, ($y0+$y1)/2 + 4, $G->{$k}{pins};
    }
}

# eticheta cu fundal alb, ca traseele sa poata trece pe sub ea
sub label {
    my ($x, $y, $txt, $anchor) = @_;
    my $w  = twidth($txt, $FS);
    my $x0 = $anchor eq 'end' ? $x - $w - 4 : $x - 4;
    printf qq{<rect x="%.1f" y="%.1f" width="%.1f" height="17" fill="#ffffff"/>\n}, $x0, $y - 12, $w + 8;
    printf qq{<text x="%.1f" y="%.1f" font-size="%s" text-anchor="%s" fill="#1a1a1a">%s</text>\n},
           $x, $y + 4, $FS, $anchor, esc($txt);
}

# --- firele din stinga -----------------------------------------------------
my $lane = 0;
my %riser_of;
for my $wr (@left) {
    my $py  = pin_y($wr->{tag}, $wr->{pin});
    my $ry  = $wr->{row};
    my $txt = sprintf('%s/%d  %s: %s', $wr->{tag}, $wr->{pin}, $wr->{desc}, $wr->{color});
    my $v   = $veh_of{"$wr->{tag}/$wr->{pin}"};
    my $col = $wr->{rs} ? '#1f7a4d' : ($v ? '#0b5394' : '#3c4043');
    my $dash = $wr->{w2w} ? q{ stroke-dasharray="7 4"} : q{};
    # firele astea ocolesc pe dedesubt, deci borna de pe starter nu incape
    # linga el: se scrie in eticheta
    $txt .= '  -> ' . ($rs_term{"$wr->{tag}/$wr->{pin}"} // mirror($wr->{func}))
        if $wr->{rs};

    printf qq{<polyline points="%.1f,%.1f %.1f,%.1f %.1f,%.1f %.1f,%.1f" fill="none" stroke="%s" stroke-width="1.6"%s/>\n},
           $MOD_X, $py, $MOD_X - 14, $py, $LBL_L + 18, $ry, $LBL_L + 6, $ry, $col, $dash;

    if ($wr->{rs}) {
        my $dx = 40 + $lane * 9;
        my $by = $BUS_Y + $lane * 10;
        my $ux = $RS_X + 42 + $lane * 18;
        printf qq{<polyline points="%.1f,%.1f %.1f,%.1f %.1f,%.1f %.1f,%.1f %.1f,%.1f" fill="none" stroke="%s" stroke-width="1.6"%s/>\n},
               $LBL_L, $ry, $dx, $ry, $dx, $by, $ux, $by, $ux, $MOD_Y + $RS_H, $col, $dash;
        $lane++;
    } elsif ($v) {
        my $B = $blk{ $v->{blk} };
        my $k = $riser_of{ $v->{blk} }++;
        my ($tx, $ty, $rx);
        if ($v->{anchor}) {
            # firul intra direct pe cavitatea pinului, ca in schema originala
            my $a = $v->{anchor}[0];
            ($tx, $ty) = ($B->{x} + $a->[0] * $B->{iw}, $B->{iy} + $a->[1] * $B->{ih});
            # se apropie din partea in care pleaca si mustata din desenul original
            # dupa latimea DESENULUI, nu a blocului: latimea blocului e data de
            # legenda, si traseul ar ajunge peste conectorul vecin
            $rx = $a->[0] >= 0.5 ? $B->{x} + $B->{iw} + 18 + $k * 9
                                 : $B->{x} - 20 - $k * 9;
        } else {
            ($tx, $ty) = ($B->{x} + 4, $v->{by});
            $rx = $B->{x} - 20 - $k * 9;
        }
        printf qq{<polyline points="%.1f,%.1f %.1f,%.1f %.1f,%.1f %.1f,%.1f" fill="none" stroke="%s" stroke-width="1.6"%s/>\n},
               $LBL_L, $ry, $rx, $ry, $rx, $ty, $tx, $ty, $col, $dash;
    } else {
        printf qq{<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="%s" stroke-width="1.6"%s/>\n},
               $LBL_L, $ry, $VEH_X, $ry, $col, $dash;
        printf qq{<circle cx="%.1f" cy="%.1f" r="4" fill="#fff" stroke="#3c4043" stroke-width="1.6"/>\n},
               $VEH_X, $ry;
    }
    label($LBL_L, $ry, $txt, 'end');
}

# --- firele din dreapta ----------------------------------------------------
for my $wr (@right) {
    my $py  = pin_y($wr->{tag}, $wr->{pin});
    my $ry  = $wr->{row};
    my $txt = sprintf('%s/%d  %s: %s', $wr->{tag}, $wr->{pin}, $wr->{desc}, $wr->{color});
    my $col = $wr->{rs} ? '#1f7a4d' : '#3c4043';
    my $dash = $wr->{w2w} ? q{ stroke-dasharray="7 4"} : q{};

    printf qq{<polyline points="%.1f,%.1f %.1f,%.1f %.1f,%.1f %.1f,%.1f" fill="none" stroke="%s" stroke-width="1.6"%s/>\n},
           $MOD_X + $MOD_W, $py, $MOD_X + $MOD_W + 14, $py, $LBL_R - 18, $ry, $LBL_R - 6, $ry, $col, $dash;
    my $end = $wr->{rs} ? $RS_X + 4 : $LBL_R + twidth($txt, $FS) + 130;
    printf qq{<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="%s" stroke-width="1.6"%s/>\n},
           $LBL_R, $ry, $end, $ry, $col, $dash;
    label($LBL_R, $ry, $txt, 'start');
    if ($wr->{rs}) {
        printf qq{<text x="%d" y="%.1f" font-size="10.5" fill="#1f7a4d" text-anchor="end">%s</text>\n},
               $RS_X - 8, $ry - 5, esc($rs_term{"$wr->{tag}/$wr->{pin}"} // mirror($wr->{func}));
    } else {
        printf qq{<circle cx="%.1f" cy="%.1f" r="4" fill="#fff" stroke="#3c4043" stroke-width="1.6"/>\n},
               $end, $ry;
    }
}

# --- remote starter + cablul D2D ------------------------------------------
if ($has_rs) {
    if ($artwork) {
        my $d = b64($IMG_RS);
        printf qq{<image x="%d" y="%d" width="%d" height="%d" xlink:href="%s" href="%s"/>\n},
               $RS_X, $MOD_Y, $RS_W, $RS_H, $d, $d;
    } else {
        printf qq{<rect x="%d" y="%d" width="%d" height="%d" rx="8" fill="#9aa0a6" stroke="#5f6368" stroke-width="2"/>\n},
               $RS_X, $MOD_Y, $RS_W, $RS_H;
        printf qq{<text x="%d" y="%d" font-size="16" font-weight="700" fill="#fff" text-anchor="middle">Remote Starter</text>\n},
               $RS_X + $RS_W/2, $MOD_Y + $RS_H/2;
    }
    # legaturi care pleaca de la remote starter direct in masina, fara sa treaca
    # prin modul (in DB2: Lock/Unlock Output -> pinul de Key Lock din BCM)
    my $k = 0;
    for my $t (grep { $_->{from_rs} } @veh) {
        my $B    = $blk{ $t->{blk} } or next;
        my $lane = $MOD_Y - 84 + $k * 16;
        my ($tx, $ty) = ($B->{x} + 4, $t->{by});
        if ($t->{anchor}) {
            my $a = $t->{anchor}[0];
            ($tx, $ty) = ($B->{x} + $a->[0] * $B->{iw}, $B->{iy} + $a->[1] * $B->{ih});
        }
        my $rx = $B->{x} - 60 - $k * 9;
        printf qq{<polyline points="%d,%.1f %d,%.1f %.1f,%.1f %.1f,%.1f %.1f,%.1f" fill="none" stroke="#6a3d9a" stroke-width="1.6" stroke-dasharray="7 4"/>\n},
               $RS_X, $MOD_Y + 60 + $k * 14, $RS_X - 90 - $k * 14, $MOD_Y + 60 + $k * 14,
               $RS_X - 90 - $k * 14, $lane, $rx, $lane, $rx, $ty;
        printf qq{<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="#6a3d9a" stroke-width="1.6" stroke-dasharray="7 4"/>\n},
               $rx, $ty, $tx, $ty;
        printf qq{<text x="%.1f" y="%.1f" font-size="10.5" fill="#6a3d9a">%s</text>\n},
               $RS_X - 86 - $k * 14, $MOD_Y + 56 + $k * 14, esc($t->{from_rs});
        $k++;
    }

    if (exists $G->{H4}) {
        my (undef, $y0, $y1) = bar('H4');
        my $port = $RS_X + int($RS_W * 0.485);
        my $top  = $MOD_Y - 48;
        printf qq{<polyline points="%d,%.1f %d,%.1f %d,%d %d,%d %d,%d" fill="none" stroke="#c0392b" stroke-width="3"/>\n},
               $MOD_X + $MOD_W, ($y0+$y1)/2, $RS_X - 150, ($y0+$y1)/2,
               $RS_X - 150, $top, $port, $top, $port, $MOD_Y + 6;
        printf qq{<text x="%d" y="%d" font-size="11.5" font-weight="600" fill="#c0392b" text-anchor="middle">H4  D2D 4-pin (XKD2D65)</text>\n},
               ($MOD_X + $MOD_W + $RS_X) / 2, $top - 8;
    }
}

print @legend;
printf qq{<text x="24" y="%d" font-size="11" fill="#666">Albastru = catre conectoarele din masina.%s  Cercurile goale = pini ramasi neconectati.</text>\n},
       $H - 22, ($has_rs ? ' Verde = catre remote starter, cu borna lui alaturi.'
                         . ' Mov = de la remote starter direct in masina, fara sa treaca prin modul.' : '');
printf qq{<text x="24" y="%d" font-size="11" fill="#666">Linia intrerupta = fir necesar doar in instalare W2W; in D2D informatia trece pe magistrala si firul nu se leaga.</text>\n},
       $H - 6;
print "</svg>\n";
