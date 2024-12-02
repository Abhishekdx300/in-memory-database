run:
	@g++ -O2 avl.cpp hashtable.cpp heap.cpp thread_pool.cpp zset.cpp serveer.cpp -o server
	@g++ clientt.cpp -o client