kernel_modules = ./src/modules
kernel = ./src/kernel
test_kernel = ./test/kernel
test_algo = ./test/algo

sub_dir =  ./src/*

.PHONY: build_concurrent build_hash build_hashtable build_memory build_all build_test_concurrent build_test_hashtable build_test_memory build_tests clean

build_concurrent:
	$(MAKE) -C ./src/concurrent all

build_hash:
	$(MAKE) -C ./src/hash all

build_hashtable:
	$(MAKE) -C ./src/hashtable all

build_memory:
	$(MAKE) -C ./src/memory all

build_all: build_concurrent build_hash build_hashtable build_memory

build_test_concurrent:
	$(MAKE) -C ./test/concurrent all

build_test_hashtable:
	$(MAKE) -C ./test/hashtable all

build_test_memory:
	$(MAKE) -C ./test/memory all

build_tests: build_test_concurrent build_test_hashtable build_test_memory

all: build_all build_tests

clean:
	for dir in $(sub_dir); do \
	  ($(MAKE) -C $$dir clean);\
	done
