gcc -c loan.c -I ../include -I ../../Manager/include -I ../../Utils/include -I ../../data_base *.c

ar rcs ../lib/libloan.a *.o