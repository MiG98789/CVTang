ALL: Main

Main:
	g++ -g -std=c++11 Core/* FibonacciHeap/fibheap.cpp -o main `pkg-config --cflags --libs opencv` && ./main

Clean:
	rm main
