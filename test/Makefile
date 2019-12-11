# CXX:=clang++
CXXOPT:=-g -std=c++2a -fPIC

CATCH2_SINGLE_INCLUDE_FOLDER:=catch2/repo/single_include/catch2/
STUFF_FILES:=$(shell find ../cpp/ -type f -name '*.hpp' -o -name '*.h' -o -name '*.cpp')
CATCH2_HEADER_FILES:=$(shell find $(CATCH2_SINGLE_INCLUDE_FOLDER) -type f -name '*.hpp' -o -name '*.h')

runtests : testprog.exe
	./testprog.exe

TEST_CPP_FILES=$(shell find . -maxdepth 1 -type f -name '*.cpp')
TEST_CPP_OBJ_FILES = $(TEST_CPP_FILES:cpp=obj)

testprog.exe : $(TEST_CPP_OBJ_FILES) catch2/catch2.obj
	$(CXX) $(CXXOPT) $^ -o $@

%.obj : %.cpp $(STUFF_FILES) $(CATCH2_HEADER_FILES) Makefile
	$(CXX) $(CXXOPT) -c $< -o $@
catch2/catch2.obj : catch2/*.cpp $(CATCH2_HEADER_FILES) Makefile
	$(CXX) $(CXXOPT) -c catch2/*.cpp -o catch2/catch2.obj

clean :
	rm -rf *.obj *.exe catch2/*.obj *.pdb *.ilk *.gcno

