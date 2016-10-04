#!/bin/bash
pdflatex -shell-escape main.tex
biber main
biber main
pdflatex -shell-escape main.tex
atril main.pdf &
