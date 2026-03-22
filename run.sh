#!/bin/env bash

# make sure everything is built

cppweb compile
cd components
make
cd ..
# compile/build/compile components/build/components.so input.html -o test.html

# for starting background jobs.
main_pids=()
trap 'for pid in ${main_pids[@]}; do kill $pid; done; exit' INT

start_bg() {
  $1 &
  main_pids=("${main_pids[@]}" $!)
}

reload_input() {
  echo -e "input.html\ncomponents/build/components.so" | entr -s "build/compile/compile components/build/components.so input.html -o test.html && echo reloaded!"
}

reload_components() {
  cd components
  find components/ | entr -s "make"
}

start_bg reload_input
start_bg reload_components

while sleep 10; do :; done
