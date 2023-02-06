test_assign_1:test_assign1_1.c dberror.o storage_mgr.o
	gcc -o test_assign_1 test_assign1_1.c dberror.o storage_mgr.o

dberror.o:dberror.c
	gcc -c dberror.c
	
storage_mgr.o:storage_mgr.c
	gcc -c storage_mgr.c

run:test_assign_1
	./test_assign_1
	
clean:
	rm *.o test_assign_1
