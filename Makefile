project3:
	g++ -w dchat.cpp server_udp.cpp -o dchat -lpthread -std=c++11
fair:
	g++ -w dchat_fairQ.cpp server_udp.cpp -o dchat -lpthread -std=c++11
encrypt:
	g++ -w dchat_encrypt.cpp server_udp.cpp -o dchat -lpthread -std=c++11	
clean:
	rm dchat
