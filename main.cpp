#include "graphInitializer.cpp"
#include <string>
#include <iostream>

using namespace std;

// Simple main
int main() {
	Graph movie;
	char userMainSelect;
	int userSelectionNum;
	string userInput;
	while (true) {
		cout << "Welcome to CineGenius!" << endl << "Find your next favourite movie within our catalog of over thousands of movies." << endl << "------------------------------------------------------------------------------------" << endl;
		cout << "Type 1 to start/continue, other key to exit: ";
		cin >> userMainSelect;
		cout << endl;
		if (userMainSelect == '2')
		{
			cout << "Goodbye! " << endl;
			break;
		}
		cout << "Recommender Menu" << endl << "Select a filter to search our dataset and we'll share our top 10 similar movies!" << endl;
		cout << "Type: " << endl;;
		cout << "1 to search by IMDB movie ID \n2 to search by genre \n3 to search by movie name\n";
		cout << "Enter your choice: ";
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
		getline(cin >> ws, userInput);
		cout << "Searching \"" << userInput << "\"...\n\n";

		movie.Search(userInput, userSelectionNum);
	}
}
