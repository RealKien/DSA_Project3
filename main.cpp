#include "graphInitializer.cpp"

// Simple main
int main() {
	Graph movie;
	movie.ReadFile();
	movie.CreateGraph();
	char userSelectionNum;
	string userInput;
	cout << "Welcome to CineGenius!" << endl << "Find your next favourite movie within our catalog of over 20k movies." << endl << "------------------------------------------------------------------------------------" << endl;
	cout << "Recommender Menu" << endl << "Select a filter to search our dataset and we'll share our top 5 similar movies!" << endl;
	cout << "Type 1 to search by IMDB movie ID, 2 to search by genre, and 3 to search by  movie name: ";
	cin >> userSelectionNum;

	if (userSelectionNum == '1')
	{
		cout << "Enter your IMDB movie ID: ";
	}
	else if (userSelectionNum == '2')
	{
		cout << "Enter your genre: ";
	}
	else if (userSelectionNum == '3')
	{
		cout << "Enter your movie name: ";
	}
	cin >> userInput;

	movie.bfsSearch(userInput, userSelectionNum);
	movie.dfsSearch(userInput, userSelectionNum);
}