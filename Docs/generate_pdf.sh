#!/bin/bash
pandoc -s --highlight-style tango README.md  -o README.pdf
pandoc -s --highlight-style tango README_ESP.md  -o README_ESP.pdf

