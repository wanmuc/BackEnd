mySort : main.o sort.o print.o  
	gcc -g -o mySort main.o sort.o print.o  
main.o : main.c  
	gcc -g -c main.c -o main.o  
sort.o : sort.c  
	gcc -g -c sort.c -o sort.o  
print.o : print.c  
	gcc -g -c print.c -o print.o  
clean :  
	rm -rf mySort main.o sort.o print.o  
.PHONY : clean

