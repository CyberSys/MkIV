## fgrep -r HACK ../src/ > hacks.out
fgrep -r HACK ../src/ | grep -v "/\.svn/" > hacks.out
