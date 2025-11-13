#!/usr/bin/python

import sys,string
from subprocess import call
import shutil, os

file = open(sys.argv[1])
data = file.readlines()
file.close()

for line in data:
	aux = line.find('epsfile')
	if (aux == 0):
		filename = (line[9:-2])[:-4]
		break
print(filename)

call(['gnuplot' ,sys.argv[1]])

texfiledata =""" 
\\documentclass[prl,10pt]{revtex4}
\\usepackage[dvips]{graphicx}
\\usepackage[latin1]{inputenc}
\\usepackage{amsmath}
\\usepackage{xcolor}
\\begin{document}
\\thispagestyle{empty}

\\begin{figure}
\\input{"%(filename)s"}
\\end{figure} 

\\end{document}
"""%locals()

file = open("aux1459.tex",'w')
file.write(texfiledata)
file.close()

call(['latex', 'aux1459.tex'])
call(['dvips', "aux1459.dvi" ])
call(['ps2eps', "aux1459.ps" ])

os.remove("aux1459.tex")
os.remove("aux1459.dvi")
os.remove("aux1459.ps")
os.remove("aux1459.log")
os.remove("aux1459.aux")
os.remove(filename+".tex")
shutil.move("aux1459.eps", filename+".eps")
call(['ps2pdf',  "-dEPSCrop",  filename+".eps"])
# Convierte a BMP usando ImageMagick
call(['convert', '-density', '300', '-background', 'white', '-flatten',
      filename+".pdf", filename+".bmp"])

# ==== NUEVO: eliminar PDF y BMP ====
try:
    os.remove(filename + ".eps")
    os.remove(filename + ".bmp")
    print("Archivos EPS y BMP eliminados, solo queda:", filename + ".pdf")
except FileNotFoundError:
    pass