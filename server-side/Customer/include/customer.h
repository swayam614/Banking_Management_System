#ifndef __CUSTOMER_H__
#define __CUSTOMER_H__

// Function prototypes
// int login_customer(char *uname, char *psd);
// void customer_fun();
float check_balance(char *);
int deposit_balance(char *, float);
int transfer_funds(char *, char *, float);
int apply_loan(char *);
int change_password_customer(int);
int add_feedback(char *str);
char *get_transaction(char *);
void retrieve_transaction_history(int, char *);
extern char whoami[20];
void submit_feedback(const char *username, const char *feedback);
void get_feedbacks(char *result);

#endif
