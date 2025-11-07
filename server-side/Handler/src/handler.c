#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <handler.h>
#include <customer.h>
#include <employee.h>
#include <manager.h>
#include <admin.h>
#include <utils.h>
#include <loan.h>
#include <fcntl.h>

extern char (*shared_usernames)[50]; 
#define MAX_USERS 100                

// ✅ Logout user from shared memory
void logout_user(const char *username)
{
        for (int i = 0; i < MAX_USERS; i++)
        {
                if (strcmp(shared_usernames[i], username) == 0)
                {
                        memset(shared_usernames[i], 0, sizeof(shared_usernames[i]));
                        printf("✅ User %s logged out and removed from shared memory.\n", username);
                        break;
                }
        }
}

// 💳 Customer main handler function
void customer(int sock)
{
        char *msg = "\n👋 Welcome, Customer!\nPlease enter your username and password to log in:\n";
        int temp;

        send(sock, msg, strlen(msg), 0);

        char uname[1024], pwd[1024];
        read(sock, uname, sizeof(uname));
        read(sock, pwd, sizeof(pwd));

        int flag = authenticate_user(uname, pwd, 1);
        send(sock, &flag, sizeof(int), 0);
        read(sock, &temp, sizeof(temp));

        if (flag == 1)
        {
                msg = "\n✅ Login Successful! Welcome to your dashboard.\n";
                send(sock, msg, strlen(msg), 0);
                read(sock, &temp, sizeof(temp));

                int choice = 0;
                int ack = 1;
                char *whoami = uname;

                while (1)
                {
                        // 📋 Customer Main Menu
                        msg = "\n==============================\n"
                              "💼 CUSTOMER MENU\n"
                              "==============================\n"
                              "1️⃣  View Account Balance\n"
                              "2️⃣  Deposit Money\n"
                              "3️⃣  Withdraw Money\n"
                              "4️⃣  Transfer Funds\n"
                              "5️⃣  Apply for a Loan\n"
                              "6️⃣  Check Loan Status\n"
                              "7️⃣  Change Password\n"
                              "8️⃣  Add Feedback\n"
                              "9️⃣  View Transaction History\n"
                              "🔟  Logout\n"
                              "------------------------------\n"
                              "Enter your choice: ";
                        send(sock, msg, strlen(msg), 0);
                        read(sock, &choice, sizeof(choice));

                        int status;
                        switch (choice)
                        {
                        case 1:
                        {
                                float balance = check_balance(whoami);
                                send(sock, &balance, sizeof(float), 0);
                                read(sock, &ack, sizeof(ack));
                                break;
                        }

                        case 2:
                        {
                                msg = "\n💰 Enter amount to deposit: ";
                                send(sock, msg, strlen(msg), 0);

                                float amount;
                                read(sock, &amount, sizeof(amount));

                                status = deposit_balance(whoami, amount);
                                msg = (status == 0) ? "✅ Deposit Successful!\n" : "❌ Deposit Failed.\n";
                                send(sock, msg, strlen(msg), 0);
                                read(sock, &ack, sizeof(ack));
                                break;
                        }

                        case 3:
                        {
                                msg = "\n🏧 Enter amount to withdraw: ";
                                send(sock, msg, strlen(msg), 0);

                                float amount;
                                read(sock, &amount, sizeof(amount));

                                status = deposit_balance(whoami, -1 * amount);
                                msg = (status == 0) ? "✅ Withdrawal Successful!\n" : "❌ Withdrawal Failed.\n";
                                send(sock, msg, strlen(msg), 0);
                                read(sock, &ack, sizeof(ack));
                                break;
                        }

                        case 4:
                        {
                                msg = "\n➡️  Enter receiver's username: ";
                                send(sock, msg, strlen(msg), 0);

                                char receiver[50];
                                read(sock, receiver, sizeof(receiver));

                                msg = "💸 Enter amount to transfer: ";
                                send(sock, msg, strlen(msg), 0);

                                float amount;
                                read(sock, &amount, sizeof(amount));

                                status = transfer_funds(whoami, receiver, amount);
                                msg = (status == 0) ? "✅ Transfer Successful!\n" : "❌ Transfer Failed.\n";
                                send(sock, msg, strlen(msg), 0);
                                read(sock, &ack, sizeof(ack));
                                break;
                        }

                        case 5:
                        {
                                msg = "\n🏦 Loan Application Portal\n"
                                      "Enter details below:\n"
                                      "1️⃣  Loan Amount\n"
                                      "2️⃣  Loan Type\n"
                                      "3️⃣  Loan Tenure (in months)\n";
                                send(sock, msg, strlen(msg), 0);

                                float loan_amount;
                                char loan_type[30];
                                int tenure;

                                read(sock, &loan_amount, sizeof(float));
                                send(sock, &ack, sizeof(int), 0);

                                read(sock, loan_type, sizeof(loan_type));
                                send(sock, &ack, sizeof(int), 0);

                                read(sock, &tenure, sizeof(int));

                                int res = apply_for_loan(whoami, loan_amount, loan_type, tenure);
                                msg = (res == 0) ? "✅ Loan Application Submitted!\n"
                                                 : "❌ Loan Application Failed.\n";
                                send(sock, msg, strlen(msg), 0);
                                read(sock, &ack, sizeof(ack));
                                break;
                        }

                        case 6:
                        {
                                char *loan_status = view_customer_loans(whoami);
                                send(sock, loan_status, strlen(loan_status), 0);
                                read(sock, &temp, sizeof(temp));
                                free(loan_status);
                                break;
                        }

                        case 7:
                        {
                                msg = "\n🔑 Enter your new password: ";
                                send(sock, msg, strlen(msg), 0);

                                char newpwd[50];
                                read(sock, newpwd, sizeof(newpwd));

                                int result = change_password_user(whoami, newpwd, 1);
                                msg = (result == 0) ? "✅ Password changed successfully!\n"
                                                    : "❌ Failed to change password.\n";
                                send(sock, msg, strlen(msg), 0);
                                read(sock, &ack, sizeof(ack));
                                break;
                        }

                        case 8:
                        {
                                msg = "\n💬 Please enter your feedback below:\n";
                                send(sock, msg, strlen(msg), 0);

                                char feedback[200];
                                read(sock, feedback, sizeof(feedback));

                                submit_feedback(whoami, feedback);
                                msg = "✅ Feedback submitted successfully. Thank you!\n";
                                send(sock, msg, strlen(msg), 0);
                                read(sock, &ack, sizeof(ack));
                                break;
                        }

                        case 9:
                        {
                                retrieve_transaction_history(sock, whoami);
                                read(sock, &temp, sizeof(temp));
                                break;
                        }

                        case 10:
                        {
                                msg = "\n👋 Logging out... Thank you for banking with us!\n";
                                send(sock, msg, strlen(msg), 0);
                                logout_user(whoami);
                                break;
                        }

                        default:
                        {
                                msg = "⚠️  Invalid option. Please try again.\n";
                                send(sock, msg, strlen(msg), 0);
                                read(sock, &ack, sizeof(ack));
                                break;
                        }
                        }

                        if (choice == 10)
                                break;
                }
        }
        else if(flag == -1)
        {
                msg = "🔄 You're already logged in. Please logout before trying again.\n";
                send(sock, msg, strlen(msg), 0);
        }
        else
        {
                msg = "❌ Login Failed. Invalid username or password.\n";
                send(sock,msg,strlen(msg),0);
        }
}

