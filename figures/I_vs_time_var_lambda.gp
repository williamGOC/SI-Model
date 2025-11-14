#!/usr/bin/gnuplot

# Dimensiones (en pulgadas)
mpl_top    = 0.3
mpl_bot    = 0.4
mpl_left   = 0.4
mpl_right  = 0.3
mpl_height = 2.0  # Altura común a todos
mpl_width  = 2.0  # Ancho de cada panel
mpl_dx1    = 0.5  # Espacio entre panel 1 y 2
mpl_dx2    = 0.5  # Espacio entre panel 2 y 3

# Tamaño total
xsize = mpl_left + mpl_right + 3*mpl_width + mpl_dx1 + mpl_dx2
ysize = mpl_top + mpl_bot + mpl_height

# Posiciones horizontales (left y right en pantalla)
p1_left  = mpl_left / xsize
p1_right = (mpl_left + mpl_width) / xsize

p2_left  = (mpl_left + mpl_width + mpl_dx1) / xsize
p2_right = (mpl_left + 2*mpl_width + mpl_dx1) / xsize

p3_left  = (mpl_left + 2*mpl_width + mpl_dx1 + mpl_dx2) / xsize
p3_right = (mpl_left + 3*mpl_width + mpl_dx1 + mpl_dx2) / xsize

# Verticales
p_top = 1 - mpl_top / ysize
p_bot = mpl_bot / ysize


# Salida
set terminal epslatex size xsize,ysize
epsfile='I_vs_time_var_lambda.eps'
set output epsfile

set border linewidth 1.5
set samples 1000
set grid lc rgb "#BBBBBB"
set style line 1 lc rgb '#0060ad' lt 1 lw 2 pt 7 ps 0.5

set multiplot

########### PANEL IZQUIERDO (DEFF) ###########
set lmargin at screen p1_left
set rmargin at screen p1_right
set tmargin at screen p_top
set bmargin at screen p_bot

set key samplen 1 spacing 1.2 font ",15" at graph 0.85, 0.75
set xlabel '\Large{$t$}'
set ylabel '\Large{$\langle I_t\rangle$}' offset 0.1,0.1 rotate by 0

set format "$10^{%T}$"

set logscale

set xrange [:1000]
#set yrange [0:]


set label 1 '$\beta=0.9$' at graph 0.4, 0.55
set label 2 '$\alpha=1.0$' at graph 0.4, 0.475
set label 3 '$\sigma=0.5$' at graph 0.4, 0.400

set style fill solid 0.3 noborder

plot "../SERIE/lambda_0.5000/MEAN_TEMPORAL.dat" u 2:5 w lp pt 5 ps 1.3 lw 2 lc rgb 'red' title '$\lambda=0.5$',\
     "../SERIE/lambda_1.0000/MEAN_TEMPORAL.dat" u 2:5 w lp pt 7 ps 1.3 lw 2 lc rgb 'blue' title '$1.0$',\
     "../SERIE/lambda_1.5000/MEAN_TEMPORAL.dat" u 2:5 w lp pt 9 ps 1.3 lw 2 lc rgb 'magenta' title '$1.5$',\
     "../SERIE/lambda_2.0000/MEAN_TEMPORAL.dat" u 2:5 w lp pt 11 ps 1.3 lw 2 lc rgb 'olive' title '$2.0$';


########### PANEL CENTRAL (Nz) ###########

unset multiplot