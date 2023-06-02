BOOST = /opt/homebrew/Cellar/boost/1.81.0_1/include
ASIO = /opt/homebrew/Cellar/asio/1.28.0/include

all:
	g++ -I $(BOOST) -I $(ASIO) main.cpp