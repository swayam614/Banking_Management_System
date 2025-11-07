#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <manager.h>
#include <utils.h>
#include <loan.h>
#include <path.h>

/*-----------------------------------------------------
 * Function: apply_for_loan
 * Handles new loan applications by customers.
 *----------------------------------------------------*/
int apply_for_loan(const char *customer_id, float loan_amount, const char *loan_type, int term)
{
    Loan new_loan;
    static int loan_counter = 1;

    // Fetch the latest loan ID
    FILE *file = fopen(LOAN_PATH, "r");
    if (file != NULL)
    {
        char line[256];
        int last_id = 0;
        while (fgets(line, sizeof(line), file))
        {
            sscanf(line, "%d", &last_id);
        }
        loan_counter = last_id + 1;
        fclose(file);
    }

    // Assign loan details
    new_loan.loan_id = loan_counter++;
    strcpy(new_loan.customer_id, customer_id);
    new_loan.loan_amount = loan_amount;
    strcpy(new_loan.loan_type, loan_type);
    new_loan.term = term;
    strcpy(new_loan.status, "pending");
    strcpy(new_loan.assigned_to, "unassigned");

    // Save the loan
    file = fopen(LOAN_PATH, "a");
    if (file == NULL)
    {
        perror("❌ Error opening loan file");
        return -1;
    }

    fprintf(file, "%d,%s,%.2f,%s,%d,%s,%s\n",
            new_loan.loan_id, new_loan.customer_id, new_loan.loan_amount,
            new_loan.loan_type, new_loan.term, new_loan.status, new_loan.assigned_to);

    fclose(file);

    printf("\n============================================\n");
    printf("💰  LOAN APPLICATION SUBMISSION\n");
    printf("============================================\n");
    printf("✅ Loan Application Submitted Successfully!\n");
    printf("📄 Customer ID : %s\n", customer_id);
    printf("💵 Amount      : %.2f\n", loan_amount);
    printf("🏦 Type        : %s\n", loan_type);
    printf("📆 Term        : %d months\n", term);
    printf("--------------------------------------------\n\n");

    return 0;
}

/*-----------------------------------------------------
 * Function: assign_loan_to_employee
 * Assigns a pending loan to a specific employee.
 *----------------------------------------------------*/
int assign_loan_to_employee(int loan_id, const char *employee_id)
{
    FILE *file = fopen(LOAN_PATH, "r");
    FILE *temp = fopen("loans_temp.txt", "w");
    if (!file || !temp)
    {
        perror("❌ Error opening loan file");
        return -1;
    }

    char line[256];
    int found = 0;

    while (fgets(line, sizeof(line), file))
    {
        Loan loan;
        sscanf(line, "%d,%[^,],%f,%[^,],%d,%[^,],%[^,\n]",
               &loan.loan_id, loan.customer_id, &loan.loan_amount,
               loan.loan_type, &loan.term, loan.status, loan.assigned_to);

        if (loan.loan_id == loan_id && strcmp(loan.status, "pending") == 0)
        {
            strcpy(loan.assigned_to, employee_id);
            found = 1;

            printf("\n============================================\n");
            printf("📋  LOAN ASSIGNMENT UPDATE\n");
            printf("============================================\n");
            printf("✅ Loan ID %d assigned to Employee: %s\n", loan_id, employee_id);
            printf("--------------------------------------------\n\n");
        }

        fprintf(temp, "%d,%s,%.2f,%s,%d,%s,%s\n",
                loan.loan_id, loan.customer_id, loan.loan_amount,
                loan.loan_type, loan.term, loan.status, loan.assigned_to);
    }

    fclose(file);
    fclose(temp);
    rename("loans_temp.txt", LOAN_PATH);

    if (!found)
        printf("⚠️  Loan ID %d not found or already assigned.\n", loan_id);

    return 0;
}

/*-----------------------------------------------------
 * Function: approve_or_reject_loan
 * Lets employees approve or reject a loan.
 *----------------------------------------------------*/
int approve_or_reject_loan(int loan_id, const char *employee_id, const char *decision)
{
    FILE *file = fopen(LOAN_PATH, "r");
    FILE *temp = fopen("loans_temp.txt", "w");
    if (!file || !temp)
    {
        perror("❌ Error opening loan file");
        return -1;
    }

    char line[256];
    int found = 0;

    while (fgets(line, sizeof(line), file))
    {
        Loan loan;
        sscanf(line, "%d,%[^,],%f,%[^,],%d,%[^,],%[^,\n]",
               &loan.loan_id, loan.customer_id, &loan.loan_amount,
               loan.loan_type, &loan.term, loan.status, loan.assigned_to);

        if (loan.loan_id == loan_id && strcmp(loan.assigned_to, employee_id) == 0)
        {
            strcpy(loan.status, decision);
            found = 1;

            printf("\n============================================\n");
            printf("🏦  LOAN DECISION UPDATE\n");
            printf("============================================\n");
            printf("✅ Loan ID %d has been %s by Employee: %s\n", loan_id, decision, employee_id);
            printf("--------------------------------------------\n\n");
        }

        fprintf(temp, "%d,%s,%.2f,%s,%d,%s,%s\n",
                loan.loan_id, loan.customer_id, loan.loan_amount,
                loan.loan_type, loan.term, loan.status, loan.assigned_to);
    }

    fclose(file);
    fclose(temp);
    rename("loans_temp.txt", LOAN_PATH);

    if (!found)
        printf("⚠️  Loan not found or not assigned to this employee.\n");

    return 0;
}

