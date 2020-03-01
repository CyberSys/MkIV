## fgrep -r TODO ../src/ > todos.out
fgrep -r TODO ../src/ | grep -v "/\.svn/" > todos.out
