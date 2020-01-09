# Automatically clean and rebuild on makefile changes https://stackoverflow.com/a/5901262/871842
-include .makefile_date
.makefile_date : Makefile*
	@touch $@
	+$(MAKE) --no-print-directory clean

# CXX:=clang++
CXXOPT:=-g -std=c++2a -fPIC -Wall -Wno-unused-variable -Wfatal-errors -Werror -I"../cpp/include/"

ifneq (,$(findstring clang++, $(CXX)))
CXXOPT += --stdlib=libc++
CXXOPT += -I/usr/lib/llvm-10/include/c++/v1
# LDFLAGS += /usr/lib/llvm-10/lib/libc++.so /usr/lib/llvm-10/lib/libc++abi.so -L/usr/lib/llvm-10/lib
else ifneq (,$(findstring g++, $(CXX)))
CXXOPT += -Wno-unused-but-set-variable
endif

CATCH2_SINGLE_INCLUDE_FOLDER:=catch2/repo/single_include/catch2/
STUFF_FILES:=$(shell find ../cpp/ -type f -name '*.hpp' -o -name '*.h' -o -name '*.cpp')
CATCH2_HEADER_FILES:=$(shell find $(CATCH2_SINGLE_INCLUDE_FOLDER) -type f -name '*.hpp' -o -name '*.h')

runtests : testprog.exe
	./testprog.exe --abort

TEST_CPP_FILES = $(wildcard *.cpp)

ALL_OBJS = $(addprefix out/$(CXX)/, $(TEST_CPP_FILES:cpp=obj)) out/$(CXX)/catch2/catch2.obj

testprog.exe : $(ALL_OBJS)
	$(CXX) $(CXXOPT) $^ -o $@

out/$(CXX)/%.obj : %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXOPT) -c $< -o $@

clean :
	rm -rf out *.exe

# automatic dependencies
out/$(CXX)/%.d : %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXOPT) -MM -MF $@ -MT $(@:.d=.obj) -E $<

-include $(ALL_OBJS:%.obj=%.d)
