#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <customer.h>
#include <utils.h>
#include <admin.h>
#include <handler.h>
#include <path.h>

#define MAX_TRANSACTIONS 100
#define BUFFER_SIZE 1024

// ============================================================
// 🧾 LOG TRANSACTIONS
// ============================================================
void log_transaction(const char *username, const char *sender, const char *receiver, float amount, const char *type)
{
    char filename[150];
    snprintf(filename, sizeof(filename), "%s/%s_transactions.csv", CUSTOMER_DATA_PATH, username);

    FILE *log_file = fopen(filename, "a+");
    if (log_file == NULL)
    {
        perror("❌ Error opening transaction history file");
        return;
    }

    time_t now = time(NULL);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

    fprintf(log_file, "%s,%s,%s,%.2f,%s,%s\n", username, sender, receiver, amount, type, timestamp);
    fclose(log_file);
}

// ============================================================
// 💳 RETRIEVE TRANSACTION HISTORY
// ============================================================
void retrieve_transaction_history(int sock, char *username)
{
    char filename[150];
    snprintf(filename, sizeof(filename), "%s/%s_transactions.csv", CUSTOMER_DATA_PATH, username);
    FILE *file = fopen(filename, "r");

    if (file == NULL)
    {
        perror("❌ Error opening transaction history file");
        char *msg = "⚠️  No transactions found for this account.\n";
        send(sock, msg, strlen(msg), 0);
        return;
    }

    struct Transaction transactions[MAX_TRANSACTIONS];
    int count = 0;
    char buffer[256];

    while (fgets(buffer, sizeof(buffer), file))
    {
        sscanf(buffer, "%19[^,],%19[^,],%19[^,],%f,%9[^,],%19[^,]",
               transactions[count].transaction_id,
               transactions[count].sender,
               transactions[count].receiver,
               &transactions[count].amount,
               transactions[count].type,
               transactions[count].timestamp);
        count++;
    }

    fclose(file);

    char send_buffer[BUFFER_SIZE];
    snprintf(send_buffer, sizeof(send_buffer),
             "========================================\n"
             "📜 Transaction History for %s\n"
             "========================================\n",
             username);

    for (int i = count - 1; i >= 0; i--)
    {
        snprintf(send_buffer + strlen(send_buffer), sizeof(send_buffer) - strlen(send_buffer),
                 "🆔 ID: %s | 💸 %.2f | 🧾 %s → %s | 📅 %s\n",
                 transactions[i].transaction_id,
                 transactions[i].amount,
                 transactions[i].sender,
                 transactions[i].receiver,
                 transactions[i].timestamp);
    }

    send(sock, send_buffer, strlen(send_buffer), 0);
}

// ============================================================
// 💰 CHECK ACCOUNT BALANCE
// ============================================================
float check_balance(char *uname)
{
    char filename[150];
    snprintf(filename, sizeof(filename), "%s/%s.csv", CUSTOMER_DATA_PATH, uname);

    int fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
        perror("❌ Error opening account file");
        return -1;
    }

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
        perror("❌ Error reading file");
        unlock_file(fd);
        close(fd);
        return -1;
    }

    buffer[bytes_read] = '\0';
    sscanf(buffer, "%[^,],%[^,],%[^,],%[^,],%f,%d,%f",
           cust.username, cust.account_number, cust.name, cust.phone_number,
           &cust.balance, &cust.loan_status, &cust.loan_amount);

    unlock_file(fd);
    close(fd);

    printf("========================================\n");
    printf("💰 Current Balance for %s: ₹%.2f\n", uname, cust.balance);
    printf("========================================\n");

    return cust.balance;
}

// ============================================================
// 💵 DEPOSIT OR WITHDRAW BALANCE
// ============================================================
int deposit_balance(char *uname, float amount)
{
    char filename[150];
    snprintf(filename, sizeof(filename), "%s/%s.csv", CUSTOMER_DATA_PATH, uname);

    int fd = open(filename, O_RDWR);
    if (fd == -1)
    {
        perror("❌ Error opening account file");
        return -1;
    }

    if (lock_file(fd, F_WRLCK) == -1)
    {
        close(fd);
        return -1;
    }

    struct Customer cust;
    char buffer[256];
    int bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1)
    {
        perror("❌ Error reading file");
        unlock_file(fd);
        close(fd);
        return -1;
    }
    buffer[bytes_read] = '\0';

    sscanf(buffer, "%[^,],%[^,],%[^,],%[^,],%f,%d,%f",
           cust.username, cust.account_number, cust.name, cust.phone_number,
           &cust.balance, &cust.loan_status, &cust.loan_amount);

    if (cust.balance + amount < 0)
    {
        unlock_file(fd);
        close(fd);
        printf("⚠️  Insufficient balance.\n");
        return -1;
    }

    cust.balance += amount;

    lseek(fd, 0, SEEK_SET);
    int bytes_written = snprintf(buffer, sizeof(buffer),
                                 "%s,%s,%s,%s,%.2f,%d,%.2f\n",
                                 cust.username, cust.account_number, cust.name, cust.phone_number,
                                 cust.balance, cust.loan_status, cust.loan_amount);

    if (write(fd, buffer, bytes_written) == -1)
    {
        perror("❌ Error writing to file");
        close(fd);
        return -1;
    }

    unlock_file(fd);
    close(fd);

    if (amount > 0)
        log_transaction(uname, "self", "self", amount, "Deposit");
    else
        log_transaction(uname, "self", "self", amount, "Withdraw");

    printf("✅ Transaction successful! New balance: ₹%.2f\n", cust.balance);
    return 0;
}

