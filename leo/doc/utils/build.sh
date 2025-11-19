#!/usr/bin/env bash

set -xe
mkdir -p build

pdflatex -interaction=nonstopmode -halt-on-error -file-line-error -shell-escape -output-directory=build changelog.tex
bibtex build/changelog
pdflatex -interaction=nonstopmode -halt-on-error -file-line-error -shell-escape -output-directory=build changelog.tex
pdflatex -interaction=nonstopmode -halt-on-error -file-line-error -shell-escape -output-directory=build changelog.tex
