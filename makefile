SPS: main.cpp
	g++ -std=c++17 -DLINUX -o SPS main.cpp -I. -I../libnodave -I../asio/asio/include -I../rapidjson/include -L../libnodave -l:openSocket.o -l:setport.o -l:nodave.o  -lpthread -g -Wall -Wextra
