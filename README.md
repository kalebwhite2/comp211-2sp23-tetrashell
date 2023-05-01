#Tetrashell
###Created by: Kenechukwu Ochuba and Kaleb White

Welcome to Tetrashell! Our implementation is the shell of your dreams - the shell you've been wanting your whole life. 

##Getting started
To start out, run

```
./tetrashell
```

If the executable isn't present, use the Makefile by running the command 

```
make
```

Upon running `./tetrashell`, you will be asked for a valid tetris savefile. If using the server directory,`starter2.bin` is available.
If the filename you provide is not valid, you will be prompted again until the file you give is valid and can be opened, or until the shell reads 'exit'.
Once you have a valid file, you can type and run commands.

###Prompt
The prompt in the following format: username@TShell\[star...\]\[300/8\] tetrashell>.
From left to right: username @Tshell, first four letters of the current quicksave file, current game score, current game lines.
Username is taken from the environment variables; all other info is saved locally when a savefile pathname is entered.
Quick inputs are supported; try using any shorthand of any command names.

##Running Commands
###Advanced modify
Advanced modify has a number of uses. It can be called with:

1. modify score #
2. modify lines #
3. modify board x y #
4. modify next\_piece (# between -1 and 19)
5. modify current\_piece x y

Calling any of these modify commands will change the current quicksave as specified.

###Rank
Rank is used to observe the current global leaderboard. For example, try `rank score 10`
The aforementioned rank command will rank the top 10 quicksaves by score and print them out. Rank can also be specified to use lines.
Quick rank is also implemented, so if you only type rank and a metric, such as score, it will print out the top 10 games by that metric from the leaderboard. 
If the command given is just the word rank, our pretty rank implementation will print out the 5 quicksaves above yours, if possible, your quicksave file, and then the five quicksave files below your file, if possible.

###Check
Check is implemented as suggested, printing out whether the file is legitimate or not.

###Recover
Recover is implemented as suggested. 
However, advanced pretty recover is also implemented. Recovered quicksaves are printed out in the format
```
 ---- --------------------- ------- -------
 #    File path             Score   Lines
 ---- --------------------- ------- -------
```
It also allows the user to switch the current quicksave to any of the quicksaves recovered (within the max 100 printed).

###Exit
The “exit” command, if called, causes tetrashell to exit. 

###Switch
The switch command, if called using, `switch filepath`, switches the current quicksave to the quick save from the filepath given. 
If the shell encounters an error trying to open and read from the given filepath, it will prompt the user until the `exit` command is given
or the file can be found.

###Help
The help command, if called using, `help recover`, would return information on the recover command.
`help` works with recover, modify, check, rank, undo, visualize, exit, info and switch.

###Undo
The undo function allows the user to undo a modify command. It allows up to three undos of modify commands.
If the save file is switched by `switch` or some instances of `recover`, all buffered saves will no longer be accesible.

###Info, Visualize, Train
The info, visualize and train functions are all implemented as suggested. To call them:
*`info`
*`visualize`
*`train`


#Enjoy TETRASHELL... THE SHELL OF YOUR DREAMS!

