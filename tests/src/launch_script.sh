#!/bin/sh
# Harmless script launched from inside /home.
# The interpreter must be able to read it (bind mount) and the log
# must not report those reads as BLOCKED.
echo HELLO
