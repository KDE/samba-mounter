#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.rc -o -name \*.ui -o -name \*.kcfg` >> rc.cpp
$XGETTEXT `find . -name \*.qml -o -name \*.h -o -name \*.cpp` -o $podir/kcm_sambamounter.pot
rm -f rc.cpp