// ============================================================
// 🔁 TRANSFER FUNDS BETWEEN CUSTOMERS
// ============================================================
int transfer_funds(char *sender_uname, char *receiver_uname, float amount)
{
    if (strcmp(sender_uname, receiver_uname) == 0)
    {
        printf("⚠️  Cannot transfer to the same account.\n");
        return -1;
    }

    char sender_file[150], receiver_file[150];
    snprintf(sender_file, sizeof(sender_file), "%s/%s.csv", CUSTOMER_DATA_PATH, sender_uname);
    snprintf(receiver_file, sizeof(receiver_file), "%s/%s.csv", CUSTOMER_DATA_PATH, receiver_uname);

    int sender_fd = open(sender_file, O_RDWR);
    if (sender_fd == -1)
    {
        perror("❌ Error opening sender account file");
        return -1;
    }

    int receiver_fd = open(receiver_file, O_RDWR);
    if (receiver_fd == -1)
    {
        perror("❌ Error opening receiver account file");
        close(sender_fd);
        return -1;
    }

    if (lock_file(sender_fd, F_WRLCK) == -1 || lock_file(receiver_fd, F_WRLCK) == -1)
    {
        close(sender_fd);
        close(receiver_fd);
        return -1;
    }

    struct Customer sender, receiver;
    char buffer[256];

    read(sender_fd, buffer, sizeof(buffer) - 1);
    sscanf(buffer, "%[^,],%[^,],%[^,],%[^,],%f,%d,%f",
           sender.username, sender.account_number, sender.name, sender.phone_number,
           &sender.balance, &sender.loan_status, &sender.loan_amount);

    read(receiver_fd, buffer, sizeof(buffer) - 1);
    sscanf(buffer, "%[^,],%[^,],%[^,],%[^,],%f,%d,%f",
           receiver.username, receiver.account_number, receiver.name, receiver.phone_number,
           &receiver.balance, &receiver.loan_status, &receiver.loan_amount);

    if (sender.balance < amount)
    {
        printf("⚠️  Insufficient funds for transfer.\n");
        unlock_file(sender_fd);
        unlock_file(receiver_fd);
        close(sender_fd);
        close(receiver_fd);
        return -1;
    }

    sender.balance -= amount;
    receiver.balance += amount;

    lseek(sender_fd, 0, SEEK_SET);
    snprintf(buffer, sizeof(buffer),
             "%s,%s,%s,%s,%.2f,%d,%.2f\n",
             sender.username, sender.account_number, sender.name, sender.phone_number,
             sender.balance, sender.loan_status, sender.loan_amount);
    write(sender_fd, buffer, strlen(buffer));

    lseek(receiver_fd, 0, SEEK_SET);
    snprintf(buffer, sizeof(buffer),
             "%s,%s,%s,%s,%.2f,%d,%.2f\n",
             receiver.username, receiver.account_number, receiver.name, receiver.phone_number,
             receiver.balance, receiver.loan_status, receiver.loan_amount);
    write(receiver_fd, buffer, strlen(buffer));

    unlock_file(sender_fd);
    unlock_file(receiver_fd);
    close(sender_fd);
    close(receiver_fd);

    log_transaction(sender_uname, sender_uname, receiver_uname, amount, "Debit");
    log_transaction(receiver_uname, sender_uname, receiver_uname, amount, "Credit");

    printf("✅ Transfer Successful!\n💸 %.2f transferred from %s → %s\n", amount, sender_uname, receiver_uname);
    return 0;
}

// ============================================================
// 💬 FEEDBACK HANDLING
// ============================================================
void submit_feedback(const char *username, const char *feedback)
{
    FILE *file = fopen(FEEDBACK_DATA_PATH, "a");
    if (file == NULL)
    {
        perror("❌ Error opening feedback file");
        return;
    }

    fprintf(file, "%s,\"%s\"\n", username, feedback);
    fclose(file);

    printf("✅ Feedback submitted successfully! Thank you, %s.\n", username);
}

void get_feedbacks(char *result)
{
    FILE *file = fopen(FEEDBACK_DATA_PATH, "r");
    if (file == NULL)
    {
        strcpy(result, "⚠️  No feedback available.\n");
        return;
    }

    char lines[100][256];
    int count = 0;

    while (fgets(lines[count], sizeof(lines[count]), file))
    {
        lines[count][strcspn(lines[count], "\n")] = 0;
        count++;
    }

    fclose(file);

    int start = count >= 5 ? count - 5 : 0;
    strcpy(result, "========================================\n📋 Last 5 Feedbacks:\n========================================\n");

    for (int i = start; i < count; i++)
    {
        strcat(result, lines[i]);
        strcat(result, "\n");
    }

    if (count == 0)
    {
        strcpy(result, "⚠️  No feedback available.\n");
    }
}
