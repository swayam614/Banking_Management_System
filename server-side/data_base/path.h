// #ifndef __PATH__
// #define __PATH__

// #define CUSTOMER_DATA_PATH "/Users/swayampalrecha/Desktop/My_Projects/Banking_Management_System/SS_Project-main/server-side/data_base/customer_data"

// #define EMPLOYEE_DATA_PATH "/Users/swayampalrecha/Desktop/My_Projects/Banking_Management_System/SS_Project-main/server-side/data_base/employee_data.csv"

// #define FEEDBACK_DATA_PATH "/Users/swayampalrecha/Desktop/My_Projects/Banking_Management_System/SS_Project-main/server-side/data_base/feedback.txt"

// #define LOAN_PATH "/Users/swayampalrecha/Desktop/My_Projects/Banking_Management_System/SS_Project-main/server-side/data_base/loans.txt"

// #define ADMIN_CREDENTIALS_DATA_PATH "/Users/swayampalrecha/Desktop/My_Projects/Banking_Management_System/SS_Project-main/server-side/data_base/credentials/admin_credentials"

// #define CUSTOMER_CREDENTIALS_DATA_PATH "/Users/swayampalrecha/Desktop/My_Projects/Banking_Management_System/SS_Project-main/server-side/data_base/credentials/customer_credentials"

// #define EMPLOYEE_CREDENTIALS_DATA_PATH "/Users/swayampalrecha/Desktop/My_Projects/Banking_Management_System/SS_Project-main/server-side/data_base/credentials/employee_credentials"

// #define MANAGER_CREDENTIALS_DATA_PATH "/Users/swayampalrecha/Desktop/My_Projects/Banking_Management_System/SS_Project-main/server-side/data_base/credentials/manager_credentials"

// #endif

#ifndef __PATH__
#define __PATH__

// Base directory for database files (relative to where the program runs)
#define BASE_PATH "./data_base"

// Individual paths relative to BASE_PATH
#define CUSTOMER_DATA_PATH BASE_PATH "/customer_data"
#define EMPLOYEE_DATA_PATH BASE_PATH "/employee_data.csv"
#define FEEDBACK_DATA_PATH BASE_PATH "/feedback.txt"
#define LOAN_PATH BASE_PATH "/loans.txt"

// Credentials folder
#define CREDENTIALS_PATH BASE_PATH "/credentials"
#define ADMIN_CREDENTIALS_DATA_PATH CREDENTIALS_PATH "/admin_credentials"
#define CUSTOMER_CREDENTIALS_DATA_PATH CREDENTIALS_PATH "/customer_credentials"
#define EMPLOYEE_CREDENTIALS_DATA_PATH CREDENTIALS_PATH "/employee_credentials"
#define MANAGER_CREDENTIALS_DATA_PATH CREDENTIALS_PATH "/manager_credentials"

#endif
