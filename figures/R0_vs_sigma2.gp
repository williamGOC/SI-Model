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
epsfile='R0_vs_sigma2.eps'
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

set key samplen 1 spacing 1.2 font ",15" at graph 0.95, 0.95
set xlabel '\Large{$\sigma^{2}$}'
set ylabel '\Large{$R_{0}$}' offset 0.1,0.1

set format x "$10^{%T}$"

set logscale x

#set xrange [0:0.12]
set yrange [0:]


#set label 1 '$\tau_p=1$' at graph 0.7, 0.25

set style fill solid 0.3 noborder

plot "../R0/resultados_beta_sigma.dat" u (($1==0.2)?($2*$2):(NaN)):3:4 w yerrorline pt 5 ps 1.3 lw 2 lc rgb 'red' title '$\beta=0.2$',\
     "../R0/resultados_beta_sigma.dat" u (($1==0.6)?($2*$2):(NaN)):3:4 w yerrorline pt 7 ps 1.3 lw 2 lc rgb 'blue' title '$0.6$',\
     "../R0/resultados_beta_sigma.dat" u (($1==1.0)?($2*$2):(NaN)):3:4 w yerrorline pt 9 ps 1.3 lw 2 lc rgb 'magenta' title '$1.0$',\
     #"./diagram_LX32.dat" u (($3==32 && $4==0.200)?($5):(NaN)):6 w lp pt 13 ps 1.3 lw 2 lc rgb 'olive' title '$0.20$',\
     #"./STRAIN_X/MEAN_VALUES.dat" u (($3==256)?($5):(NaN)):9 w p pt 15 ps 1.3 lw 2 lc rgb 'cyan' title '$256$';


########### PANEL CENTRAL (Nz) ###########

unset multiplot