gcc -c employee.c -I ../include -I ../../Admin/include -I ../../Utils/include -I ../../data_base *.c

ar rcs ../lib/libemployee.a *.o