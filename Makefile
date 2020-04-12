CXX = g++
CXXFLAGS = -Wall -fPIC
ROOTCONFIG = `root-config --cflags --glibs`
SOURCE = virus.cxx
TARGET = virus
DEBUG = -g

$(TARGET): $(SOURCE)
	$(CXX) -o $(TARGET) $(SOURCE) $(CXXFLAGS) $(ROOTCONFIG) 

debug:
	$(CXX) -o $(TARGET) $(SOURCE) $(CXXFLAGS) $(ROOTCONFIG) $(DEBUG)

clean:
	rm -rf virus gif_files/* virus.dSYM
