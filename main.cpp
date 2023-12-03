#include "GraphBuilding.cpp"

// Simple main
int main() {
	Graph movie;
	movie.ReadFile();
	movie.CreateGraph();
	movie.dfsSearch("Home", 3);
	movie.dfsSearch("ttt03", 1);
	movie.dfsSearch("Drama", 2);
}
