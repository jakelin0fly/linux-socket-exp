#main: main_client.cpp client.cpp
#	g++ -o client main_client.cpp client.cpp -pthread

# OBJ:main_client.o client.o

# main: $(OBJ)
# 	g++ -o client $(OBJ)

# main_client: main_client.cpp
# 	g++ -o main_client.o main_client.cpp

# client: client.cpp
# 	g++ -std=c++11 -o client.o client.cpp  -pthread

# clean: 
# 	rm edit main_client.o client.o

all:main_client.cpp client.cpp
	g++ -o client main_client.cpp client.cpp -pthread