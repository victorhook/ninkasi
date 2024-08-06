#!/bin/bash

# Set first argument to ninkasi environment
export NINKASI_ENV=$1

if [ -z "$NINKASI_ENV" ]; then
    echo "Running onboard ninkasi"
else
    echo "Running in ninkasi environment: $NINKASI_ENV"
fi

source ./venv/bin/activate
cd src
./main.py