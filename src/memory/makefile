DEBUG ?= 1
ifeq ($(DEBUG), 1)
    FLAGS=-g
else
    FLAGS=-O3
endif
#source files---

src_files = $(wildcard *.cpp)

#source files end---

#include directories start---
inc_folder_core := ../core ../render ../gl ../file ../asset ../math

include_dirs = $(inc_folder_core)

#include directories end---

include_flags = $(foreach d, $(include_dirs), -I$d)

objects := $(patsubst %.cpp,%.o,$(src_files))

dir_intermediate := ./intermediate

$(shell mkdir -p $(dir_intermediate))

%.o: %.cpp
	$(CXX) $(FLAGS) -std=c++1z -c -o $(dir_intermediate)/$@ $< $(include_flags)

.PHONY: all
all: $(objects)

.PHONY: clean
clean:
	rm -f $(dir_intermediate)/*.o 

