#!/bin/bash
# Run xmllint on all xds files in this directory
for x in *.xds;
    do (xmllint --noout --dtdvalid xds.dtd $x);
done

