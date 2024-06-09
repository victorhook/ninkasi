#!/bin/bash

INCLUDE_DIRS=$(find ./include/mavlink -type d)
INCLUDES=""
for dir in $INCLUDE_DIRS; do
    INCLUDES="$INCLUDES -I$dir"
done
echo $INCLUDES