// =====================================================
// 🧑‍💼 EMPLOYEE PORTAL
// =====================================================
void employee(int sock)
{
        char *a = "\n===================================\n"
                  "🧑‍💼  EMPLOYEE LOGIN PORTAL\n"
                  "===================================\n"
                  "👉 ENTER USERNAME AND PASSWORD TO LOGIN:\n";
        int temp;
        send(sock, a, strlen(a), 0);

        char uname[1024], pwd[1024];
        read(sock, uname, 1024);
        read(sock, pwd, 1024);

        int flag = authenticate_user(uname, pwd, 2);
        send(sock, &flag, sizeof(int), 0);
        read(sock, &temp, sizeof(temp));

        if (flag == 1)
        {
                a = "\n✅ LOGIN SUCCESSFUL!\n"
                    "-----------------------------------\n"
                    "Welcome to Employee Dashboard 👋\n";
                send(sock, a, strlen(a), 0);
                read(sock, &temp, sizeof(temp));

                int opt = 0, nop;

                while (1)
                {
                        a = "\n===================================\n"
                            "🧾  EMPLOYEE MENU\n"
                            "===================================\n"
                            "1️⃣  Add New Customer\n"
                            "2️⃣  Modify Customer Detail\n"
                            "3️⃣  Approve / Reject Loan\n"
                            "4️⃣  View Assigned Loan Applications\n"
                            "5️⃣  View Customer Transactions\n"
                            "6️⃣  Change Password\n"
                            "7️⃣  Logout\n"
                            "-----------------------------------\n"
                            "👉 Enter your choice: ";
                        send(sock, a, strlen(a), 0);
                        read(sock, &opt, sizeof(opt));

                        switch (opt)
                        {
                        case 1:
                                add_cust(sock);
                                read(sock, &nop, sizeof(nop));
                                break;

                        case 2:
                                modify_customer_employee(sock);
                                read(sock, &nop, sizeof(nop));
                                break;

                        case 3:
                        {
                                a = "\n💰 Enter Loan ID and Decision (approve/reject): ";
                                send(sock, a, strlen(a), 0);
                                int loan_id;
                                read(sock, &loan_id, sizeof(int));
                                char decision[10];
                                read(sock, decision, sizeof(decision));

                                flag = approve_or_reject_loan(loan_id, whoami, decision);
                                a = (flag == 0) ? "✅ Operation Successful!\n" : "❌ Operation Failed!\n";
                                send(sock, a, strlen(a), 0);
                                read(sock, &nop, sizeof(nop));
                                break;
                        }

                        case 4:
                        {
                                char *assigned_loan = view_loans_assigned_to_employee(whoami);
                                send(sock, assigned_loan, strlen(assigned_loan) + 1, 0);
                                read(sock, &nop, sizeof(nop));
                                break;
                        }

                        case 5:
                        {
                                a = "\n👤 Enter Customer Username: ";
                                send(sock, a, strlen(a) + 1, 0);
                                char cuname[20];
                                read(sock, cuname, sizeof(cuname));
                                retrieve_transaction_history(sock, cuname);
                                read(sock, &nop, sizeof(nop));
                                break;
                        }

                        case 6:
                        {
                                a = "\n🔑 Enter New Password: ";
                                send(sock, a, strlen(a), 0);
                                char newpwd[20];
                                read(sock, newpwd, sizeof(newpwd));
                                int flag = change_password_user(whoami, newpwd, 2);
                                a = (flag == 0) ? "✅ Password Changed Successfully!\n"
                                                : "❌ Password Change Failed.\n";
                                send(sock, a, strlen(a), 0);
                                read(sock, &nop, sizeof(nop));
                                break;
                        }

                        case 7:
                                a = "\n👋 Logging Out... See you soon!\n";
                                send(sock, a, strlen(a), 0);
                                logout_user(uname);
                                read(sock, &nop, sizeof(nop));
                                break;
                        }

                        if (opt == 7)
                                break;
                }
        }
        else
        {
                a = "\n❌ LOGIN FAILED. PLEASE TRY AGAIN.\n";
                send(sock, a, strlen(a), 0);
        }
        return;
}

