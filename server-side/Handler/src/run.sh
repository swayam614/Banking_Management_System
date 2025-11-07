gcc -c handler.c -I ../include -I ../../Customer/include -I ../../Employee/include -I ../../Manager/include -I ../../Admin/include -I ../../Utils/include -I ../../Loan/include *.c

ar rcs ../lib/libhandler.a *.o