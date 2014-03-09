#!/bin/bash
git pull
git add -A
git config --global user.name "Bob Cole"
git config --global user.email cole.robert.c@gmail.com
git commit -a --author="Bob Cole <cole.robert.c@gmail.com>"
git push