// =====================================================
// 👨‍💼 MANAGER PORTAL
// =====================================================
void manager(int sock)
{
        char *a = "\n===================================\n"
                  "👨‍💼  MANAGER LOGIN PORTAL\n"
                  "===================================\n"
                  "👉 ENTER USERNAME AND PASSWORD TO LOGIN:\n";
        int temp;
        send(sock, a, strlen(a), 0);

        char uname[1024], pwd[1024];
        read(sock, uname, 1024);
        read(sock, pwd, 1024);

        int flag = authenticate_user(uname, pwd, 3);
        send(sock, &flag, sizeof(int), 0);
        read(sock, &temp, sizeof(temp));
        int nop;

        if (flag == 1)
        {
                a = "\n✅ LOGIN SUCCESSFUL!\n"
                    "-----------------------------------\n"
                    "Welcome to Manager Dashboard 👋\n";
                send(sock, a, strlen(a), 0);
                read(sock, &temp, sizeof(temp));

                int opt = 0;

                while (1)
                {
                        a = "\n===================================\n"
                            "📋  MANAGER MENU\n"
                            "===================================\n"
                            "1️⃣  Assign Loan Applications to Employees\n"
                            "2️⃣  Review Customer Feedback\n"
                            "3️⃣  Change Password\n"
                            "4️⃣  Logout\n"
                            "-----------------------------------\n"
                            "👉 Enter your choice: ";
                        send(sock, a, strlen(a), 0);
                        read(sock, &opt, sizeof(opt));

                        switch (opt)
                        {
                        case 1:
                        {
                                char *unassigned_loans = view_unassigned_loans();
                                send(sock, unassigned_loans, strlen(unassigned_loans) + 1, 0);
                                read(sock, &opt, sizeof(opt));

                                a = "\n📦 Enter Loan ID and Employee ID to Assign:\n";
                                send(sock, a, strlen(a), 0);

                                int loan_id;
                                read(sock, &loan_id, sizeof(int));
                                char emp_id[20];
                                read(sock, emp_id, sizeof(emp_id));

                                flag = assign_loan_to_employee(loan_id, emp_id);
                                a = (flag == 0) ? "✅ Loan Assigned Successfully!\n"
                                                : "❌ Assignment Failed.\n";
                                send(sock, a, strlen(a), 0);
                                read(sock, &opt, sizeof(opt));
                                break;
                        }

                        case 2:
                        {
                                char getfeedback[1024];
                                get_feedbacks(getfeedback);
                                send(sock, getfeedback, strlen(getfeedback) + 1, 0);
                                read(sock, &nop, sizeof(nop));
                                break;
                        }

                        case 3:
                        {
                                a = "\n🔑 Enter New Password: ";
                                send(sock, a, strlen(a), 0);
                                char newpwd[20];
                                read(sock, newpwd, sizeof(newpwd));

                                flag = change_password_user(whoami, newpwd, 3);
                                a = (flag == 0) ? "✅ Password Changed Successfully!\n"
                                                : "❌ Password Change Failed.\n";
                                send(sock, a, strlen(a), 0);
                                read(sock, &nop, sizeof(nop));
                                break;
                        }

                        case 4:
                                a = "\n👋 Logging Out... See you soon!\n";
                                send(sock, a, strlen(a), 0);
                                logout_user(uname);
                                read(sock, &opt, sizeof(opt));
                                break;
                        }

                        if (opt == 4)
                                break;
                }
        }
        else
        {
                a = "\n❌ LOGIN FAILED. PLEASE TRY AGAIN.\n";
                send(sock, a, strlen(a), 0);
        }
        return;
}

