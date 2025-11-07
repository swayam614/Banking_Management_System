#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <utils.h>
#include <handler.h>
#include <server.h>
#include <path.h>

char whoami[20];

/*-----------------------------------------------------
 * Function: read_line
 * Reads a single line from a file descriptor.
 *----------------------------------------------------*/
int read_line(int fd, char *buffer)
{
    char c;
    int i = 0;
    while (read(fd, &c, 1) == 1)
    {
        if (c == '\n' || c == '\0')
        {
            buffer[i] = '\0';
            return 1;
        }
        buffer[i++] = c;
    }
    return (i > 0); // Return true if at least one character was read
}

/*-----------------------------------------------------
 * Function: lock_file
 * Locks a file (read or write lock).
 *----------------------------------------------------*/
int lock_file(int fd, int lock_type)
{
    struct flock lock;
    lock.l_type = lock_type;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();

    if (fcntl(fd, F_SETLKW, &lock) == -1)
    {
        perror("❌ Error acquiring lock");
        return -1;
    }
    return 0;
}

/*-----------------------------------------------------
 * Function: unlock_file
 * Unlocks a locked file.
 *----------------------------------------------------*/
void unlock_file(int fd)
{
    struct flock lock;
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(fd, F_SETLK, &lock) == -1)
    {
        perror("❌ Error releasing lock");
    }
}

/*-----------------------------------------------------
 * Function: authenticate_user
 * Authenticates the user based on credentials.
 *----------------------------------------------------*/
int authenticate_user(char *username, char *password, int choice)
{

    printf("\n===================================\n");
    printf("🔐  AUTHENTICATION PORTAL\n");
    printf("===================================\n");

    // Prevent multiple logins for same username
    for (int i = 0; i < 10; i++)
    {
        if (strlen(shared_usernames[i]) > 0 && strcmp(shared_usernames[i], username) == 0)
        {
            printf("⚠️  User '%s' is already logged in!\n", username);
            return -1;
        }
    }

    int fd;
    switch (choice)
    {
    case 1:
        fd = open(CUSTOMER_CREDENTIALS_DATA_PATH, O_RDONLY);
        break;
    case 2:
        fd = open(EMPLOYEE_CREDENTIALS_DATA_PATH, O_RDONLY);
        break;
    case 3:
        fd = open(MANAGER_CREDENTIALS_DATA_PATH, O_RDONLY);
        break;
    case 4:
        fd = open(ADMIN_CREDENTIALS_DATA_PATH, O_RDONLY);
        break;
    default:
        printf("❌ Invalid user type.\n");
        return 0;
    }

    if (fd == -1)
    {
        perror("❌ Error opening credentials file");
        return 0;
    }

    char buffer[256];
    char stored_username[50], stored_password[50];
    while (read_line(fd, buffer))
    {
        sscanf(buffer, "%[^,],%s", stored_username, stored_password);

        if (strcmp(username, stored_username) == 0 && strcmp(password, stored_password) == 0)
        {
            close(fd);
            strcpy(whoami, username);

            for (int i = 0; i < 10; i++)
            {
                if (strlen(shared_usernames[i]) == 0)
                {
                    strcpy(shared_usernames[i], username);
                    break;
                }
            }

            printf("✅ Login Successful! Welcome, %s.\n", username);
            printf("===================================\n\n");
            return 1;
        }
    }

    close(fd);
    printf("❌ Invalid Username or Password.\n");
    printf("===================================\n\n");
    return 0;
}

/*-----------------------------------------------------
 * Function: change_password_user
 * Changes password for a given user safely.
 *----------------------------------------------------*/
int change_password_user(const char *username, const char *newpassword, int choice)
{

    printf("\n===================================\n");
    printf("🔑  PASSWORD CHANGE PORTAL\n");
    printf("===================================\n");

    char filename[150];
    switch (choice)
    {
    case 1:
        strcpy(filename, CUSTOMER_CREDENTIALS_DATA_PATH);
        break;
    case 2:
        strcpy(filename, EMPLOYEE_CREDENTIALS_DATA_PATH);
        break;
    case 3:
        strcpy(filename, MANAGER_CREDENTIALS_DATA_PATH);
        break;
    case 4:
        strcpy(filename, ADMIN_CREDENTIALS_DATA_PATH);
        break;
    default:
        printf("❌ Invalid user type.\n");
        return -1;
    }

    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("❌ Error opening credentials file");
        return -1;
    }

    int file_fd = fileno(file);
    if (lock_file(file_fd, F_RDLCK) == -1)
    {
        fclose(file);
        return -1;
    }

    FILE *temp = fopen("temp", "w");
    if (!temp)
    {
        perror("❌ Error creating temporary file");
        unlock_file(file_fd);
        fclose(file);
        return -1;
    }

    int temp_fd = fileno(temp);
    if (lock_file(temp_fd, F_WRLCK) == -1)
    {
        fclose(temp);
        unlock_file(file_fd);
        fclose(file);
        return -1;
    }

    char line[100];
    int found = 0;

    // Read credentials and write updated file
    while (fgets(line, sizeof(line), file) != NULL)
    {
        char copy_line[100];
        strcpy(copy_line, line);

        char *token = strtok(line, ",");
        if (strcmp(token, username) == 0)
        {
            found = 1;
            char newline[100];
            snprintf(newline, sizeof(newline), "%s,%s\n", username, newpassword);
            fwrite(newline, sizeof(char), strlen(newline), temp);
        }
        else
        {
            fwrite(copy_line, sizeof(char), strlen(copy_line), temp);
        }
    }

    unlock_file(file_fd);
    fclose(file);
    unlock_file(temp_fd);
    fclose(temp);

    if (found)
    {
        if (rename("temp", filename) == -1)
        {
            perror("❌ Error updating password file");
            return -1;
        }
        printf("✅ Password successfully updated for user '%s'.\n", username);
    }
    else
    {
        printf("⚠️  Username '%s' not found. No changes made.\n", username);
    }

    printf("===================================\n\n");
    return 0;
}
