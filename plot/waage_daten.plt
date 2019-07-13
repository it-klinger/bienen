# waage_daten.plt 
#
# für Schriftart:
#   apt-get install cm-super groff

set encoding utf8

set xdata time
set x2data time
set timefmt "%d.%m.%Y-%H:%M:%S"


set lmargin 8
set rmargin 3

set terminal pngcairo size 1600,900
set output datei.".".ext.".png"

set style line 3 lt 1 lc rgb "light-blue" lw 2
set style line 4 lt 1 lc rgb "blue" lw 3
set style line 5 lt 1 lc rgb "light-pink" lw 1
set style line 8 lt 1 lc rgb "brown" lw 3

set grid front

set multiplot layout 3,1 title stocka." ".stockb." ".datei offset -0.01 scale 0.95 # offset 0,-0.01

set format x "%d.%m\n%H:%M"
set mxtics 4

set yrange [*:*]
set ylabel "Masse"
set y2tics mirror
set y2range [*:*]

plot datei.".".ext using 1:5 axis x1y1 with lines ls 5 title stocka." - raw [g]", \
     datei.".".ext using 1:($5) smooth bezier axis x1y1 with lines ls 8 title stocka." - Bezier [g]"
     
set format x ""
set format x2 ""

plot datei.".".ext using 1:7 axis x1y1 with lines ls 5 title stockb." - raw [g]", \
     datei.".".ext using 1:($7) smooth bezier axis x1y1 with lines ls 8 title stockb." - Bezier [g]"

set yrange [*<5:15<*]
set ylabel "Temperatur"
set ytics 5
set ytics add ("Flug" 12)
set y2range [*<5:15<*]
set y2tics 5

plot f(x) = 12, f(x)  axis x1y1 with lines ls 3 title "Flugtemperatur", \
     datei.".".ext using 1:2 axis x1y1 with lines ls 4 title "Temperatur [°C]

unset multiplot

