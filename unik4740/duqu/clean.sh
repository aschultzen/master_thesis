#!/bin/bash
rm *.aux
rm *.bbl
rm *.bcf 
rm *.blg
rm *.out
rm *.log
rm *.toc
rm *.run.xml
rm *.nav
rm *.snm
rm *.pdf
pdflatex statusmote_02_10_2015.tex 
pdflatex statusmote_02_10_2015.tex 
okular statusmote_02_10_2015.pdf

