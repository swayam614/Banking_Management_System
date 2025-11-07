#ifndef __EMPLOYEE_H__
#define __EMPLOYEE_H__

// Function prototypes
// int login_employee(char*uname,char*psd);
void add_cust(int sock);
void process_loan(int sock);
void approve_loan(int sock);
void assigned_loan(int sock);
void view_transaction(int sock);
void change_pwd(int sock);
void logout_emp(int sock);
void modify_customer_employee(int sock);
extern char whoami[20];

#endif
