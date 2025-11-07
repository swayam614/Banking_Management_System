#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <employee.h>
#include <admin.h>
#include <utils.h>
#include <path.h>

// 🔒 Update a customer's information (internal use)
int update_customer_internal(char *uname, char *newval, int opt)
{
    char filename[150];
    snprintf(filename, sizeof(filename), "%s/%s.csv", CUSTOMER_DATA_PATH, uname);

    int fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
        perror("❌ Error opening customer file");
        return -1;
    }

    // Lock file for reading
    if (lock_file(fd, F_RDLCK) == -1)
    {
        close(fd);
        return -1;
    }

    struct Customer cust;
    char buffer[256];
    int bytes_read = read(fd, buffer, sizeof(buffer) - 1);

    if (bytes_read == -1)
    {
        perror("❌ Error reading the file");
        unlock_file(fd);
        close(fd);
        return -1;
    }

    buffer[bytes_read] = '\0';
    sscanf(buffer, "%[^,],%[^,],%[^,],%[^,],%f,%d,%f\n",
           cust.username, cust.account_number, cust.name,
           cust.phone_number, &cust.balance, &cust.loan_status, &cust.loan_amount);

    unlock_file(fd);
    close(fd);

    // Update the selected field
    switch (opt)
    {
    case 1:
        strcpy(cust.account_number, newval);
        break;
    case 2:
        strcpy(cust.name, newval);
        break;
    case 3:
        strcpy(cust.phone_number, newval);
        break;
    default:
        printf("⚠️ Invalid update option\n");
        return -1;
    }

    // Write updated data
    fd = open(filename, O_WRONLY | O_TRUNC, 0666);
    if (fd == -1)
    {
        perror("❌ Error opening file for writing");
        return -1;
    }

    if (lock_file(fd, F_WRLCK) == -1)
    {
        close(fd);
        return -1;
    }

    int bytes_written = snprintf(buffer, sizeof(buffer), "%s,%s,%s,%s,%.2f,%d,%.2f\n",
                                 cust.username, cust.account_number, cust.name,
                                 cust.phone_number, cust.balance,
                                 cust.loan_status, cust.loan_amount);

    if (write(fd, buffer, bytes_written) == -1)
    {
        perror("❌ Error writing updated data");
        unlock_file(fd);
        close(fd);
        return -1;
    }

    unlock_file(fd);
    close(fd);
    printf("✅ Customer data updated successfully!\n");
    return 0;
}

// 🧾 Add a new customer record
int add_customer_internal(struct Customer cust)
{
    char filename[150];
    snprintf(filename, sizeof(filename), "%s/%s.csv", CUSTOMER_DATA_PATH, cust.username);

    int fd = open(filename, O_CREAT | O_EXCL | O_RDWR, 0666);
    if (fd == -1)
    {
        perror("❌ Error creating customer file");
        return -1;
    }

    if (lock_file(fd, F_WRLCK) == -1)
    {
        close(fd);
        return -1;
    }

    // Write new customer details
    char buffer[200];
    int bytes_written = snprintf(buffer, sizeof(buffer), "%s,%s,%s,%s,%.2f,%d,%.2f\n",
                                 cust.username, cust.account_number, cust.name,
                                 cust.phone_number, cust.balance,
                                 cust.loan_status, cust.loan_amount);

    if (write(fd, buffer, bytes_written) == -1)
    {
        perror("❌ Error writing customer data");
        close(fd);
        return -1;
    }

    printf("✅ Customer data saved successfully!\n");

    unlock_file(fd);
    close(fd);

    // Add default password to credentials file
    int pwd_fd = open(CUSTOMER_CREDENTIALS_DATA_PATH, O_RDWR | O_CREAT | O_APPEND, 0666);
    if (pwd_fd == -1)
    {
        perror("❌ Error opening customer_credentials file");
        return -1;
    }

    if (lock_file(pwd_fd, F_WRLCK) == -1)
    {
        close(pwd_fd);
        return -1;
    }

    char password_entry[50];
    snprintf(password_entry, sizeof(password_entry), "%s,%s\n", cust.username, "12345");

    if (write(pwd_fd, password_entry, strlen(password_entry)) == -1)
    {
        perror("❌ Error writing to credentials file");
        unlock_file(pwd_fd);
        close(pwd_fd);
        return -1;
    }

    unlock_file(pwd_fd);
    close(pwd_fd);

    return 0;
}

// 👤 Add customer — called from server
void add_cust(int sock)
{
    struct Customer cust;
    char menu[] =
        "========================================\n"
        "🧾 ADD NEW CUSTOMER\n"
        "========================================\n"
        "👉 Enter details in the following order:\n"
        "1️⃣ Username\n2️⃣ Account Number\n3️⃣ Name\n4️⃣ Phone\n5️⃣ Balance\n\n> ";

    send(sock, menu, strlen(menu), 0);

    int dummy = 0;
    read(sock, cust.username, sizeof(cust.username));
    send(sock, &dummy, sizeof(int), 0);
    read(sock, cust.account_number, sizeof(cust.account_number));
    send(sock, &dummy, sizeof(int), 0);
    read(sock, cust.name, sizeof(cust.name));
    send(sock, &dummy, sizeof(int), 0);
    read(sock, cust.phone_number, sizeof(cust.phone_number));
    send(sock, &dummy, sizeof(int), 0);
    read(sock, &cust.balance, sizeof(cust.balance));
    send(sock, &dummy, sizeof(int), 0);

    cust.loan_status = 0;
    cust.loan_amount = 0.0;

    int flag = add_customer_internal(cust);
    const char *msg = (flag == 0)
                          ? "✅ Customer added successfully!\n"
                          : "❌ Failed to add customer!\n";
    send(sock, msg, strlen(msg), 0);
}

// ✏️ Modify existing customer details
void modify_customer_employee(int sock)
{
    struct Customer cust;
    char uname[20];

    char *msg = "🔍 Enter username to update: ";
    send(sock, msg, strlen(msg), 0);
    read(sock, uname, sizeof(uname));

    char menu[] =
        "========================================\n"
        "✏️ MODIFY CUSTOMER DETAILS\n"
        "========================================\n"
        "1️⃣ Account Number\n2️⃣ Name\n3️⃣ Phone\n\n> Enter your choice: ";

    send(sock, menu, strlen(menu), 0);

    int opt;
    read(sock, &opt, sizeof(int));

    int flag = -1;

    switch (opt)
    {
    case 1:
        msg = "Enter new Account Number: ";
        send(sock, msg, strlen(msg), 0);
        read(sock, cust.account_number, sizeof(cust.account_number));
        flag = update_customer_internal(uname, cust.account_number, 1);
        break;

    case 2:
        msg = "Enter new Name: ";
        send(sock, msg, strlen(msg), 0);
        read(sock, cust.name, sizeof(cust.name));
        flag = update_customer_internal(uname, cust.name, 2);
        break;

    case 3:
        msg = "Enter new Phone Number: ";
        send(sock, msg, strlen(msg), 0);
        read(sock, cust.phone_number, sizeof(cust.phone_number));
        flag = update_customer_internal(uname, cust.phone_number, 3);
        break;

    default:
        msg = "⚠️ Invalid choice!\n";
        send(sock, msg, strlen(msg), 0);
        return;
    }

    const char *result = (flag == 0)
                             ? "✅ Update successful!\n"
                             : "❌ Update failed!\n";
    send(sock, result, strlen(result), 0);
}
