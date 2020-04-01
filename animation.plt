set terminal gif animate delay 1
set output 'animation.gif'
do for [i=1:10000] {plot [-10:160] [-10:110] sprintf('gif_files/time_%d.dat',i) with points linecolor variable pointtype 7 pointsize 1.0 title sprintf("time = %d",i); pause 0.0005}
set output
