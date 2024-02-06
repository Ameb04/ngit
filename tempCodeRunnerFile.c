char *cwd = malloc(MAX_SIZE);
    getcwd(cwd, strlen(cwd));
    debug(cwd);