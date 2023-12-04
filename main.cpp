#include "graphInitializer.cpp"

// Simple main
int main() {
	Graph movie;
	movie.ReadFile();
	movie.CreateGraph();
	char userMainSelect;
	int userSelectionNum;
	string userInput;
	while (true) {
		cout << "Welcome to CineGenius!" << endl << "Find your next favourite movie within our catalog of over 20k movies." << endl << "------------------------------------------------------------------------------------" << endl;
		cout << "Type 1 to start/continue, 2 to exit: " << endl;
		cin >> userMainSelect;
		if (userMainSelect == '2')
		{
			cout << "Goodbye! " << endl;
			break;
		}
		cout << "Recommender Menu" << endl << "Select a filter to search our dataset and we'll share our top 5 similar movies!" << endl;
		cout << "Type 1 to search by IMDB movie ID, 2 to search by genre, and 3 to search by  movie name: ";
		cin >> userSelectionNum;

		if (userSelectionNum == 1)
		{
			cout << "Enter your IMDB movie ID: ";
		}
		else if (userSelectionNum == 2)
		{
			cout << "Enter your genre: ";
		}
		else if (userSelectionNum == 3)
		{
			cout << "Enter your movie name: ";
		}
		else {  // Invalid
			cout << "Invalid choice! Please try again!";
			continue;
		}
		cin >> userInput;

		movie.Search(userInput, userSelectionNum);
	}
}
