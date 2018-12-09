#!/bin/sh

find ../../src/ansa -name "*.cc" -o -name "*.h" -o -name "*.msg" -o -name "*.ned" | grep -v ../../src/inet/transportlayer/tcp_lwip/lwip  | grep -v '_m.' | sort >filelist4rename.txt
#find ../../tests -name "*.cc" -o -name "*.h" -o -name "*.msg" -o -name "*.ned" -o -name "*.test" | grep -v '_m.' | sort >>filelist4rename.txt
