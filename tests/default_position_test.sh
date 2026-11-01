#!/bin/sh
# perft before any "position" command must use the standard start position.
out=$(printf 'perft 4\nquit\n' | "${1:-./chess-engine}" | grep '^perft(4)')
case "$out" in
  "perft(4) = 197281 "*) echo "Default start position PASS"; exit 0 ;;
  *) echo "Default start position FAIL: $out"; exit 1 ;;
esac
