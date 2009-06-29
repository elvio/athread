all:
	mpicc -o split_vector split_vector.c -lathread -lmpi
	mpicc -o big_foot big_foot.c -lathread -lmpi
  
clean:
	rm -f split_vector
	rm -f big_foot