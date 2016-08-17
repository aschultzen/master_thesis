#!/bin/bash
pdflatex main.tex
biber main
biber main
pdflatex main.tex
atril main.pdf &
