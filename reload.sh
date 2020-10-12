#!/bin/bash
function ph()  {
    for ((i = 0;i < ${#1};i++)); do
        echo -n -
    done
    echo
    echo $1
    for ((i = 0;i < ${#1};i++)); do
        echo -n -
    done
    echo
}

ph "Rebuilding..."
make clean
make

echo
RUNNING=$(launchctl list | grep com.lucas.sts | wc -l)

ph "Disabling current instance..."
if [[ $RUNNING -eq 1 ]]; then
    launchctl unload ~/Library/LaunchAgents/com.lucas.sts.plist
    echo Done
else
    echo No active instance found
fi
echo
ph "Relaunching..."
launchctl load ~/Library/LaunchAgents/com.lucas.sts.plist
echo Done