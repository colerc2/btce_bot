#!/bin/bash
git pull
git add -A
git config --global user.name "Bob Cole"
git config --global user.email colerc2@miamioh.edu
git commit -a --author="Bob Cole <cole.robert.c@gmail.com>"
git push
