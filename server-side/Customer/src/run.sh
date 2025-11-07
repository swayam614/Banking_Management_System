gcc -c customer.c -I ../include -I ../../Utils/include -I ../../Admin/include -I ../../Handler/include -I ../../data_base *.c

ar rcs ../lib/libcustomer.a *.o