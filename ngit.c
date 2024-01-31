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
#include <time.h>
#define debug(x) printf("*%s*\n", x);
#define barresi printf("dorost\n");
#define MAX_SIZE 1000
char *project_path;
char *find_file_path(char *dir, char *file_name)
{
    DIR *dp = opendir(dir);
    if (dp == NULL)
    {
        return NULL;
    }
    char *path = malloc(MAX_PATH);
    if (path == NULL)
    {
        closedir(dp);
        return NULL;
    }
    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }
        sprintf(path, "%s\\%s", dir, entry->d_name);
        struct stat statbuf;
        if (stat(path, &statbuf) == 0)
        {
            if (S_ISREG(statbuf.st_mode))
            {
                if (strcmp(entry->d_name, file_name) == 0)
                {
                    closedir(dp);
                    return path;
                }
            }
            else if (S_ISDIR(statbuf.st_mode))
            {
                char *sub_path = find_file_path(path, file_name);
                if (sub_path != NULL)
                {
                    free(path);
                    closedir(dp);
                    return sub_path;
                }
            }
        }
    }
    free(path);
    closedir(dp);
    return NULL;
}
char *strrev(char *str)
{
    int len = strlen(str);
    char *rev = malloc(len + 1);
    rev[len] = '\0';
    for (int i = 0; i < len; i++)
    {
        rev[i] = str[len - i - 1];
    }
    return rev;
}
int compare_files(char *path1, char *path2)
{
    HANDLE handle1 = CreateFile(path1, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle1 == INVALID_HANDLE_VALUE)
    {
        printf("Error: Unable to open file %s\n", path1);
        return -2;
    }
    HANDLE handle2 = CreateFile(path2, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle2 == INVALID_HANDLE_VALUE)
    {
        printf("Error: Unable to open file %s\n", path2);
        return -2;
    }
    FILETIME last_write_time1, last_write_time2;
    BOOL result1 = GetFileTime(handle1, NULL, NULL, &last_write_time1);
    BOOL result2 = GetFileTime(handle2, NULL, NULL, &last_write_time2);
    if (result1 == FALSE || result2 == FALSE)
    {
        printf("Error: Unable to get file times for %s and %s\n", path1, path2);
        return -2;
    }
    CloseHandle(handle1);
    CloseHandle(handle2);
    // Use CompareFileTime to compare the two FILETIME values
    // The function returns -1 if the first value is less than the second value
    // The function returns 0 if the two values are equal
    // The function returns 1 if the first value is greater than the second value
    int comparison = CompareFileTime(&last_write_time1, &last_write_time2);
    return comparison;
}
int check_folder_of_file_exist_in_now_and_subdirectories(char *path, char *name)
{
    char search_path[MAX_PATH];
    sprintf(search_path, "%s\\*", path);
    WIN32_FIND_DATA data;
    HANDLE handle = FindFirstFile(search_path, &data);
    if (handle == INVALID_HANDLE_VALUE)
    {
        printf("Error: Unable to open directory %s\n", path);
        return -1;
    }
    do
    {
        if (strcmp(data.cFileName, name) == 0)
        {
            FindClose(handle);
            return 1;
        }
        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (strcmp(data.cFileName, ".") != 0 && strcmp(data.cFileName, "..") != 0)
            {
                char sub_path[MAX_PATH];
                sprintf(sub_path, "%s\\%s", path, data.cFileName);
                int result = check_folder_of_file_exist_in_now_and_subdirectories(sub_path, name);
                if (result != 0)
                {
                    FindClose(handle);
                    return result;
                }
            }
        }
    } while (FindNextFile(handle, &data));
    FindClose(handle);
    return 0;
}
void copy_file(const char *src_path, const char *dst_path)
{
    BOOL result = CopyFile(src_path, dst_path, FALSE);
    if (result)
    {
        printf("File copied successfully.\n");
    }
    else
    {
        printf("File copy failed. Error: %u\n", GetLastError());
    }
    char command[MAX_SIZE];
    strcpy(command, "xcopy ");
    strcat(command, src_path);
    strcat(command, " ");
    strcat(command, dst_path);
    system(command);
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
        // must be checked that a alias exist or not
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
        system("cd ngit & type nul > PersonalVariables.txt & type nul > PersonalUser.txt & echo 1 > StageCase.txt & echo master > CommitsAndBranches.txt");
        copy_global_to_all();
    }
}
void add_to_stage(int argc, char **argv)
{
    char address[MAX_SIZE];
    GetCurrentDirectory(sizeof(address), address);
    int is_project = dir_exist("ngit");
    SetCurrentDirectory(address);
    char location[MAX_SIZE];
    strcpy(location, project_path);
    strcat(location, "\\ngit\\StageCase.txt");
    FILE *stagecase = fopen(location, "r");
    char is_commited[MAX_SIZE];
    fscanf(stagecase, "%[^\0]", is_commited);
    fclose(stagecase);
    if (is_project)
    {
        if (strncmp(is_commited, "1", 1) == 0)
        {
            srand(time(NULL));
            int random_number = rand() % 100000 + 1234567;
            stagecase = fopen(location, "w");
            fprintf(stagecase, "%d %d", 0, random_number); // when commited must be 1
            fclose(stagecase);
            char random[MAX_SIZE];
            sprintf(random, "%d", random_number);
            strcpy(location, project_path);
            strcat(location, "\\ngit");
            SetCurrentDirectory(location);
            mkdir(random);
            SetCurrentDirectory(address);
            int result = 0;
            strcat(location, "\\");
            strcat(location, random);
            for (int i = 2; i < argc; i++)
            {
                int is_file = 0;
                if (strchr(argv[i], '.') != NULL)
                {
                    is_file = 1;
                }
                if (is_file)
                {
                    if (access(argv[i], F_OK) == 0)
                    {

                        copy_file(argv[i], location);
                    }
                    else
                    {
                        printf("Error : file not found\n");
                    }
                }
                else
                {
                    if (access(argv[i], F_OK) == 0)
                    {
                        copy_folder(argv[i], location);
                    }
                    else
                    {
                        printf("Error : folder not found\n");
                    }
                }
            }
        }
        else if (strncmp(is_commited, "0", 1) == 0)
        {
            char *reshte = strtok(is_commited, " ");
            reshte = strtok(NULL, " ");
            char random[MAX_SIZE] = "\\";
            sprintf(random + 1, "%s", reshte);
            strcpy(location, project_path);
            strcat(location, "\\ngit\\");
            strcat(location, is_commited + 2);
            int result = 0;
            for (int i = 2; i < argc; i++)
            {
                int is_file = 0;
                if (strchr(argv[i], '.') != NULL)
                {
                    is_file = 1;
                }
                if (is_file)
                {
                    int added_in_previos = check_folder_of_file_exist_in_now_and_subdirectories(location, argv[i]);
                    if (added_in_previos)
                    {
                        char *argv_copy = malloc(MAX_SIZE);
                        strcpy(argv_copy, argv[i]);
                        argv_copy = strrev(argv_copy);
                        char *token = strtok(argv_copy, "\\");
                        if (token == NULL)
                        {
                            strcpy(argv_copy, argv[i]);
                        }
                        else
                        {
                            argv_copy = strrev(argv_copy);
                        }
                        char absolute_file_path[MAX_PATH];
                        DWORD length;
                        LPTSTR fileName;
                        length = GetFullPathName(argv_copy, MAX_PATH, absolute_file_path, &fileName);
                        debug(location);
                        debug(argv_copy);
                        char *file_full_path = find_file_path(location, argv_copy);
                        if (length == 0)
                        {
                            printf("Error: %lu\n", GetLastError());
                        }
                        else
                        {
                            debug(file_full_path);
                            debug(absolute_file_path);
                            int is_change = compare_files(file_full_path, absolute_file_path);
                            if (is_change != 0)
                            {
                                if (access(argv[i], F_OK) == 0)
                                {
                                    copy_file(absolute_file_path, file_full_path);
                                }
                            }
                        }
                        free(argv_copy);
                    }
                    else
                    {
                        if (access(argv[i], F_OK) == 0)
                        {
                            copy_file(argv[i], location);
                        }
                    }
                    // if (access(argv[i], F_OK) == 0)
                    // {
                    //     copy_file(argv[i], location);
                    // }
                    // else
                    // {
                    //     printf("Error : file not found\n");
                    // }
                }
                else
                {
                    if (access(argv[i], F_OK) == 0)
                    {
                        copy_folder(argv[i], location);
                    }
                    else
                    {
                        printf("Error : folder not found\n");
                    }
                }
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
        else if ((strncmp(argv[1], "add", 3) == 0) && argc > 2) // must code -f and -n and function need to be change in i
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
    free(project_path);
    return 0;
}