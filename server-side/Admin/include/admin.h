#ifndef __ADMIN_H__
#define __ADMIN_H__

// Function prototypes

struct Employee
{
    char username[20];
    char name[100];
    char role[20];
    char phone[15];
    char salary[10];
};

struct Customer
{
    char username[20];
    char account_number[20];
    char name[100];
    char phone_number[15];
    float balance;
    int loan_status;
    float loan_amount;
};

struct Transaction
{
    char transaction_id[20];
    char sender[20];
    char receiver[20];
    float amount;
    char type[10];      // "deposit", "withdraw", "transfer"
    char timestamp[20]; // Format: YYYY-MM-DD HH:MM:SS
};

int add_manager_role(const char *username);
int remove_manager_role(const char *username);
int add_employee(struct Employee emp);
int update_employee_internal(char *uname, char *newval, int opt);
extern char whoami[20];

#endif
