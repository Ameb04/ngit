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
void delete_empty_subdirs(char *path)
{
    char buffer[MAX_PATH];
    DWORD length;
    length = GetFullPathName(path, MAX_PATH, buffer, NULL);
    if (length > 0 && length < MAX_PATH)
    {
        strcat(buffer, "\\*");
        WIN32_FIND_DATA file_data;
        HANDLE file_handle = FindFirstFile(buffer, &file_data);
        if (file_handle != INVALID_HANDLE_VALUE)
        {
            do
            {
                if (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
                    strcmp(file_data.cFileName, ".") != 0 &&
                    strcmp(file_data.cFileName, "..") != 0)
                {
                    char temp[MAX_PATH];
                    strcpy(temp, buffer);
                    temp[strlen(temp) - 1] = '\0';
                    strcat(temp, file_data.cFileName);
                    delete_empty_subdirs(temp);
                }
            } while (FindNextFile(file_handle, &file_data)); // Get the next subdirectory
            FindClose(file_handle);
        }
        buffer[strlen(buffer) - 2] = '\0';
        if (!RemoveDirectory(buffer))
        {
            DWORD error = GetLastError();
            if (error != ERROR_DIR_NOT_EMPTY)
            {
                printf("Error: %lu\n", error);
            }
        }
    }
    else
    {
        printf("Error: %lu\n", GetLastError());
    }
}
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
int delete_file(char *dir, char *filename)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    char path[1024];
    int result = 0;
    if ((dp = opendir(dir)) == NULL)
    {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return 0;
    }
    while ((entry = readdir(dp)) != NULL)
    {
        if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
        {
            continue;
        }
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);
        if (stat(path, &statbuf) == -1)
        {
            fprintf(stderr, "Cannot get file information: %s\n", path);
            continue;
        }
        if (S_ISDIR(statbuf.st_mode))
        {
            result = delete_file(path, filename);
            if (result == 1)
            {
                break;
            }
        }
        else
        {
            if (strcmp(filename, entry->d_name) == 0)
            {
                if (remove(path) == 0)
                {
                    printf("File deleted: %s\n", path);
                    result = 1;
                    break;
                }
                else
                {
                    fprintf(stderr, "Cannot delete file: %s\n", path);
                }
            }
        }
    }
    closedir(dp);
    return result;
}
void delete_folder(char *path1, char *path2)
{
    char buffer1[MAX_PATH];
    DWORD length1;
    length1 = GetFullPathName(path1, MAX_PATH, buffer1, NULL);
    if (length1 > 0 && length1 < MAX_PATH)
    {
        strcat(buffer1, "\\*");
        WIN32_FIND_DATA file_data;
        HANDLE file_handle = FindFirstFile(buffer1, &file_data);
        if (file_handle != INVALID_HANDLE_VALUE)
        {
            do
            {
                if (strcmp(file_data.cFileName, ".") != 0 && strcmp(file_data.cFileName, "..") != 0)
                {
                    char temp1[MAX_PATH];
                    strcpy(temp1, buffer1);
                    temp1[strlen(temp1) - 1] = '\0';
                    strcat(temp1, file_data.cFileName);
                    char buffer2[MAX_PATH];
                    DWORD length2;
                    length2 = GetFullPathName(path2, MAX_PATH, buffer2, NULL);
                    if (length2 > 0 && length2 < MAX_PATH)
                    {
                        strcat(buffer2, "\\");
                        strcat(buffer2, file_data.cFileName);
                        if (GetFileAttributes(buffer2) != INVALID_FILE_ATTRIBUTES)
                        {
                            if (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                            {
                                delete_folder(temp1, buffer2);
                            }
                            else
                            {
                                DeleteFile(buffer2);
                            }
                        }
                    }
                    else
                    {
                        printf("Error: %lu\n", GetLastError());
                    }
                }
            } while (FindNextFile(file_handle, &file_data));
            FindClose(file_handle);
        }
    }
    else
    {
        printf("Error: %lu\n", GetLastError());
    }
}
BOOL CopyFileToHiddenFolder(const char *src_path, const char *dst_path)
{
    SHFILEOPSTRUCT shfo = {0};
    shfo.pFrom = src_path;
    shfo.pTo = dst_path;
    shfo.wFunc = FO_COPY;
    shfo.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
    int result = SHFileOperation(&shfo);
    if (result == 0)
    {
        printf("File copied successfully.\n");
        return TRUE;
    }
    else
    {
        printf("File copy failed. Error: %d\n", result);
        return FALSE;
    }
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
            int added_in_previos = check_folder_of_file_exist_in_now_and_subdirectories(dst_path, src_path);
            if (added_in_previos)
            {
                char *argv_copy = malloc(MAX_SIZE);
                strcpy(argv_copy, src_path);
                argv_copy = strrev(argv_copy);
                char *token = strtok(argv_copy, "\\");
                if (token == NULL)
                {
                    strcpy(argv_copy, src_path);
                }
                else
                {
                    argv_copy = strrev(argv_copy);
                }
                char absolute_file_path[MAX_PATH];
                DWORD length;
                LPTSTR fileName;
                length = GetFullPathName(argv_copy, MAX_PATH, absolute_file_path, &fileName);
                char *file_full_path = find_file_path(dst_path, argv_copy);
                if (length == 0)
                {
                    printf("Error: %lu\n", GetLastError());
                }
                else
                {
                    int is_change = compare_files(file_full_path, absolute_file_path);
                    if (is_change != 0)
                    {
                        if (access(src_path, F_OK) == 0)
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
                    }
                }
                free(argv_copy);
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
        fprintf(local_user, "name:%s\n", argv[3]);
        fclose(local_user);
    }
    else if ((strncmp(argv[2], "user.email", 6) == 0) && argc > 3)
    {
        char *address;
        strcpy(address, project_path);
        strcpy(address + strlen(address), "\\ngit\\PersonalUser.txt");
        FILE *local_user;
        local_user = fopen(address, "a");
        fprintf(local_user, "email:%s", argv[3]);
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
        system("cd ngit & type nul > PersonalVariables.txt & type nul > PersonalUser.txt & echo 1 > StageCase.txt & echo master*null > Branches.txt & type nul > Commits.txt & mkdir commits & type nul > NowLastCommits.txt & echo master > NowBranch.txt & type nul > LocalOrGlobal.txt");
        copy_global_to_all();
    }
}
void add_to_stage(int argc, char **argv)
{
    char address[MAX_SIZE];
    GetCurrentDirectory(sizeof(address), address);
    debug(address);
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

                        CopyFileToHiddenFolder(argv[i], location);
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
                                    CopyFileToHiddenFolder(absolute_file_path, file_full_path);
                                }
                            }
                        }
                        free(argv_copy);
                    }
                    else
                    {
                        if (access(argv[i], F_OK) == 0)
                        {
                            CopyFileToHiddenFolder(argv[i], location);
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
                delete_file(dir_address, argv[i]);
            }
            else
            {

                delete_file(dir_address, address_reverse);
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
            delete_empty_subdirs(path);
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

    char *location = malloc(MAX_SIZE);
    strcpy(location, project_path);
    strcat(location, "\\ngit\\StageCase.txt");
    FILE *open_stage = fopen(location, "r");
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
        FILE *write_stage = fopen(location, "w");
        fprintf(write_stage, "%d", 1);
        fclose(open_stage);
        // FILE *
        free(commit_places);
        free(command);
    }
    free(location);
    free(line);
    free(random_number);
    free(stage_commit_address);
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