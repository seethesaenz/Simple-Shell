CS/SE 3377 Systems Programming in Unix and other Environments
Introduction

This project is a simple shell program that allows the user to execute commands on their system. It supports basic functionality such as changing directories, executing commands with arguments, and piping commands. In addition, it also includes a history feature that allows the user to access previously executed commands.

Features

    Change directories using the cd command.
    Execute commands with arguments.
    Pipe commands together using the | operator.
    Keep track of command history using the history command.
    Rerun a previously executed command using its index in the command history.
    Clear the command history using the history -c command.
    Exit the shell using the exit command.

Usage

To run the shell program, simply compile the program using the following command:

    gcc sish.c -o sish -Wall -Werror -std=gnu99

Then, run the program using the following command:

bash

    ./sish

Once the program is running, you can execute commands in the shell. The following commands are supported:

    cd: Change directories. Usage: cd [directory]
    history: View command history. Usage: history [index] or history -c
    exit: Exit the shell.
    Any other command that can be executed on the system.

To pipe commands together, simply separate the commands with the | operator. For example:

bash

    ls | grep shell | wc -l

To rerun a previously executed command, use the history command to view the command history, then use the index of the command to rerun it. For example:

bash

history
0 ls
1 cd ..
2 history
3 ./shell
4 ps aux | grep firefox
5 exit
history 0

This will rerun the command ls from the command history.
Limitations

This shell program is a very basic implementation and has some limitations. For example, it does not support background processes, input/output redirection, or advanced shell scripting. In addition, it may not handle certain types of commands or arguments correctly.
