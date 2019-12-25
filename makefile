all:
	g++ -std=c++0x disk.cpp -o disk.out
	g++ -std=c++0x process.cpp -o process.out
	g++ -std=c++0x kernel.cpp -o kernel.out