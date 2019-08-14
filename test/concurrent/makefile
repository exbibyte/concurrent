#include directories start---
inc_folder_basic := $(dir $(wildcard ../../src/*/))
inc_folder_test := ../

#common basic include folders
inc_folder_basic := $(sort $(dir $(wildcard ../../src/*/)))

include_dirs = $(inc_folder_basic) $(inc_folder_test)

#include directories end---

include_flags = $(foreach d, $(include_dirs), -I$d)

objects_concurrent = $(wildcard ../../src/concurrent/intermediate/*.o)
objects_core = $(wildcard ../../src/core/intermediate/*.o)

object_files = $(objects_concurrent) $(objects_core)

dir_build = build
$(shell mkdir -p $(dir_build))

targets := $(patsubst %.cpp, %, $(wildcard *.cpp))

libs := -pthread

%: %.cpp
	$(CXX) -g -std=c++17 -o $(dir_build)/$@ $^ $(object_files) $(include_flags) $(libs)

.PHONY: all
all: $(targets)

.PHONY: clean
clean:
	rm -f -r ./$(dir_build)

testcase_excludes := $(dir_build)/

testcases:
	@for i in $(filter-out $(testcase_excludes), $(wildcard $(dir_build)/*) ); do \
		./$$i; \
	done \