void admin(int sock)
{
        char *a = "\n===================================\n"
                  "👑  ADMIN LOGIN PORTAL\n"
                  "===================================\n"
                  "👉 ENTER USERNAME AND PASSWORD TO LOGIN:\n";
        int temp;
        send(sock, a, strlen(a), 0);

        char uname[1024], pwd[1024];
        read(sock, uname, 1024);
        read(sock, pwd, 1024);

        int flag = authenticate_user(uname, pwd, 4);
        send(sock, &flag, sizeof(int), 0);
        read(sock, &temp, sizeof(temp));

        if (flag == 1)
        {
                a = "\n✅ LOGIN SUCCESSFUL!\n"
                    "-----------------------------------\n"
                    "Welcome to Admin Dashboard 👋\n";
                send(sock, a, strlen(a), 0);
                read(sock, &temp, sizeof(temp));

                int opt = 0;
                struct Employee emp;
                char menu[300];
                char msg[300];
                char emp_uname[20];

                while (1)
                {
                        a = "\n===================================\n"
                            "🧾  ADMIN MENU\n"
                            "===================================\n"
                            "1️⃣  Add New Employee\n"
                            "2️⃣  Modify Customer Details\n"
                            "3️⃣  Modify Employee Details\n"
                            "4️⃣  Manage Roles (Promote/Demote)\n"
                            "5️⃣  Change Password\n"
                            "6️⃣  Logout\n"
                            "-----------------------------------\n"
                            "👉 Enter your choice: ";
                        send(sock, a, strlen(a), 0);
                        read(sock, &opt, sizeof(opt));

                        int nop;

                        switch (opt)
                        {
                        // 🟢 Add New Employee
                        case 1:
                        {
                                strcpy(menu,
                                       "\n===================================\n"
                                       "➕  ADD NEW EMPLOYEE\n"
                                       "===================================\n"
                                       "Enter the following details:\n"
                                       "1️⃣ Username:\n"
                                       "2️⃣ Name:\n"
                                       "3️⃣ Role (Manager/Employee):\n"
                                       "4️⃣ Contact Number:\n"
                                       "5️⃣ Salary:\n");
                                send(sock, menu, strlen(menu), 0);

                                int num = 0;
                                read(sock, emp.username, sizeof(emp.username));
                                send(sock, &num, sizeof(int), 0);

                                read(sock, emp.name, sizeof(emp.name));
                                send(sock, &num, sizeof(int), 0);

                                read(sock, emp.role, sizeof(emp.role));
                                send(sock, &num, sizeof(int), 0);

                                read(sock, emp.phone, sizeof(emp.phone));
                                send(sock, &num, sizeof(int), 0);

                                read(sock, emp.salary, sizeof(emp.salary));
                                send(sock, &num, sizeof(int), 0);

                                flag = add_employee(emp);
                                strcpy(msg, (flag == 0)
                                                ? "✅ Employee Added Successfully!\n"
                                                : "❌ Failed to Add Employee.\n");
                                send(sock, msg, strlen(msg), 0);
                                read(sock, &opt, sizeof(opt));
                                break;
                        }

                        // 🟡 Modify Customer
                        case 2:
                                modify_customer_employee(sock);
                                read(sock, &nop, sizeof(nop));
                                break;

                        // 🟠 Modify Employee Details
                        case 3:
                        {
                                strcpy(msg, "\n👤 Enter Username to Update: ");
                                send(sock, msg, strlen(msg), 0);
                                read(sock, emp_uname, sizeof(emp_uname));

                                strcpy(menu,
                                       "\n===================================\n"
                                       "✏️  UPDATE EMPLOYEE DETAILS\n"
                                       "===================================\n"
                                       "1️⃣  Name\n"
                                       "2️⃣  Contact Number\n"
                                       "3️⃣  Salary\n"
                                       "-----------------------------------\n"
                                       "👉 Enter your choice (1-3): ");
                                send(sock, menu, strlen(menu), 0);

                                int menu_opt;
                                read(sock, &menu_opt, sizeof(int));
                                int flag_curr = -1;

                                switch (menu_opt)
                                {
                                case 1:
                                        strcpy(msg, "Enter new Name: ");
                                        send(sock, msg, strlen(msg), 0);
                                        read(sock, emp.name, sizeof(emp.name));
                                        flag_curr = update_employee_internal(emp_uname, emp.name, 2);
                                        break;

                                case 2:
                                        strcpy(msg, "Enter new Contact Number: ");
                                        send(sock, msg, strlen(msg), 0);
                                        read(sock, emp.phone, sizeof(emp.phone));
                                        flag_curr = update_employee_internal(emp_uname, emp.phone, 4);
                                        break;

                                case 3:
                                        strcpy(msg, "Enter new Salary: ");
                                        send(sock, msg, strlen(msg), 0);
                                        read(sock, emp.salary, sizeof(emp.salary));
                                        flag_curr = update_employee_internal(emp_uname, emp.salary, 5);
                                        break;
                                }

                                strcpy(msg, (flag_curr == 0)
                                                ? "✅ Update Successful!\n"
                                                : "❌ Update Failed!\n");
                                send(sock, msg, strlen(msg), 0);
                                read(sock, &nop, sizeof(nop));
                                break;
                        }

                        // 🔵 Manage Roles
                        case 4:
                        {
                                strcpy(msg,
                                       "\n===================================\n"
                                       "⚙️  MANAGE ROLES\n"
                                       "===================================\n"
                                       "1️⃣  Promote to Manager\n"
                                       "2️⃣  Demote to Employee\n"
                                       "👉 Enter your choice: ");
                                send(sock, msg, strlen(msg), 0);

                                int choice;
                                read(sock, &choice, sizeof(int));

                                strcpy(msg, "Enter Username: ");
                                send(sock, msg, strlen(msg), 0);
                                read(sock, emp_uname, sizeof(emp_uname));

                                flag = (choice == 1)
                                           ? add_manager_role(emp_uname)
                                           : remove_manager_role(emp_uname);

                                strcpy(msg, (flag == 0)
                                                ? "✅ Role Updated Successfully!\n"
                                                : "❌ Role Update Failed.\n");
                                send(sock, msg, strlen(msg), 0);
                                read(sock, &nop, sizeof(nop));
                                break;
                        }

                        // 🔑 Change Password
                        case 5:
                        {
                                a = "\n🔑 Enter New Password: ";
                                send(sock, a, strlen(a), 0);
                                char newpwd[20];
                                read(sock, newpwd, sizeof(newpwd));

                                flag = change_password_user(whoami, newpwd, 4);
                                a = (flag == 0)
                                        ? "✅ Password Changed Successfully!\n"
                                        : "❌ Password Change Failed!\n";
                                send(sock, a, strlen(a), 0);
                                read(sock, &nop, sizeof(nop));
                                break;
                        }

                        // 🚪 Logout
                        case 6:
                                a = "\n👋 Logging Out... See you again, Admin!\n";
                                send(sock, a, strlen(a), 0);
                                logout_user(uname);
                                read(sock, &opt, sizeof(opt));
                                break;
                        }

                        if (opt == 6)
                                break;
                }
        }
        else
        {
                a = "\n❌ LOGIN FAILED. PLEASE TRY AGAIN.\n";
                send(sock, a, strlen(a), 0);
        }
        return;
}

