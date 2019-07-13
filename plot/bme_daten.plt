# waage_daten.plt 
#
# für Schriftart:
#   apt-get install cm-super groff
#

set encoding utf8

set xdata time
set x2data time
set timefmt "%d.%m.%Y-%H:%M:%S"

set lmargin 8
set rmargin 3

# folgende Zeilen aktivieren, um EPS zu generieren
# #
#set terminal postscript eps enhanced color fontfile '/usr/share/texmf/fonts/type1/public/cm-super/sfrm1000.pfb' "SFRM1000" size 16cm,20cm
#set output "bme_daten.eps"
#set terminal png size 1200,800
#set terminal png size 1600,900
set terminal pngcairo size 1600,900
set output datei.".".ext.".png"

set style line 3 lt 1 lc rgb "light-blue" lw 2
set style line 4 lt 1 lc rgb "blue" lw 3
set style line 5 lt 1 lc rgb "light-pink" lw 1
set style line 8 lt 1 lc rgb "brown" lw 3

set grid front

set multiplot layout 4,1 title "Temperatur, Luftdruck, Feuchtigkeit, Regen".datei offset -0.01 scale 0.95 # offset 0,-0.01

set format x "%d.%m\n%H:%M"
set mxtics 4

set yrange [*<5:15<*]
set ylabel "Temperatur"
set ytics 5
set ytics add ("Flug" 12)
set y2tics mirror
set y2range [*<5:15<*]
set y2tics 5

plot f(x) = 12, f(x)  axis x1y1 with lines ls 3 title "Flugtemperatur", \
     datei.".".ext using 1:2 axis x1y1 with lines ls 4 title "Temperatur [°C]

set format x ""
set format x2 ""

set yrange [*<1010:1015<*]
set ylabel "Luftdruck"
set ytics 5
set ytics add ("Normale" 1013)
set y2tics 5
set y2range [*<1010:1015<*]

plot f(x) = 1013, f(x)  axis x1y1 with lines ls 3 title "Normale", \
     datei.".".ext using 1:3 axis x1y1 with lines ls 4 title "Luftdruck [hPa]", \
     datei.".".ext using 1:3 smooth bezier axis x1y1 with lines ls 8 title "Luftdruck - Bezier"

set yrange [*:*]
set ylabel "Feuchtigkeit"
set y2range [*:*]

plot datei.".".ext using 1:4 axis x1y1 with lines ls 5 title "Feuchtigkeit [\%]", \
     datei.".".ext using 1:4 smooth bezier axis x1y1 with lines ls 8 title "Feuchtigkeit - Bezier"
     
set ylabel "Regen"
set yrange [0:1<*]
set y2range [0:1<*]

plot datei.".".ext using 1:5 axis x1y1 with boxes title "Regen [l/m2]"
     
unset multiplot

