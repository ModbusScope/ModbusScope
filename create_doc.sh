#!/bin/bash

DOCS_FOLDER="docs/manual"

cd $DOCS_FOLDER

### Generate HTML docs

python3 create_venv.py --requirements requirements.txt
. venv/bin/activate

make html

### Generate PDF
# requires following packages on Ubuntu: latexmk texlive-latex-recommended texlive-latex-extra

make latexpdf