/*-----------------------------------------------------
 * Function: view_customer_loans
 * Displays all loans belonging to a specific customer.
 *----------------------------------------------------*/
char *view_customer_loans(const char *customer_id)
{
    FILE *file = fopen(LOAN_PATH, "r");
    if (file == NULL)
    {
        perror("❌ Error opening loan file");
        return NULL;
    }

    char *result = malloc(4096);
    if (!result)
    {
        perror("Memory allocation error");
        fclose(file);
        return NULL;
    }

    strcpy(result, "\n============================================\n");
    strcat(result, "📄  CUSTOMER LOAN DETAILS\n");
    strcat(result, "============================================\n");
    strcat(result, "Loan ID | Amount  | Type        | Term | Status    | Assigned To\n");
    strcat(result, "---------------------------------------------------------------\n");

    char line[256];
    int found = 0;

    while (fgets(line, sizeof(line), file))
    {
        Loan loan;
        sscanf(line, "%d,%[^,],%f,%[^,],%d,%[^,],%[^,\n]",
               &loan.loan_id, loan.customer_id, &loan.loan_amount,
               loan.loan_type, &loan.term, loan.status, loan.assigned_to);

        if (strcmp(loan.customer_id, customer_id) == 0)
        {
            char info[256];
            sprintf(info, "%7d | %.2f | %-10s | %4d | %-9s | %-10s\n",
                    loan.loan_id, loan.loan_amount, loan.loan_type,
                    loan.term, loan.status, loan.assigned_to);
            strcat(result, info);
            found = 1;
        }
    }

    if (!found)
        strcat(result, "No loans found for this customer.\n");

    fclose(file);
    return result;
}

/*-----------------------------------------------------
 * Function: view_unassigned_loans
 * Displays all loans that are not yet assigned.
 *----------------------------------------------------*/
char *view_unassigned_loans()
{
    FILE *file = fopen(LOAN_PATH, "r");
    if (file == NULL)
    {
        perror("❌ Error opening loan file");
        return NULL;
    }

    char *result = malloc(2048);
    if (!result)
    {
        perror("Memory allocation error");
        fclose(file);
        return NULL;
    }
    result[0] = '\0';

    strcat(result, "\n============================================\n");
    strcat(result, "📋  UNASSIGNED LOANS LIST\n");
    strcat(result, "============================================\n");
    strcat(result, "Loan ID | Customer ID | Amount  | Type        | Term | Status\n");
    strcat(result, "------------------------------------------------------------\n");

    char line[256];
    int found = 0;

    while (fgets(line, sizeof(line), file))
    {
        Loan loan;
        sscanf(line, "%d,%[^,],%f,%[^,],%d,%[^,],%[^,\n]",
               &loan.loan_id, loan.customer_id, &loan.loan_amount,
               loan.loan_type, &loan.term, loan.status, loan.assigned_to);

        if (strcmp(loan.assigned_to, "unassigned") == 0)
        {
            char row[256];
            sprintf(row, "%7d | %-11s | %.2f | %-10s | %4d | %-9s\n",
                    loan.loan_id, loan.customer_id, loan.loan_amount,
                    loan.loan_type, loan.term, loan.status);
            strcat(result, row);
            found = 1;
        }
    }

    if (!found)
        strcat(result, "No unassigned loans found.\n");

    fclose(file);
    return result;
}

/*-----------------------------------------------------
 * Function: view_loans_assigned_to_employee
 * Displays all loans assigned to a specific employee.
 *----------------------------------------------------*/
char *view_loans_assigned_to_employee(const char *employee_id)
{
    FILE *file = fopen(LOAN_PATH, "r");
    if (file == NULL)
    {
        perror("❌ Error opening loan file");
        return NULL;
    }

    char *result = malloc(2048);
    if (!result)
    {
        perror("Memory allocation error");
        fclose(file);
        return NULL;
    }
    result[0] = '\0';

    strcat(result, "\n============================================\n");
    strcat(result, "👷  LOANS ASSIGNED TO EMPLOYEE\n");
    strcat(result, "============================================\n");
    strcat(result, "Loan ID | Customer ID | Amount  | Type        | Term | Status\n");
    strcat(result, "------------------------------------------------------------\n");

    char line[256];
    int found = 0;

    while (fgets(line, sizeof(line), file))
    {
        Loan loan;
        sscanf(line, "%d,%[^,],%f,%[^,],%d,%[^,],%[^,\n]",
               &loan.loan_id, loan.customer_id, &loan.loan_amount,
               loan.loan_type, &loan.term, loan.status, loan.assigned_to);

        if (strcmp(loan.assigned_to, employee_id) == 0)
        {
            char row[256];
            sprintf(row, "%7d | %-11s | %.2f | %-10s | %4d | %-9s\n",
                    loan.loan_id, loan.customer_id, loan.loan_amount,
                    loan.loan_type, loan.term, loan.status);
            strcat(result, row);
            found = 1;
        }
    }

    if (!found)
        strcat(result, "No loans assigned to this employee.\n");

    fclose(file);
    return result;
}
