gcc -c utils.c -I ../include -I ../../Handler/include -I ../../ -I ../../data_base/ *.c

ar rcs ../lib/libutils.a *.o