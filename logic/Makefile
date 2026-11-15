# Build:  make
# Clean:  make clean
# Without make:
#   g++ -std=c++17 -Wall main.cpp RoutePlanner.cpp Problem.cpp InputReader.cpp SweepAlgorithm.cpp KMeansAlgorithm.cpp ClarkeWrightAlgorithm.cpp InsertionAlgorithm.cpp Improvement.cpp Display.cpp -o wasteflow

CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

SRC = main.cpp RoutePlanner.cpp Problem.cpp InputReader.cpp \
      SweepAlgorithm.cpp KMeansAlgorithm.cpp ClarkeWrightAlgorithm.cpp \
      InsertionAlgorithm.cpp Improvement.cpp Display.cpp

wasteflow: $(SRC) $(wildcard *.hpp)
	$(CXX) $(CXXFLAGS) $(SRC) -o wasteflow

clean:
	rm -f wasteflow
