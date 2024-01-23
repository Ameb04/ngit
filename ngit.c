#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_SIZE 100
struct brancehs
{
    char name[MAX_SIZE];
    struct brancehs *next_brancehs[MAX_SIZE];
    struct brancehs *previous_brancehs[MAX_SIZE];
};

int main()
{
    while (1)
    {
        // getting command
        char pieces[MAX_SIZE][MAX_SIZE];
        char command[MAX_SIZE];
        gets(command);
        char save_command_to_show_errors[MAX_SIZE];
        strcpy(save_command_to_show_errors, command);
        // delimating command with space
        int pieces_iterator_and_num = 0;
        char *pieces_token;
        pieces_token = strtok(command, " ");
        while (pieces_token != NULL)
        {
            strcpy(pieces[pieces_iterator_and_num], pieces_token);
            pieces_token = strtok(NULL, " ");
            pieces_iterator_and_num++;
        }
        // starting proccess
        if ((strstr(pieces[0], "ngit") != NULL) && pieces_iterator_and_num > 1) // checking that first piece of command is "ngit" and there is a rest command
        {
        }
        else
        {
            printf("command \"%s\" not found : please put \"ngit\" in first of your command and type the rest of command \n", save_command_to_show_errors);
        }
    }
}