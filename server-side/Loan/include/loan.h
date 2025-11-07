#ifndef __LOAN_H__
#define __LOAN_H__

typedef struct
{
    int loan_id;
    char customer_id[50];
    float loan_amount;
    char loan_type[50];
    int term;
    char status[20];
    char assigned_to[50];
} Loan;

// Function declarations
int apply_for_loan(const char *customer_id, float loan_amount, const char *loan_type, int term);
int assign_loan_to_employee(int loan_id, const char *employee_id);
int approve_or_reject_loan(int loan_id, const char *employee_id, const char *decision);
char *view_customer_loans(const char *customer_id);
char *view_unassigned_loans();
char *view_loans_assigned_to_employee(const char *employee_id);

#endif
