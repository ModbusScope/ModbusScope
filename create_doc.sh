#!/bin/bash

VENV_DIR="venv"
DOCS_FOLDER="docs"

cd $DOCS_FOLDER

if [ ! -d "$VENV_DIR" ]; then
  # Take action if $DIR exists. #

  python3 create_venv.py --requirements requirements.txt

fi

source venv/bin/activate

make html
