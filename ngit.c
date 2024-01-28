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
#include <unistd.h>
#define debug(x) printf("*%s*\n", x);
#define MAX_SIZE 1000
char *project_path;
void copy_file(const char *src_path, const char *dst_path)
{
    FILE *src_file = fopen(src_path, "rb");
    if (src_file == NULL)
    {
        perror("fopen src_file");
        exit(EXIT_FAILURE);
    }
    FILE *dst_file = fopen(dst_path, "wb");
    if (dst_file == NULL)
    {
        perror("fopen dst_file");
        exit(EXIT_FAILURE);
    }
    char buffer[4096];
    size_t n;
    while ((n = fread(buffer, 1, sizeof(buffer), src_file)) > 0)
    {
        if (fwrite(buffer, 1, n, dst_file) != n)
        {
            perror("fwrite");
            exit(EXIT_FAILURE);
        }
    }
    if (ferror(src_file))
    {
        perror("fread");
        exit(EXIT_FAILURE);
    }
    fclose(src_file);
    fclose(dst_file);
}
void copy_folder(const char *src_path, const char *dst_path)
{
    char *src_pattern = (char *)malloc(strlen(src_path) + 3);
    strcpy(src_pattern, src_path);
    strcat(src_pattern, "/*");
    DIR *dir = opendir(src_path);
    if (dir == NULL)
    {
        perror("opendir");
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }
        char *src_file = (char *)malloc(strlen(src_path) + strlen(entry->d_name) + 2);
        char *dst_file = (char *)malloc(strlen(dst_path) + strlen(entry->d_name) + 2);
        sprintf(src_file, "%s/%s", src_path, entry->d_name);
        sprintf(dst_file, "%s/%s", dst_path, entry->d_name);
        if (entry->d_type == DT_DIR)
        {
            if (mkdir(dst_file) != 0)
            {
                perror("mkdir");
            }
            copy_folder(src_file, dst_file);
        }
        else
        {
            FILE *src_fp = fopen(src_file, "rb");
            if (src_fp == NULL)
            {
                perror("fopen src_file");
                continue;
            }
            FILE *dst_fp = fopen(dst_file, "wb");
            if (dst_fp == NULL)
            {
                perror("fopen dst_file");
                continue;
            }
            char buffer[4096];
            size_t n;
            while ((n = fread(buffer, 1, sizeof(buffer), src_fp)) > 0)
            {
                if (fwrite(buffer, 1, n, dst_fp) != n)
                {
                    perror("fwrite");
                    break;
                }
            }
            if (ferror(src_fp))
            {
                perror("fread");
            }
            fclose(src_fp);
            fclose(dst_fp);
        }
        free(src_file);
        free(dst_file);
    }
    closedir(dir);
}
int dir_exist(char *name)
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
        result = access(name, F_OK);
        if (result == 0)
        {
            strcpy(project_path, cwd);
            return 1;
        }
        if (strcmp(name, strrchr(cwd, '\\') + 1) == 0)
        {
            strcpy(project_path, cwd);
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
    projects = fopen("C:\\newgit\\Projects.txt", "r");
    fscanf(projects, "%[^\0]", lines);
    fclose(projects);
    char *one_line;
    one_line = strtok(lines, "\n");
    while (one_line != NULL)
    {
        char copy_global_variables[MAX_SIZE];
        char copy_global_user[MAX_SIZE];
        strcpy(copy_global_variables, "copy /y ");
        strcpy(copy_global_variables + strlen(copy_global_variables), "C:\\newgit\\GlobalVariables.txt ");
        strcpy(copy_global_variables + strlen(copy_global_variables), one_line);
        strcpy(copy_global_user, "copy /y ");
        strcpy(copy_global_user + strlen(copy_global_user), "C:\\newgit\\GlobalUser.txt ");
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
        global_user = fopen("C:\\newgit\\GlobalUser.txt", "w");
        fprintf(global_user, "%s\n", argv[4]);
        fclose(global_user);
    }
    else if ((strncmp(argv[3], "user.email", 6) == 0) && argc > 4)
    {
        FILE *global_user;
        global_user = fopen("C:\\newgit\\GlobalUser.txt", "a");
        fprintf(global_user, "%s", argv[4]);
        fclose(global_user);
    }
    else if ((strncmp(argv[3], "alias", 5) == 0) && argc > 4)
    {
        char *alias = malloc(MAX_SIZE);
        alias = strtok(argv[3], ".");
        alias = strtok(NULL, ".");
        FILE *global_variable;
        global_variable = fopen("C:\\newgit\\GlobalVariables.txt", "a");
        fprintf(global_variable, "%s %s\n", argv[4], alias);
        free(alias);
    }
    else
    {
    } // must be completed
}
void create_local_config(int argc, char **argv)
{

    if ((strncmp(argv[2], "user.name", 9) == 0) && argc > 3)
    {
        char address[MAX_SIZE];
        strcpy(address, project_path);
        strcpy(address + strlen(address), "\\ngit\\PersonalUser.txt");
        FILE *local_user;
        local_user = fopen(address, "w");
        fprintf(local_user, "%s\n", argv[3]);
        fclose(local_user);
    }
    else if ((strncmp(argv[2], "user.email", 6) == 0) && argc > 3)
    {
        char *address;
        strcpy(address, project_path);
        strcpy(address + strlen(address), "\\ngit\\PersonalUser.txt");
        FILE *local_user;
        local_user = fopen(address, "a");
        fprintf(local_user, "%s", argv[3]);
        fclose(local_user);
    }
    else if ((strncmp(argv[2], "alias", 5) == 0) && argc > 3)
    {
        char *address;
        strcpy(address, project_path);
        strcpy(address + strlen(address), "\\ngit\\PersonalVariables.txt");
        char *alias = malloc(MAX_SIZE);
        alias = strtok(argv[2], ".");
        alias = strtok(NULL, ".");
        FILE *locak_variables;
        locak_variables = fopen("C:\\newgit\\GlobalVariables.txt", "a");
        fprintf(locak_variables, "%s %s\n", argv[3], alias);
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
    int exist = dir_exist("ngit");
    SetCurrentDirectory(address);
    strcpy(address + strlen(address), "\\ngit");
    if (exist == 1)
        printf("Error : ngit folder is exist in here or it's parents.\n");
    else if (exist == 0)
    {
        FILE *project_path_adding;
        project_path_adding = fopen("C:\\newgit\\Projects.txt", "a");
        fprintf(project_path_adding, "%s\n", address);
        fclose(project_path_adding);
        system("mkdir ngit");
        system("attrib +h ngit");
        system("cd ngit & type nul > PersonalVariables.txt & type nul > PersonalUser.txt & type nul > StageCase.txt & echo master > CommitsAndBranches.txt");
        copy_global_to_all();
    }
}
void add_to_stage(int argc, char **argv)
{
    char address[MAX_SIZE];
    GetCurrentDirectory(sizeof(address), address);
    int is_project = dir_exist("ngit");
    SetCurrentDirectory(address);

    if (is_project)
    {
        int result;
        FILE *stage_area = fopen("ngit\\StageCase.txt", "r");
        char staged_files[MAX_SIZE];
        fscanf(stage_area, "%[^\0]", staged_files);
        fclose(stage_area);
        for (int i = 2; i < argc; i++)
        {
            result = dir_exist(argv[i]);
            if (result == 1)
            {
                char *one_line = strtok(staged_files, "\n");
                int exist = 0;
                while (one_line != NULL)
                {
                    if (strncmp(one_line, argv[i], strlen(argv[i])) == 0)
                    {
                        exist = 1;
                    }
                    one_line = strtok(NULL, "\n");
                }
                if (!exist)
                {
                    FILE *add_to_stagin_area = fopen("ngit\\StageCase.txt", "a");
                    fprintf(add_to_stagin_area, "%s\n", argv[i]);
                    fclose(add_to_stagin_area);
                }
            }
            else
            {
                printf("Error : file not found\n");
            }
        }
    }
    else
    {
        printf("Error : you are not in project folder.\n");
    }
}
void show_status()
{
}
int main(int argc, char **argv)
{
    if ((strncmp(argv[0], "ngit", 4) == 0) && argc > 1)
    {
        project_path = (char *)malloc(MAX_SIZE);
        char address[MAX_SIZE];
        GetCurrentDirectory(sizeof(address), address);
        dir_exist("ngit");
        SetCurrentDirectory(address);
        if ((strncmp(argv[1], "config", 6) == 0) && argc > 2)
        {
            if ((strncmp(argv[2], "-global", 7) == 0) && argc > 3)
            {
                create_global_config(argc, argv);
                copy_global_to_all();
            }
            else if (argc > 3)
            {
                create_local_config(argc, argv);
            }
            else
            {
            } // must be completed
        }
        else if ((strncmp(argv[1], "init", 4) == 0) && argc == 2)
        {
            run_init(argc, argv);
        }
        else if ((strncmp(argv[1], "add", 3) == 0) && argc > 2)
        {
            add_to_stage(argc, argv);
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