// =====================================================
// 🚪 LOGOUT & ERROR HANDLERS
// =====================================================
void logout(int sock)
{
        char msg[] = "\n👋 Bye Bye! Have a great day! 😊\n";
        send(sock, msg, strlen(msg), 0);
        return;
}

void otherwise(int sock)
{
        char msg[] = "\n⚠️ Invalid Choice! Please try again.\n";
        send(sock, msg, strlen(msg), 0);
        return;
}

// =====================================================
// 🔄 CLIENT HANDLER
// =====================================================
void *handle_client(void *socket_desc)
{
        int sock = *(int *)socket_desc;
        int choice = 0;
        int temp;

        char menu[] =
            "\n===================================\n"
            "🏦  BANKING SYSTEM LOGIN\n"
            "===================================\n"
            "1️⃣  Customer\n"
            "2️⃣  Employee\n"
            "3️⃣  Manager\n"
            "4️⃣  Admin\n"
            "5️⃣  Exit\n"
            "-----------------------------------\n"
            "👉 Enter Your Choice: ";

        while (1)
        {
                send(sock, menu, strlen(menu), 0);
                read(sock, &choice, sizeof(int));

                switch (choice)
                {
                case 1:
                        customer(sock);
                        read(sock, &temp, sizeof(int));
                        break;
                case 2:
                        employee(sock);
                        read(sock, &temp, sizeof(int));
                        break;
                case 3:
                        manager(sock);
                        read(sock, &temp, sizeof(int));
                        break;
                case 4:
                        admin(sock);
                        read(sock, &temp, sizeof(int));
                        break;
                case 5:
                        logout(sock);
                        read(sock, &temp, sizeof(int));
                        break;
                default:
                        otherwise(sock);
                        break;
                }

                if (choice == 5)
                        break;
        }

        close(sock);
        free(socket_desc);
        return NULL;
}