#!/bin/bash
#pdflatex essay_kladd.tex
biber essay_kladd
biber essay_kladd
pdflatex essay_kladd.tex
okular essay_kladd.pdf &

 
