gcc -c admin.c -I ../include -I ../../Utils/include -I ../../Employee/include -I ../../Handler/include -I ../../data_base *.c

ar rcs ../lib/libadmin.a *.o