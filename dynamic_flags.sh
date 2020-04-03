#!/bin/bash

if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "-dynamic"
else
    echo "-Wl,--export-dynamic"
fi
