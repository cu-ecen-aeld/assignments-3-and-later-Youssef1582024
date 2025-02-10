#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#define MAX_PATH_LEN 1024

int check_arguments(int argc, char *argv[]) {
    if (argc != 3) {
        syslog(LOG_ERR, "Error: Missing arguments. Please provide both file path and content.");
        return 1;
    }
    return 0;
}

int write_to_file(const char *writefile, const char *writestr) {
    FILE *file = fopen(writefile, "w");
    if (file == NULL) {
        syslog(LOG_ERR, "Error: Failed to open file %s for writing.", writefile);
        return 1;
    }

    if (fprintf(file, "%s", writestr) < 0) {
        syslog(LOG_ERR, "Error: Failed to write to file %s.", writefile);
        fclose(file);
        return 1;
    }

    fclose(file);
    return 0;
}

int main(int argc, char *argv[]) {
    openlog("writer", LOG_PID | LOG_CONS, LOG_USER);

    // Check if the arguments are correct
    if (check_arguments(argc, argv)) {
        closelog();
        return EXIT_FAILURE;
    }

    // Extract file path and string to write
    const char *writefile = argv[1];
    const char *writestr = argv[2];

    // Log the action with LOG_DEBUG
    syslog(LOG_DEBUG, "Writing \"%s\" to %s", writestr, writefile);

    // Write the string to the file
    if (write_to_file(writefile, writestr)) {
        closelog();
        return EXIT_FAILURE;
    }

    // Log success with LOG_DEBUG
    syslog(LOG_DEBUG, "File created successfully at %s with content: \"%s\"", writefile, writestr);

    closelog();
    return EXIT_SUCCESS;
}

