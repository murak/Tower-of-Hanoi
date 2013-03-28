all:hanoi

hanoi: imageloader.o main.cpp
	g++ main.cpp imageloader.o -o tower-of-hanoi -l GL -l GLU -l glut

imageloader.o: imageloader.h imageloader.cpp
	g++ -c imageloader.h imageloader.cpp -l GL -l GLU -l glut

clean:
	rm -rf *.o hanoi
