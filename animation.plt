set terminal gif animate delay 1
set output 'animation.gif'
do for [i=1:2000] {plot [-10:160] [-10:110] sprintf('gif_files/time_%d.dat',i) with points linecolor variable pointtype 7 pointsize 1.0; pause 0.02}
set output
