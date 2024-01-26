#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shlwapi.h>
#include <dirent.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <io.h>
#define MAX_SIZE 1000
int dir_exists(char *name)
{
    char cwd[MAX_SIZE];
    char parent[MAX_SIZE] = "..";
    if (GetCurrentDirectory(sizeof(cwd), cwd) == 0)
    {
        perror("GetCurrentDirectory");
        return -1;
    }
    int result;
    while (strcmp(cwd, "C:\\") != 0)
    {
        result = access("ngit", F_OK);
        if (result == 0)
        {
            return 1;
        }
        if (strcmp(name, strrchr(cwd, '\\') + 1) == 0)
        {
            return 1;
        }
        if (SetCurrentDirectory(parent) == 0)
        {
            perror("SetCurrentDirectory");
            return -1;
        }
        if (GetCurrentDirectory(sizeof(cwd), cwd) == 0)
        {
            perror("GetCurrentDirectory");
            return -1;
        }
    }
    return 0;
}
void copy_global_to_all()
{
    char lines[MAX_SIZE];
    FILE *projects;
    projects = fopen("C:\\ngit\\Projects.txt", "r");
    fscanf(projects, "%[^\0]", lines);
    fclose(projects);
    char *one_line;
    one_line = strtok(lines, "\n");
    while (one_line != NULL)
    {
        printf("%s\n", one_line);
        char copy_global_variables[MAX_SIZE];
        char copy_global_user[MAX_SIZE];
        strcpy(copy_global_variables, "copy /y ");
        strcpy(copy_global_variables + strlen(copy_global_variables), "C:\\ngit\\GlobalVariables.txt ");
        strcpy(copy_global_variables + strlen(copy_global_variables), one_line);
        strcpy(copy_global_user, "copy /y ");
        strcpy(copy_global_user + strlen(copy_global_user), "C:\\ngit\\GlobalUser.txt ");
        strcpy(copy_global_user + strlen(copy_global_user), one_line);
        system(copy_global_variables);
        system(copy_global_user);
        one_line = strtok(NULL, "\n");
    }
}
void create_global_config(int argc, char **argv)
{
    if ((strncmp(argv[3], "user.name", 9) == 0) && argc > 4)
    {
        FILE *global_user;
        global_user = fopen("C:\\ngit\\GlobalUser.txt", "w");
        fprintf(global_user, "%s\n", argv[4]);
        fclose(global_user);
    }
    else if ((strncmp(argv[3], "user.email", 6) == 0) && argc > 4)
    {
        FILE *global_user;
        global_user = fopen("C:\\ngit\\GlobalUser.txt", "a");
        fprintf(global_user, "%s", argv[4]);
        fclose(global_user);
    }
    else if ((strncmp(argv[3], "alias", 5) == 0) && argc > 4)
    {
        char *alias = malloc(MAX_SIZE);
        alias = strtok(argv[3], ".");
        alias = strtok(NULL, ".");
        FILE *global_variable;
        global_variable = fopen("C:\\ngit\\GlobalVariables.txt", "a");
        fprintf(global_variable, "%s#%s\n", argv[4], alias);
        free(alias);
    }
    else
    {
    } // must be completed
}
void run_init(int argc, char **argv)
{
    char address[MAX_SIZE];
    GetCurrentDirectory(sizeof(address), address);
    int exist = dir_exists("ngit");
    SetCurrentDirectory(address);
    strcpy(address + strlen(address), "\\ngit");
    if (exist == 1)
        printf("Error : ngit folder is exist in here or it's parents\n");
    else if (exist == 0)
    {
        FILE *project_path_adding;
        project_path_adding = fopen("C:\\ngit\\Projects.txt", "a");
        fprintf(project_path_adding, "%s\n", address);
        fclose(project_path_adding);
        system("mkdir ngit");
        system("attrib +h ngit");
        system("cd ngit & type nul > PersonalVariables.txt & type nul > PersonalUser.txt & type nul > StageCase.txt & echo master > CommitsAndBranches.txt");
        copy_global_to_all();
    }
}
int main(int argc, char **argv)
{
    if ((strncmp(argv[0], "ngit", 4) == 0) && argc > 1)
    {
        if ((strncmp(argv[1], "config", 6) == 0) && argc > 2)
        {
            if ((strncmp(argv[2], "-global", 7) == 0) && argc > 3)
            {
                create_global_config(argc, argv);
                copy_global_to_all();
            }
            else if ((strncmp(argv[2], "user.name", 9) == 0) && argc > 3) // must be completed
            {
            }
            else if ((strncmp(argv[2], "user.email", 6) == 0) && argc > 3) // must be completed
            {
            }
            else if ((strncmp(argv[2], "alias", 5) == 0) && argc > 3) // must be completed
            {
            }
            else
            {
            } // must be completed
        }
        else if ((strncmp(argv[1], "init", 4) == 0) && argc == 2)
        {
            run_init(argc, argv);
        }
        else
        {
        } // must be completed
    }
    else
    {
        printf("Error : command not found. please put \"ngit\" in first of your command and type the rest of command.\n");
    }
}