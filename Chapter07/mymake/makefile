mymake : mymake.o makefileparser.o makefiletarget.o
	g++ -g -std=c++11 -o mymake mymake.o makefileparser.o makefiletarget.o
mymake.o : mymake.cpp
	g++ -g -std=c++11 -c mymake.cpp -o mymake.o
makefileparser.o : makefileparser.cpp
	g++ -g -std=c++11 -c makefileparser.cpp -o makefileparser.o
makefiletarget.o : makefiletarget.cpp
	g++ -g -std=c++11 -c makefiletarget.cpp -o makefiletarget.o
clean :
	rm -rf mymake mymake.o makefileparser.o makefiletarget.o
.PHONY : clean