all:
	for number in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15; do \
		g++ test$$number.cpp -pthread -o test$$number.out; \
	done

clean: 
	rm -rf *.out *.o *.core *~
