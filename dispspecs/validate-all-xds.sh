#!/bin/bash
# Run xmllint on all xds files in this directory
for x in *.xds;
  do 
  echo "Checking: $x";
  (xmllint --noout --dtdvalid xds.dtd $x);
done

