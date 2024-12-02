run:
	@g++ -O2 avl.cpp hashtable.cpp heap.cpp thread_pool.cpp zset.cpp server.cpp -o server
	@g++ client.cpp -o client