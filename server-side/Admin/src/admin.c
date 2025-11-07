#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <admin.h>
#include <utils.h>
#include <sys/socket.h>
#include <employee.h>
#include <handler.h>
#include <path.h>

#define LINE "----------------------------------------\n"
#define PANEL_HEADER(title)                                 \
    printf("\n========================================\n"); \
    printf("        🔐 ADMIN PANEL – %s\n", title);         \
    printf("========================================\n");

int add_manager_role(const char *username)
{
    PANEL_HEADER("ADD MANAGER ROLE");

    FILE *emp = fopen(EMPLOYEE_CREDENTIALS_DATA_PATH, "r");
    if (emp == NULL)
    {
        perror("Error opening employee_credentials");
        return EXIT_FAILURE;
    }
    int emp_fd = fileno(emp);
    if (lock_file(emp_fd, F_RDLCK) == -1)
    {
        close(emp_fd);
        return -1;
    }

    FILE *mgr = fopen(MANAGER_CREDENTIALS_DATA_PATH, "r");
    if (mgr == NULL)
    {
        perror("Error opening manager_credentials");
        unlock_file(emp_fd);
        close(emp_fd);
        return EXIT_FAILURE;
    }

    int mgr_fd = fileno(mgr);
    if (lock_file(mgr_fd, F_RDLCK) == -1)
    {
        close(mgr_fd);
        unlock_file(emp_fd);
        close(emp_fd);
        return -1;
    }

    char line[100];
    while (fgets(line, sizeof(line), mgr) != NULL)
    {
        char *token = strtok(line, ",");
        if (strcmp(token, username) == 0)
        {
            printf("> Status : ⚠️  User '%s' is already a manager.\n", username);
            unlock_file(mgr_fd);
            unlock_file(emp_fd);
            fclose(emp);
            fclose(mgr);
            printf(LINE);
            return -1;
        }
    }

    fclose(mgr);
    mgr = fopen(MANAGER_CREDENTIALS_DATA_PATH, "a");
    if (mgr == NULL)
    {
        perror("Error opening manager_credentials");
        unlock_file(emp_fd);
        close(emp_fd);
        return EXIT_FAILURE;
    }

    mgr_fd = fileno(mgr);
    if (lock_file(mgr_fd, F_WRLCK) == -1)
    {
        close(mgr_fd);
        unlock_file(emp_fd);
        close(emp_fd);
        return -1;
    }

    while (fgets(line, sizeof(line), emp) != NULL)
    {
        char myline[100];
        strcpy(myline, line);
        char *token = strtok(line, ",");
        if (strcmp(token, username) == 0)
        {
            fwrite(myline, sizeof(char), strlen(myline), mgr);
            printf("> Username : %s\n> Status   : ✅ Manager role assigned successfully.\n", username);
            unlock_file(mgr_fd);
            unlock_file(emp_fd);
            fclose(emp);
            fclose(mgr);
            printf(LINE);
            return 0;
        }
    }

    printf("> Status : ❌ Employee '%s' not found.\n", username);
    printf(LINE);
    return -1;
}

int remove_manager_role(const char *username)
{
    PANEL_HEADER("REMOVE MANAGER ROLE");

    FILE *mgr = fopen(MANAGER_CREDENTIALS_DATA_PATH, "r");
    if (mgr == NULL)
    {
        perror("Error opening manager_credentials");
        return -1;
    }

    int mgr_fd = fileno(mgr);
    if (lock_file(mgr_fd, F_RDLCK) == -1)
    {
        fclose(mgr);
        return -1;
    }

    FILE *temp = fopen("temp", "w");
    if (temp == NULL)
    {
        perror("Error creating temporary file");
        unlock_file(mgr_fd);
        fclose(mgr);
        return -1;
    }

    int temp_fd = fileno(temp);
    if (lock_file(temp_fd, F_WRLCK) == -1)
    {
        fclose(temp);
        fclose(mgr);
        return -1;
    }

    char line[100];
    int found = 0;

    while (fgets(line, sizeof(line), mgr) != NULL)
    {
        char myline[100];
        strcpy(myline, line);
        char *token = strtok(line, ",");
        if (strcmp(token, username) == 0)
        {
            found = 1;
        }
        else
        {
            fwrite(myline, sizeof(char), strlen(myline), temp);
        }
    }

    unlock_file(mgr_fd);
    fclose(mgr);
    fclose(temp);

    if (found)
    {
        rename("temp", MANAGER_CREDENTIALS_DATA_PATH);
        printf("> Username : %s\n> Status   : ✅ Manager role removed successfully.\n", username);
    }
    else
    {
        printf("> Username : %s\n> Status   : ⚠️  Not found in manager list.\n", username);
    }

    printf(LINE);
    return found ? 0 : -1;
}

