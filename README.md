# short-term-storage
An app that tracks how long files have been in a folder and deletes them when they've overstayed their welcome!

## Instructions

Run the `make` command to build, and use `sts-start.sh` to run

## Contents

`sts.c` - The actual watcher

`Makefile` - Used to build the executable

`sts-start.sh` - A Bash script used to set this to run at login on a Mac

`reload.sh` - Terminates the existing launchctl process and starts a new one
