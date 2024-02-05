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
#include <ctype.h>
#define debug(x) printf("*%s*\n", x);
#define barresi printf("dorost\n");
#define MAX_SIZE 1000
char *project_path;
char *branches_path;
char *commits_path;
char *global_user_path;
char *personal_user_path;
char *global_variables_path;
char *personal_variables_path;
char *local_or_global_path;
char *now_branch_path;
char *now_last_commits_path;
char *stage_case_path;
char *tags_path;
char *commits_folder_path;
char *linked_commits_path;
char *message_shortcuts_path;
int date_to_array(char *date, int arr[6])
{
    int n = sscanf(date, "%d-%d-%d %d:%d:%d", &arr[0], &arr[1], &arr[2], &arr[3], &arr[4], &arr[5]);
    if (n == 6)
    {
        if (arr[0] > 0 && arr[1] >= 1 && arr[1] <= 12 && arr[2] >= 1 && arr[2] <= 31 && arr[3] >= 0 && arr[3] < 24 && arr[4] >= 0 && arr[4] < 60 && arr[5] >= 0 && arr[5] < 60)
        {
            return 1;
        }
    }
    return 0;
}
int date_cmp(int arr1[6], int arr2[6])
{
    for (int i = 0; i < 6; i++)
    {
        if (arr1[i] > arr2[i])
        {
            return 1;
        }
        if (arr1[i] < arr2[i])
        {
            return -1;
        }
    }
    return 0;
}
int date_str_cmp(char *date1, char *date2)
{
    int arr1[6], arr2[6];
    int r1 = date_to_array(date1, arr1);
    int r2 = date_to_array(date2, arr2);
    if (r1 && r2)
    {
        return date_cmp(arr1, arr2);
    }
    return -2;
}
bool is_wildcard(char c)
{
    return c == '?' || c == '*';
}
bool compare_with_wildcard(char *str, char *word)
{
    if (*str == '\0' && *word != '\0')
    {
        return false;
    }
    if (*word == '\0' && *str != '\0')
    {
        return true;
    }
    if (is_wildcard(*word))
    {
        if (*word == '?')
        {
            if (*str == '\0')
            {
                return false;
            }
            return compare_with_wildcard(str + 1, word + 1);
        }
        if (*word == '*')
        {
            if (*str == '\0')
            {
                return compare_with_wildcard(str, word + 1);
            }
            return compare_with_wildcard(str + 1, word + 1);
        }
    }
    if (*word == *str)
    {
        return compare_with_wildcard(str + 1, word + 1);
    }
    return false;
}
int search_word_in_sentence(char *sentence, char *word)
{
    while (*sentence != '\0')
    {
        if (compare_with_wildcard(sentence, word))
        {
            return 1;
        }
        sentence++;
    }
    return 0;
}
int char_to_int(char *str)
{
    int result = 0;
    while (*str != '\0')
    {
        if (*str >= '0' && *str <= '9')
        {
            result = result * 10 + (*str - '0');
        }
        else
        {
            return 0;
        }
        str++;
    }
    return result;
}
void delete_file(char *file_name, char *path)
{
    char buffer[256];
    sprintf(buffer, "%s\\%s", path, file_name);
    char command[MAX_SIZE];
    strcpy(command, "del ");
    strcat(command, buffer);
    system(command);
}
void delete_folder(char *src_path, char *dst_path)
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
        sprintf(src_file, "%s/%s", src_path, entry->d_name);
        if (entry->d_type == DT_DIR)
        {
            delete_folder(src_file, dst_path);
        }
        else
        {
            src_file = strrev(src_file);
            src_file = strtok(src_file, "/");
            src_file = strrev(src_file);
            delete_file(src_file, dst_path);
        }
        free(src_file);
    }
    closedir(dir);
}
void copy_file_to_hidden_folder(char *path1, char *path2)
{
    FILE *source, *target;
    char buffer[4096];
    int n;
    source = fopen(path1, "rb");
    if (source == NULL)
    {
        printf("Error: cannot open source file %s\n", path1);
        exit(1);
    }
    char *new_file = malloc(strlen(path2) + strlen(path1) + 2);
    strcpy(new_file, path2);
    strcat(new_file, "\\");
    char *file_name = malloc(MAX_SIZE);
    file_name = strrev(path1);
    file_name = strtok(file_name, "/");
    file_name = strrev(file_name);
    if (file_name == NULL)
    {
        strcat(new_file, path1);
    }
    else
    {
        strcat(new_file, file_name);
    }
    target = fopen(new_file, "wb");
    if (target == NULL)
    {
        printf("Error: cannot create target file %s\n", new_file);
        fclose(source);
        free(new_file);
        exit(1);
    }
    while ((n = fread(buffer, 1, 4096, source)) > 0)
    {
        fwrite(buffer, 1, n, target);
    }
    free(file_name);
    fclose(source);
    fclose(target);
    free(new_file);
}
int count_files(char *folder_path)
{
    int count = 0;
    char search_path[MAX_PATH];
    sprintf(search_path, "%s\\*.*", folder_path);
    WIN32_FIND_DATA fd;
    HANDLE h = FindFirstFile(search_path, &fd);
    if (h == INVALID_HANDLE_VALUE)
    {
        perror("FindFirstFile");
        return -1;
    }
    do
    {
        if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0)
        {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                char subfolder_path[MAX_PATH];
                sprintf(subfolder_path, "%s\\%s", folder_path, fd.cFileName);
                int subcount = count_files(subfolder_path);
                if (subcount == -1)
                {
                    FindClose(h);
                    return -1;
                }
                count += subcount;
            }
            else
            {
                count++;
            }
        }
    } while (FindNextFile(h, &fd));
    FindClose(h);
    return count;
}
int is_folder_empty(char *folder_path)
{
    DIR *dir = opendir(folder_path);
    if (dir == NULL)
    {
        perror("opendir");
        return -1;
    }
    struct dirent *entry;
    int count = 0;
    while ((entry = readdir(dir)) != NULL)
    {
        if (++count > 2)
            break;
    }
    closedir(dir);
    if (count <= 2)
        return 1;
    else
        return 0;
}
// void delete_empty_subdirs(char *path)
// {
//     char buffer[MAX_PATH];
//     DWORD length;
//     length = GetFullPathName(path, MAX_PATH, buffer, NULL);
//     if (length > 0 && length < MAX_PATH)
//     {
//         strcat(buffer, "\\*");
//         WIN32_FIND_DATA file_data;
//         HANDLE file_handle = FindFirstFile(buffer, &file_data);
//         if (file_handle != INVALID_HANDLE_VALUE)
//         {
//             do
//             {
//                 if (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
//                     strcmp(file_data.cFileName, ".") != 0 &&
//                     strcmp(file_data.cFileName, "..") != 0)
//                 {
//                     char temp[MAX_PATH];
//                     strcpy(temp, buffer);
//                     temp[strlen(temp) - 1] = '\0';
//                     strcat(temp, file_data.cFileName);
//                     delete_empty_subdirs(temp);
//                 }
//             } while (FindNextFile(file_handle, &file_data)); // Get the next subdirectory
//             FindClose(file_handle);
//         }
//         buffer[strlen(buffer) - 2] = '\0';
//         if (!RemoveDirectory(buffer))
//         {
//             DWORD error = GetLastError();
//             if (error != ERROR_DIR_NOT_EMPTY)
//             {
//                 printf("Error: %lu\n", error);
//             }
//         }
//     }
//     else
//     {
//         printf("Error: %lu\n", GetLastError());
//     }
// }
int compare_files(char *path1, char *path2)
{
    FILE *f1, *f2;
    char buffer1[1024], buffer2[1024];
    size_t n1, n2;
    int result = 0;
    f1 = fopen(path1, "rb");
    f2 = fopen(path2, "rb");
    if (f1 == NULL || f2 == NULL)
    {
        printf("Error: Cannot open file %s or %s\n", path1, path2);
        return -1;
    }
    while (1)
    {
        n1 = fread(buffer1, 1, sizeof(buffer1), f1);
        n2 = fread(buffer2, 1, sizeof(buffer2), f2);
        if (n1 != n2)
        {
            result = -1;
            break;
        }
        if (n1 == 0)
        {
            result = 0;
            break;
        }
        if (memcmp(buffer1, buffer2, n1) != 0)
        {
            result = -1;
            break;
        }
    }
    fclose(f1);
    fclose(f2);
    return result;
}
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
int check_folder_of_file_exist_in_now_and_subdirectories(char *path, char *name) // may be it must be deleted
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
void copy_folder(char *src_path, char *dst_path)
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
        sprintf(src_file, "%s/%s", src_path, entry->d_name);
        if (entry->d_type == DT_DIR)
        {
            copy_folder(src_file, dst_path);
        }
        else
        {
            copy_file_to_hidden_folder(src_file, dst_path);
        }
        free(src_file);
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
void copy_global_to_all() // must be fixed to copy variables
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
        char *set_global = malloc(MAX_SIZE);
        strcpy(set_global, one_line);
        strcat(set_global, "\\LocalOrGlobal.txt");
        FILE *log = fopen(set_global, "w");
        fprintf(log, "%d", 1);
        fclose(log);
        one_line = strtok(NULL, "\n");
        free(set_global);
    }
}
void create_global_config(int argc, char **argv)
{
    if ((strncmp(argv[3], "user.name", 9) == 0) && argc > 4)
    {
        FILE *global_user;
        global_user = fopen("C:\\newgit\\GlobalUser.txt", "w");
        fprintf(global_user, "name:%s\n", argv[4]);
        fclose(global_user);
    }
    else if ((strncmp(argv[3], "user.email", 6) == 0) && argc > 4)
    {
        FILE *global_user;
        global_user = fopen("C:\\newgit\\GlobalUser.txt", "a");
        fprintf(global_user, "email:%s", argv[4]);
        fclose(global_user);
    }
    else if ((strncmp(argv[3], "alias", 5) == 0) && argc > 4)
    {
        char *alias = malloc(MAX_SIZE);
        alias = strtok(argv[3], ".");
        alias = strtok(NULL, ".");
        FILE *global_variable;
        global_variable = fopen("C:\\newgit\\GlobalVariables.txt", "a");
        fprintf(global_variable, "*%s*%s*\n", alias, argv[4]);
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
        FILE *local_user;
        local_user = fopen(personal_user_path, "w");
        fprintf(local_user, "name:%s\n", argv[3]);
        fclose(local_user);
        FILE *log = fopen(local_or_global_path, "w");
        fprintf(log, "%d", 0);
        fclose(log);
    }
    else if ((strncmp(argv[2], "user.email", 6) == 0) && argc > 3)
    {
        FILE *local_user;
        local_user = fopen(personal_user_path, "a");
        fprintf(local_user, "email:%s", argv[3]);
        fclose(local_user);
        FILE *log = fopen(local_or_global_path, "w");
        fprintf(log, "%d", 0);
        fclose(log);
    }
    else if ((strncmp(argv[2], "alias", 5) == 0) && argc > 3)
    {
        char *alias = malloc(MAX_SIZE);
        alias = strtok(argv[2], ".");
        alias = strtok(NULL, ".");
        FILE *local_variables;
        local_variables = fopen(personal_variables_path, "a");
        // must be checked command is valid
        fprintf(local_variables, "*%s*%s*\n", alias, argv[3]);
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
        system("cd ngit & type nul > PersonalVariables.txt & type nul > PersonalUser.txt & echo 1 > StageCase.txt & echo master > Branches.txt & type nul > Commits.txt & mkdir commits & type nul > NowLastCommits.txt & echo master > NowBranch.txt & type nul > LocalOrGlobal.txt & type nul > Tags.txt & echo No > LinkedCommits.txt & type nul > MessageShortcuts.txt");
        copy_global_to_all();
    }
}
void add_to_stage(int argc, char **argv)
{
    char address[MAX_SIZE];
    GetCurrentDirectory(sizeof(address), address);
    int is_project = dir_exist("ngit");
    SetCurrentDirectory(address);
    FILE *stagecase = fopen(stage_case_path, "r");
    char is_commited[MAX_SIZE];
    fscanf(stagecase, "%[^\0]", is_commited);
    fclose(stagecase);
    if (is_project)
    {
        if (strncmp(is_commited, "1", 1) == 0)
        {
            srand(time(NULL));
            int random_number = rand() % 100000 + 1234567;
            stagecase = fopen(stage_case_path, "w");
            fprintf(stagecase, "%d %d", 0, random_number); // when commited must be 1
            fclose(stagecase);
            char random[MAX_SIZE];
            sprintf(random, "%d", random_number);
            char location[MAX_SIZE];
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
                        // CopyFileToHiddenFolder(complete_path, location);
                        copy_file_to_hidden_folder(argv[i], location);
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
            char location[MAX_SIZE];
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
                        char *file_full_path = find_file_path(location, argv_copy);
                        if (length == 0)
                        {
                            printf("Error: %lu\n", GetLastError());
                        }
                        else
                        {
                            int is_change = compare_files(file_full_path, absolute_file_path);
                            if (is_change != 0)
                            {
                                if (access(argv[i], F_OK) == 0)
                                {
                                    // CopyFileToHiddenFolder(absolute_file_path, file_full_path);
                                    copy_file_to_hidden_folder(argv[i], location);
                                }
                            }
                        }
                        free(argv_copy);
                    }
                    else
                    {
                        if (access(argv[i], F_OK) == 0)
                        {
                            // CopyFileToHiddenFolder(argv[i], location);
                            copy_file_to_hidden_folder(argv[i], location);
                        }
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
    }
    else
    {
        printf("Error : you are not in project folder.\n");
    }
}
void reset_staging(int argc, char **argv)
{
    char *stage_case_address = malloc(MAX_SIZE);
    strcpy(stage_case_address, project_path);
    strcat(stage_case_address, "\\");
    strcat(stage_case_address, "ngit");
    char *dir_address = malloc(MAX_SIZE);
    strcpy(dir_address, stage_case_address);
    strcat(stage_case_address, "\\StageCase.txt");
    FILE *stage = fopen(stage_case_address, "r");
    char *line = malloc(MAX_SIZE);
    fscanf(stage, "%[^\0]", line);
    fclose(stage);
    char *random_number = malloc(MAX_SIZE);
    random_number = strtok(line, " ");
    random_number = strtok(NULL, " ");
    strcat(dir_address, "\\");
    strcat(dir_address, random_number);
    for (int i = 2; i < argc; i++) // must be edited
    {
        if (strchr(argv[i], '.') != NULL)
        {
            char *address_reverse = malloc(MAX_SIZE);
            address_reverse = strrev(argv[i]);
            address_reverse = strtok(address_reverse, "\\");
            address_reverse = strrev(address_reverse);
            if (address_reverse == NULL)
            {
                delete_file(argv[i], dir_address);
            }
            else
            {

                delete_file(address_reverse, dir_address);
            }
            free(address_reverse);
        }
        else
        {
            char *path = malloc(MAX_SIZE);
            strcpy(path, project_path);
            strcat(path, "\\ngit\\");
            strcat(path, random_number);
            delete_folder(argv[i], path);
            free(path);
        }
        free(line);
        free(random_number);
        free(stage_case_address);
        free(dir_address);
    }
}
void run_commit(char *message)
{
    FILE *open_stage = fopen(stage_case_path, "r");
    char *line = malloc(MAX_SIZE);
    fscanf(open_stage, "%[^\0]", line);
    fclose(open_stage);
    char *random_number = malloc(MAX_SIZE);
    random_number = strtok(line, " ");
    random_number = strtok(NULL, " ");
    if (random_number == NULL)
    {
        printf("Error : you dont have add any things");
        return;
    }
    char *stage_commit_address = malloc(MAX_SIZE);
    strcpy(stage_commit_address, project_path);
    strcat(stage_commit_address, "\\ngit\\");
    strcat(stage_commit_address, random_number);
    int is_empty = is_folder_empty(stage_commit_address);
    if (is_empty)
    {
        printf("Error: you have not add any things");
    }
    else
    {
        char *commit_places = malloc(MAX_SIZE);
        strcpy(commit_places, project_path);
        strcat(commit_places, "\\ngit\\commits\\");
        strcat(commit_places, random_number);
        char *command = malloc(MAX_SIZE);
        strcpy(command, "xcopy ");
        strcat(command, stage_commit_address);
        strcat(command, " ");
        strcat(command, commit_places);
        strcat(command, " /i /s /e /h");
        system(command);
        free(command);
        FILE *write_stage = fopen(stage_case_path, "w");
        fprintf(write_stage, "%d", 1);
        fclose(open_stage);
        FILE *branch_file = fopen(now_branch_path, "r");
        char branch_in_now[MAX_SIZE]; // must be editied in checkout and branch making
        fscanf(branch_file, "%[^\0]", branch_in_now);
        fclose(branch_file);
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        char time_and_hour[64];
        strftime(time_and_hour, sizeof(time_and_hour), "%Y-%m-%d %H:%M:%S", tm);
        FILE *loq_file = fopen(local_or_global_path, "r");
        char loc_or_glob[10];
        fscanf(loq_file, "%[^\0]", loc_or_glob);
        fclose(loq_file);
        char user_name_and_user_email[MAX_SIZE];
        if (strncmp(loc_or_glob, "1", 1) == 0)
        {
            FILE *loq_file = fopen(global_user_path, "r");
            fscanf(loq_file, "%[^\0]", user_name_and_user_email);
            fclose(loq_file);
        }
        else if (strncmp(loc_or_glob, "0", 1) == 0)
        {
            FILE *loq_file = fopen(personal_user_path, "r");
            fscanf(loq_file, "%[^\0]", user_name_and_user_email);
            fclose(loq_file);
        }
        int number_of_files = count_files(stage_commit_address);
        char info[10000];
        FILE *get_previos_commits = fopen(commits_path, "r");
        fscanf(get_previos_commits, "%[^\0]", info);
        fclose(get_previos_commits);
        FILE *put_commit_information = fopen(commits_path, "w");
        fprintf(put_commit_information, "commit_id:%s\n%s\ntime:%s\nbranch:%snumber_of_files_commited:%d\nmessage:%s\n**********\n", random_number, user_name_and_user_email, time_and_hour, branch_in_now, number_of_files, message);
        fclose(put_commit_information);
        FILE *put_previos_commits = fopen(commits_path, "a");
        fprintf(put_previos_commits, "%s", info);
        fclose(put_previos_commits);
        char matn[MAX_SIZE];
        FILE *link = fopen(linked_commits_path, "r");
        fscanf(link, "%[^\0]", matn);
        fclose(link);
        if (strncmp(matn, "No", 2) == 0)
        {
            FILE *link = fopen(linked_commits_path, "w");
            fprintf(link, "#%s*%s*%s#", random_number, "NULL", branch_in_now);
            fclose(link);
        }
        else
        {
            char *line = malloc(MAX_SIZE);
            line = strrev(matn);
            line = strtok(line, "#");
            line = strrev(line);
            line = strtok(line, "*");
            FILE *link = fopen(linked_commits_path, "a");
            fprintf(link, "%s*%s*%s#", random_number, line, branch_in_now);
            fclose(link);
            free(line);
        }
        char del[MAX_SIZE];
        strcpy(del, "rmdir /s /q ");
        strcat(del, stage_commit_address);
        system(del);
        free(commit_places);
        free(command);
    }
    free(line);
    free(random_number);
    free(stage_commit_address);
}
void run_all_log()
{
    FILE *logs = fopen(commits_path, "r");
    char *reshte = malloc(10000);
    fscanf(logs, "%[^\0]", reshte);
    fclose(logs);
    char *one = malloc(MAX_SIZE);
    one = strtok(reshte, "**********");
    while (one != NULL)
    {
        printf("%s\n", one);
        one = strtok(NULL, "**********");
    }
    free(one);
    free(reshte);
}
void run_n_log(int n)
{
    FILE *logs = fopen(commits_path, "r");
    char *reshte = malloc(10000);
    fscanf(logs, "%[^\0]", reshte);
    fclose(logs);
    char *one = malloc(MAX_SIZE);
    one = strtok(reshte, "**********");
    int count = 0;
    while (one != NULL, count < n)
    {
        count++;
        printf("%s\n", one);
        one = strtok(NULL, "**********");
    }
    free(one);
    free(reshte);
}
void run_branch_log(char *branch)
{
    FILE *logs = fopen(commits_path, "r");
    char *reshte = malloc(10000);
    fscanf(logs, "%[^\0]", reshte);
    fclose(logs);
    char *one = malloc(MAX_SIZE);
    one = strtok(reshte, "**********");
    char *copy = malloc(MAX_SIZE);
    strcpy(copy, one);
    while (one != NULL)
    {
        char *branch_line = malloc(MAX_SIZE);
        branch_line = strstr(one, "branch:");
        strcpy(branch_line, branch_line + 7);
        if (strncmp(branch_line, branch, strlen(branch)) == 0)
        {
            printf("%s\n", copy);
        }
        one = strtok(NULL, "**********");
        strcpy(copy, one);
        free(branch_line);
    }
    free(copy);
    free(one);
    free(reshte);
}
void run_author_log(char *author)
{
    FILE *logs = fopen(commits_path, "r");
    char *reshte = malloc(10000);
    fscanf(logs, "%[^\0]", reshte);
    fclose(logs);
    char *one = malloc(MAX_SIZE);
    one = strtok(reshte, "**********");
    char *copy = malloc(MAX_SIZE);
    strcpy(copy, one);
    while (one != NULL)
    {
        char *name_line = malloc(MAX_SIZE);
        name_line = strstr(one, "name:");
        strcpy(name_line, name_line + 5);
        if (strncmp(name_line, author, strlen(author)) == 0)
        {
            printf("%s\n", copy);
        }
        one = strtok(NULL, "**********");
        strcpy(copy, one);
        free(name_line);
    }
    free(copy);
    free(one);
    free(reshte);
}
void run_since_log(char *time)
{
    FILE *logs = fopen(commits_path, "r");
    char *reshte = malloc(10000);
    fscanf(logs, "%[^\0]", reshte);
    fclose(logs);
    char *one = malloc(MAX_SIZE);
    one = strtok(reshte, "**********");
    char *copy = malloc(MAX_SIZE);
    strcpy(copy, one);
    while (one != NULL)
    {
        char *date_line = malloc(MAX_SIZE);
        date_line = strstr(one, "time:");
        strcpy(date_line, date_line + 5);
        date_line[19] = '\0';
        int compare_times = date_str_cmp(date_line, time);
        if (compare_times >= 0)
        {
            printf("%s\n", copy);
        }
        one = strtok(NULL, "**********");
        strcpy(copy, one);
        free(date_line);
    }
    free(copy);
    free(one);
    free(reshte);
}
void run_before_log(char *time)
{
    FILE *logs = fopen(commits_path, "r");
    char *reshte = malloc(10000);
    fscanf(logs, "%[^\0]", reshte);
    fclose(logs);
    char *one = malloc(MAX_SIZE);
    one = strtok(reshte, "**********");
    char *copy = malloc(MAX_SIZE);
    strcpy(copy, one);
    while (one != NULL)
    {
        char *date_line = malloc(MAX_SIZE);
        date_line = strstr(one, "time:");
        strcpy(date_line, date_line + 5);
        date_line[19] = '\0';
        int compare_times = date_str_cmp(date_line, time);
        if (compare_times <= 0)
        {
            printf("%s\n", copy);
        }
        one = strtok(NULL, "**********");
        strcpy(copy, one);
        free(date_line);
    }
    free(copy);
    free(one);
    free(reshte);
}
void run_word_log(char *word)
{
    FILE *logs = fopen(commits_path, "r");
    char *reshte = malloc(10000);
    fscanf(logs, "%[^\0]", reshte);
    fclose(logs);
    char *one = malloc(MAX_SIZE);
    one = strtok(reshte, "**********");
    char *copy = malloc(MAX_SIZE);
    strcpy(copy, one);
    while (one != NULL)
    {
        char *message_line = malloc(MAX_SIZE);
        message_line = strstr(one, "message:");
        int word_exist = search_word_in_sentence(message_line, word);
        if (word_exist == 1)
        {
            printf("%s\n", copy);
        }
        one = strtok(NULL, "**********");
        strcpy(copy, one);
        free(message_line);
    }
    free(copy);
    free(one);
    free(reshte);
}
void run_many_word_log(int argc, char **argv) // must be completed, points
{
    char *reshte = malloc(10000);
    char *message_line = malloc(MAX_SIZE);
    char *copy = malloc(MAX_SIZE);
    char *one = malloc(MAX_SIZE);
    for (int i = 4; i < argc; i++)
    {
        FILE *logs = fopen(commits_path, "r");
        fscanf(logs, "%[^\0]", reshte);
        fclose(logs);
        one = strtok(reshte, "**********");
        strcpy(copy, one);
        while (one != NULL)
        {
            message_line = strstr(one, "message:");
            int word_exist = search_word_in_sentence(message_line, argv[i]);
            if (word_exist == 1)
            {
                printf("%s\n", copy);
            }
            one = strtok(NULL, "**********");
            strcpy(copy, one);
        }
    }
    free(message_line);
    free(copy);
    free(one);
    free(reshte);
}
void show_branches()
{
    FILE *branches = fopen(branches_path, "r");
    char reshte[MAX_SIZE];
    fscanf(branches, "%[^\0]", reshte);
    fclose(branches);
    printf(reshte);
}
void make_branches(char *branch_name)
{
    char *names = malloc(MAX_SIZE);
    FILE *branches_reading = fopen(branches_path, "r");
    fscanf(branches_reading, "%[^\0]", names);
    fclose(branches_reading);
    char *line = malloc(MAX_SIZE);
    line = strtok(names, "\n");
    while (line != NULL)
    {
        if (strncmp(line, branch_name, strlen(branch_name)) == 0)
        {
            printf("Error: branch exist");
            return;
        }
        line = strtok(NULL, "\n");
    }
    FILE *branches = fopen(branches_path, "a");
    fprintf(branches, "%s\n", branch_name);
    fclose(branches);
    FILE *now_branch = fopen(now_branch_path, "w");
    fprintf(now_branch, "%s", branch_name);
    fclose(now_branch);
    free(line);
    free(names);
}
int main(int argc, char **argv)
{
    if ((strncmp(argv[0], "ngit", 4) == 0) && argc > 1)
    {
        project_path = (char *)malloc(MAX_SIZE);
        branches_path = (char *)malloc(MAX_SIZE);
        commits_path = (char *)malloc(MAX_SIZE);
        global_user_path = (char *)malloc(MAX_SIZE);
        personal_user_path = (char *)malloc(MAX_SIZE);
        global_variables_path = (char *)malloc(MAX_SIZE);
        personal_variables_path = (char *)malloc(MAX_SIZE);
        local_or_global_path = (char *)malloc(MAX_SIZE);
        now_branch_path = (char *)malloc(MAX_SIZE);
        now_last_commits_path = (char *)malloc(MAX_SIZE);
        stage_case_path = (char *)malloc(MAX_SIZE);
        tags_path = (char *)malloc(MAX_SIZE);
        commits_folder_path = (char *)malloc(MAX_SIZE);
        message_shortcuts_path = (char *)malloc(MAX_SIZE);
        linked_commits_path = (char *)malloc(MAX_SIZE);
        char address[MAX_SIZE];
        GetCurrentDirectory(sizeof(address), address);
        int check = dir_exist("ngit");
        SetCurrentDirectory(address);
        strcpy(branches_path, project_path);
        strcpy(commits_path, project_path);
        strcpy(global_user_path, project_path);
        strcpy(personal_user_path, project_path);
        strcpy(global_variables_path, project_path);
        strcpy(personal_variables_path, project_path);
        strcpy(local_or_global_path, project_path);
        strcpy(now_last_commits_path, project_path);
        strcpy(stage_case_path, project_path);
        strcpy(commits_folder_path, project_path);
        strcpy(tags_path, project_path);
        strcpy(now_branch_path, project_path);
        strcpy(message_shortcuts_path, project_path);
        strcpy(linked_commits_path, project_path);
        strcat(branches_path, "\\ngit");
        strcat(commits_path, "\\ngit");
        strcat(global_user_path, "\\ngit");
        strcat(personal_user_path, "\\ngit");
        strcat(global_variables_path, "\\ngit");
        strcat(personal_variables_path, "\\ngit");
        strcat(local_or_global_path, "\\ngit");
        strcat(now_last_commits_path, "\\ngit");
        strcat(stage_case_path, "\\ngit");
        strcat(tags_path, "\\ngit");
        strcat(now_branch_path, "\\ngit");
        strcat(commits_folder_path, "\\ngit");
        strcat(message_shortcuts_path, "\\ngit");
        strcat(linked_commits_path, "\\ngit");
        strcat(branches_path, "\\Branches.txt");
        strcat(commits_path, "\\Commits.txt");
        strcat(global_user_path, "\\GlobalUser.txt");
        strcat(personal_user_path, "\\PersonalUser.txt");
        strcat(global_variables_path, "\\GlobalVariables.txt");
        strcat(personal_variables_path, "\\PersonalVariables.txt");
        strcat(local_or_global_path, "\\LocalOrGlobal.txt");
        strcat(now_last_commits_path, "\\NowLastCommits.txt");
        strcat(stage_case_path, "\\StageCase.txt");
        strcat(tags_path, "\\Tags.txt");
        strcat(now_branch_path, "\\NowBranch.txt");
        strcat(commits_folder_path, "\\commits");
        strcat(message_shortcuts_path, "\\MessageShortcuts.txt");
        strcat(linked_commits_path, "\\LinkedCommits.txt");
        if (check != 1)
        {
            printf("Error : you are not in project\n");
            return 0;
        }
        if ((strncmp(argv[1], "config", 6) == 0) && argc > 2)
        {
            if ((strncmp(argv[2], "-global", 7) == 0) && argc > 3)
            {
                create_global_config(argc, argv);
                copy_global_to_all();
            }
            else if (argc > 3) // must be checked
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
        else if ((strncmp(argv[1], "reset", 5) == 0) && argc > 2) // must code -f and -n and function need to be change in i
        {
            reset_staging(argc, argv);
        }
        else if ((strncmp(argv[1], "commit", 6) == 0) && argc == 4)
        {
            if ((strncmp(argv[2], "-m", 2) == 0))
            {
                if (strlen(argv[3]) <= 74)
                {
                    run_commit(argv[3]);
                }
                else
                { // must be completed
                }
            }
            else
            { // must be completed
            }
        }
        else if ((strncmp(argv[1], "set", 3) == 0) && argc > 2) // must be changed and completed
        {
        }
        else if ((strncmp(argv[1], "replace", 3) == 0) && argc > 2) // must be changed and completed
        {
        }
        else if ((strncmp(argv[1], "remove", 3) == 0) && argc > 2) // must be changed and completed
        {
        }
        else if ((strncmp(argv[1], "log", 3) == 0) && argc >= 2)
        {
            if (argc == 2)
            {
                run_all_log();
            }
            if (argc == 4 && (strncmp(argv[2], "-n", 2) == 0))
            {
                int number = char_to_int(argv[3]);
                run_n_log(number);
            }
            if (argc == 4 && (strncmp(argv[2], "-branch", 7) == 0))
            {
                run_branch_log(argv[3]);
            }
            if (argc == 4 && (strncmp(argv[2], "-author", 7) == 0))
            {
                run_author_log(argv[3]);
            }
            if (argc == 4 && (strncmp(argv[2], "-since", 7) == 0))
            {
                run_since_log(argv[3]);
            }
            if (argc == 4 && (strncmp(argv[2], "-before", 7) == 0))
            {
                run_before_log(argv[3]);
            }
            if (argc == 4 && (strncmp(argv[2], "-search", 7) == 0))
            {
                run_word_log(argv[3]);
            }
            if (argc >= 5 && (strncmp(argv[2], "-search", 7) == 0) && (strncmp(argv[3], "-f", 2) == 0))
            {
                run_many_word_log(argc, argv);
            }
            // must be add serach word
        }
        else if ((strncmp(argv[1], "branch", 6) == 0) && argc == 2)
        {
            show_branches();
        }
        else if ((strncmp(argv[1], "branch", 6) == 0) && argc == 3)
        {
            make_branches(argv[2]);
        }
        else if (argc == 2)
        {
            FILE *open_global_variables = fopen(global_variables_path, "r");
            char *lines_global_variables = malloc(MAX_SIZE);
            fscanf(open_global_variables, "%[^\0]", lines_global_variables);
            fclose(open_global_variables);
            char *line_by_line = malloc(MAX_SIZE);
            line_by_line = strtok(lines_global_variables, "\n");
            char *alias_name = malloc(MAX_SIZE);
            int exist = 0;
            while (line_by_line != NULL)
            {
                alias_name = strtok(line_by_line, "*");
                if (strncmp(argv[1], alias_name, strlen(argv[1])) == 0)
                {
                    alias_name = strtok(NULL, "*");
                    system(alias_name);
                    exist = 1;
                    break;
                }
                line_by_line = strtok(NULL, "\n");
            }
            FILE *open_local_variables = fopen(personal_variables_path, "r");
            char *lines_local_variables = malloc(MAX_SIZE);
            fscanf(open_local_variables, "%[^\0]", lines_local_variables);
            fclose(open_local_variables);
            line_by_line = strtok(lines_local_variables, "\n");
            while (line_by_line != NULL)
            {
                alias_name = strtok(line_by_line, "*");
                if (strncmp(argv[1], alias_name, strlen(argv[1])) == 0)
                {
                    alias_name = strtok(NULL, "*");
                    system(alias_name);
                    exist = 1;
                    break;
                }
                line_by_line = strtok(NULL, "\n");
            }
            if (!exist)
            {
                printf("Error : command not found. please put \"ngit\" in first of your command and type the rest of command.\n");
            }
            free(alias_name);
            free(line_by_line);
            free(lines_global_variables);
            free(lines_local_variables);
        }
    }
    else
    {
        printf("Error : command not found. please put \"ngit\" in first of your command and type the rest of command.\n");
    }
    free(project_path);
    free(branches_path);
    free(commits_path);
    free(global_user_path);
    free(personal_user_path);
    free(global_variables_path);
    free(personal_variables_path);
    free(local_or_global_path);
    free(now_branch_path);
    free(now_last_commits_path);
    free(stage_case_path);
    free(tags_path);
    free(commits_folder_path);
    free(message_shortcuts_path);
    free(linked_commits_path);
    return 0;
}