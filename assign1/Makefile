testassign1:test_assign1_1.c dberror.o storage_mgr.o
	gcc -o testassign1 test_assign1_1.c storage_mgr.o dberror.o

storage_mgr.o:storage_mgr.c
	gcc -c storage_mgr.c
dberror.o:dberror.c
	gcc -c dberror.c
run: testassign1
	./testassign1
	
clean:
		rm *.o testassign1