int add_employee(struct Employee emp)
{
    PANEL_HEADER("ADD EMPLOYEE");

    FILE *file = fopen(EMPLOYEE_DATA_PATH, "a+");
    if (file == NULL)
    {
        perror("Error opening employee_data.csv");
        return -1;
    }

    int fd = fileno(file);
    if (lock_file(fd, F_WRLCK) == -1)
    {
        fclose(file);
        return -1;
    }

    struct Employee temp;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file))
    {
        sscanf(buffer, "%[^,],%[^,],%[^,],%[^,],%[^\n]", temp.username, temp.name, temp.role, temp.phone, temp.salary);
        if (strcmp(temp.username, emp.username) == 0)
        {
            printf("> Username : %s\n> Status   : ⚠️  Employee already exists!\n", emp.username);
            unlock_file(fd);
            fclose(file);
            printf(LINE);
            return -1;
        }
    }

    fprintf(file, "%s,%s,%s,%s,%s\n", emp.username, emp.name, emp.role, emp.phone, emp.salary);
    printf("> Username : %s\n> Status   : ✅ Employee data saved successfully!\n", emp.username);

    unlock_file(fd);
    fclose(file);

    int pwd_fd = open(EMPLOYEE_CREDENTIALS_DATA_PATH, O_RDWR | O_CREAT | O_APPEND, 0666);
    if (pwd_fd == -1)
    {
        perror("Error opening employee_credentials");
        return -1;
    }

    if (lock_file(pwd_fd, F_WRLCK) == -1)
    {
        close(pwd_fd);
        return -1;
    }

    char password_entry[40];
    snprintf(password_entry, sizeof(password_entry), "%s,%s\n", emp.username, "12345");
    write(pwd_fd, password_entry, strlen(password_entry));

    unlock_file(pwd_fd);
    close(pwd_fd);

    printf(LINE);
    return 0;
}

int update_employee_internal(char *uname, char *newval, int opt)
{
    PANEL_HEADER("UPDATE EMPLOYEE");

    FILE *file = fopen(EMPLOYEE_DATA_PATH, "r+");
    if (file == NULL)
    {
        perror("Error opening employee_data.csv");
        return -1;
    }

    int fd = fileno(file);
    if (lock_file(fd, F_WRLCK) == -1)
    {
        fclose(file);
        return -1;
    }

    FILE *tempFile = fopen("temp.csv", "w");
    if (tempFile == NULL)
    {
        perror("Error opening temp.csv");
        unlock_file(fd);
        fclose(file);
        return -1;
    }

    struct Employee temp;
    char buffer[256];
    int found = 0;

    while (fgets(buffer, sizeof(buffer), file))
    {
        sscanf(buffer, "%[^,],%[^,],%[^,],%[^,],%[^\n]", temp.username, temp.name, temp.role, temp.phone, temp.salary);

        if (strcmp(temp.username, uname) == 0)
        {
            found = 1;
            switch (opt)
            {
            case 1:
                strncpy(temp.username, newval, sizeof(temp.username) - 1);
                break;
            case 2:
                strncpy(temp.name, newval, sizeof(temp.name) - 1);
                break;
            case 3:
                strncpy(temp.role, newval, sizeof(temp.role) - 1);
                break;
            case 4:
                strncpy(temp.phone, newval, sizeof(temp.phone) - 1);
                break;
            case 5:
                strncpy(temp.salary, newval, sizeof(temp.salary) - 1);
                break;
            default:
                printf("> Status : ⚠️  Invalid update option.\n");
                break;
            }
        }
        fprintf(tempFile, "%s,%s,%s,%s,%s\n", temp.username, temp.name, temp.role, temp.phone, temp.salary);
    }

    fclose(file);
    fclose(tempFile);

    if (found)
    {
        remove(EMPLOYEE_DATA_PATH);
        rename("temp.csv", EMPLOYEE_DATA_PATH);
        printf("> Username : %s\n> Status   : ✅ Updated successfully.\n", uname);
    }
    else
    {
        remove("temp.csv");
        printf("> Username : %s\n> Status   : ⚠️  Employee not found!\n", uname);
    }

    printf(LINE);
    unlock_file(fd);
    return found ? 0 : -1;
}
