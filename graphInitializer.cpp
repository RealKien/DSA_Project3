#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <stack>
#include <queue>
#include <iomanip>

using namespace std;

struct Movie {
	string ID;
	string title;
	string year_released;
	string runtime;
	string genre;
	vector<double> title_embedding;
};

class Graph {
private:
	unordered_map<string, Movie> MovieGraph;  // Graph contain all Movie's properties
	unordered_map<string, unordered_map<string, double>> WeightedGraph;  // Graph contains Movies with vector embeddings
	

	vector<string> movieIDs;
	vector<vector<double>> movieNameEmbeddings;
	vector<Movie> movieContainer;
public:
	Graph() {};
	unordered_map<string, unordered_map<string, double>> getWeightedGraph() { return WeightedGraph; };
	unordered_map<string, Movie> getMovieGraph() { return MovieGraph; }
	void ReadFile() {
		ifstream file("./imdb_movies_with_embeddings.csv");
		if (!file.is_open()) cout << "Error opening file! " << endl;

		string line;
		string token;
		bool flag = false;
		int counter = 1;
		while (getline(file, line) && counter++ < 1500) {  // Start reading the first 1500 movies ~ 600k edges
			flag = false;
			Movie movie;
			if (getline(file, line, ',')) {
				//getline(file, line, ',');  // Read ID
				movie.ID = line;

				getline(file, movie.title, ',');  // Read title
				while (!flag) {  // Edge case if title contains commas

					flag = true;
					if (getline(file, token, ',')) {
						for (char character : token) {

							if (token == "\\N") { movie.year_released += token; break; }  // Edge case if year is unknown
							else if (character < 48 || character > 57 || token[0] == '0') {  // Check if year contains letters
								flag = false; movie.title += token;  break;  // Add the rest to the title
							}
							else {
								movie.year_released += character;
							}
						}
					}
					if (flag) { break; }  // Break to read the correct year
				}

				getline(file, movie.runtime, ',');  // read runtime
				getline(file, movie.genre, ',');   // read genre
				// get vector embeddings string
				if (getline(file, line, ']')) {
					string temp = line.substr(2, line.size() - 2);
					istringstream  iss2(temp);
					double value;
					while (iss2 >> value) {  //stream each value to vector

						movie.title_embedding.push_back(value);
					}
				}
				//for (auto i : movie.title_embedding) cout << i << " " << endl;
				// Add to containers to prepare graph creating
				movieContainer.push_back(movie);
				movieIDs.push_back(movie.ID);
				movieNameEmbeddings.push_back(movie.title_embedding);
			}
		}
		file.close();
	}

	// Helper function to determine the similarity of two movies
	// cos = v1 dot v2 / (v1 * v2)
	double cosineSimilarity(vector<double>& vec1, vector<double>& vec2) {
		if (vec1.size() != vec2.size()) return 0.0;
		double dotProduct = 0.0;
		double mag1 = 0.0;
		double mag2 = 0.0;
		for (

			auto i = 0; i < vec1.size(); i++) {
			dotProduct += vec1[i] * vec2[i];
			mag1 += vec1[i] * vec1[i];
			mag2 += vec2[i] * vec2[i];
		}
		if (mag1 == 0 || mag2 == 0) return -1; // Error
		return (dotProduct / (sqrt(mag1) * sqrt(mag2)));
	}

	void CreateGraph() {  // Adjacency Matrix
		int counter = 1;
		//cout << "Length" << movieIDs.size() << endl;
		for (size_t i = 0; i < movieIDs.size(); ++i) {
			for (size_t j = i + 1; j < movieIDs.size(); ++j) {

				double similarity = cosineSimilarity(movieNameEmbeddings[i], movieNameEmbeddings[j]);  // Check for similarity

				if (similarity >= 0.5) { // 0.5 = threshold --> Add edges
					WeightedGraph[movieIDs[i]][movieIDs[j]] = similarity;
					WeightedGraph[movieIDs[j]][movieIDs[i]] = similarity;
				}
			}

			MovieGraph[movieIDs[i]] = movieContainer[i];  // Add to movie container

		}


	}
	// For debugging/ Printing later: Only print one by one
	void PrintResult(vector<string> dfs, vector<string> bfs) {
		
		
		cout << string(30 * 3, '-') << endl;
		for (int counter = 1; counter <= dfs.size(); counter++) {
			Movie moviedfs = MovieGraph[dfs[counter-1]];
			Movie moviebfs = MovieGraph[bfs[counter-1]];
			cout << "ID:            " << moviedfs.ID << setw(41-moviedfs.ID.length()) << "|  "  <<  moviebfs.ID << endl;
			// If dfs & bfs name too long
			if (moviedfs.title.length() >= 30 && moviebfs.title.length() >= 30) {
				cout << "Title:         " << moviedfs.title.substr(0, 30) << "..." << setw(8) << "|  " << moviebfs.title.substr(0, 30) << "..." << endl;
			}
			// if dfs name too long
			else if (moviedfs.title.length() >= 30){
				cout << "Title:         " << moviedfs.title.substr(0, 30) << "..." << setw(8) << "|  " << moviebfs.title << endl;
			}
			// If bfs name too long
			if (moviebfs.title.length() >= 30) {
				cout << "Title:         " << moviedfs.title << setw(41 - moviedfs.title.length()) << "|  " << moviebfs.title.substr(0, 30) << "..." << endl;
			}
			else {
				cout << "Title:         " << moviedfs.title << setw(41 - moviedfs.title.length()) << "|  " << moviebfs.title << endl;
			}

			cout << "Release year:  "  << moviedfs.year_released << setw(41 - moviedfs.year_released.length()) << "|  " << moviebfs.year_released << endl;
			cout << "Duration:      " << moviedfs.runtime << setw(41 - moviedfs.runtime.length()) << "|  " << moviebfs.runtime << endl;
			cout << "Genre:         " << moviedfs.genre << setw(41 - moviedfs.genre.length()) << "|  " << moviebfs.genre << endl;
			cout << endl;

		}
	}

	//Criteria: 1 - By ID, 2 - By genre, 3 - By name
	// Maximum return results is 10
	// dfs algorithm
	vector<string> dfs(unordered_map<string, bool>& visited, string keyword, int command) {
		// Create stack and pick first node
		stack<string> s;
		vector<string> result;
		int keyLength = keyword.length();
		// Search by movie name: Linear Search until having 5 movies matching the search Term or iterating all graph
		string startMovie = WeightedGraph.begin()->first;
		s.push(startMovie);
		int counter = 1;

		while (!s.empty() && result.size() < 10) {
			string currentID = s.top();
			s.pop();

			counter++;

			// Search changes based on which criteria to search
			switch (command) {
			case (1):  // Search by movie ID
				// Check search term in movie name
				if (!visited[currentID]) {
					for(int i = 0; i < keyLength/2; i++) {  // If part of the keyword match the criteria --> still recommend
						if (currentID.find(keyword.substr(i, keyLength - i)) != string::npos && !visited[currentID] && result.size() < 10) {  //true if match
							result.push_back(currentID);
							break;
						}
					}
					visited[currentID] = true;  // mark visited
					for (auto similarMovie : WeightedGraph[currentID]) {
						if (WeightedGraph[currentID][similarMovie.first] >= 0.9 && result.size() < 10) {
							// push unvisted similar movies
							if (!visited[similarMovie.first]) {
								s.push(similarMovie.first);
							}
						}
					}
				}
				break;
			case (2):  // Search by genre
				if (!visited[currentID]) {
					// If genre matches
					for(int i = 0; i < keyLength/2; i++) {  // If part of the keyword match the criteria --> still recommend
						if (MovieGraph[currentID].genre.find(keyword.substr(i, keyLength - i)) != string::npos && !visited[currentID] && result.size() < 10) {
							result.push_back(currentID);
							break;
						}
					}
					visited[currentID] = true;  // mark visited
					for (auto similarMovie : WeightedGraph[currentID]) {
						if (WeightedGraph[currentID][similarMovie.first] >= 0.9 && result.size() < 10) {
							// push unvisted similar movies
							if (!visited[similarMovie.first]) {
								s.push(similarMovie.first);
							}
						}
					}
				}
				break;
			case(3):  // Search by movie name
				if (!visited[currentID]) {
					for(int i = 0; i < keyLength/2; i++) {  // If part of the keyword match the criteria --> still recommend
						if (MovieGraph[currentID].title.find(keyword.substr(i, keyLength - i)) != string::npos && result.size() < 10) {
							result.push_back(currentID);
							break;
						}
					}
					visited[currentID] = true;// mark visited
					for (auto similarMovie : WeightedGraph[currentID]) {

						if (WeightedGraph[currentID][similarMovie.first] >= 0.9 && result.size() < 10) {
							// push unvisted similar movies
							if (!visited[similarMovie.first]) {						
								s.push(similarMovie.first);
							}
						}
					}
				}
				break;
			}

		}
		cout << "Number of searches done:" << counter << setw(32 - to_string(counter).length()) << "|  ";  // Print the number of calling dfs
		return result;
	}

	
	//Criteria: 1 - By ID, 2 - By genre, 3 - By name
	// Maximum return result is 10
	vector<string> bfs(unordered_map<string, bool>& visited, string keyword, int command) {
		int keyLength = keyword.length();
		// Create stack and pick first node
		queue<string> q;
		vector<string> result;
		// Search by movie name: Linear Search until having 5 movies matching the search Term or iterating all graph
		string startMovie = WeightedGraph.begin()->first;
		q.push(startMovie);
		int counter = 1;
		while (!q.empty() && result.size() < 10) {
			string currentID = q.front();
			q.pop();
			counter++;
			// Search changes based on which criteria to search
			switch (command) {
			case (1):  // Search by movie ID
				// Check search term in movie name
				if (!visited[currentID]) {
					for(int i = 0; i < keyLength/2; i++) {  // If part of the keyword match the criteria --> still recommend
						if (currentID.find(keyword.substr(i, keyLength - i)) != string::npos && !visited[currentID] && result.size() < 10) {  //true if match
							result.push_back(currentID);
							break;
						}
					}
					visited[currentID] = true;  // mark visited
					for (auto similarMovie : WeightedGraph[currentID]) {
						if (WeightedGraph[currentID][similarMovie.first] >= 0.9 && result.size() < 10) {
							// push unvisted similar movies
							if (!visited[similarMovie.first]) {							
								q.push(similarMovie.first);
							}
						}
					}
				}
				break;
			case (2):  // Search by genre
				if (!visited[currentID]) {
					for(int i = 0; i < keyLength/2; i++) {  // If part of the keyword match the criteria --> still recommend
						if (MovieGraph[currentID].genre.find(keyword.substr(i, keyLength - i)) != string::npos && !visited[currentID] && result.size() < 10) {
							result.push_back(currentID);
							break;
						}
					}
					visited[currentID] = true;  // mark visited
					for (auto similarMovie : WeightedGraph[currentID]) {
						if (WeightedGraph[currentID][similarMovie.first] >= 0.9 && result.size() < 10) {
							// push unvisted similar movies
							if (!visited[similarMovie.first]) {
								q.push(similarMovie.first);
							}
						}
					}
				}
				break;
			case(3):  // Search by movie name
				if (!visited[currentID]) {
					
					for(int i = 0; i < keyLength/2; i++) {  // If part of the keyword match the criteria --> still recommend
						if (MovieGraph[currentID].title.find(keyword.substr(i, keyLength - i)) != string::npos && result.size() < 10) {  // If match
							result.push_back(currentID);
							break;
						}
					}
					visited[currentID] = true;  // mark visited
					for (auto similarMovie : WeightedGraph[currentID]) {
						if (WeightedGraph[currentID][similarMovie.first] >= 0.9 && result.size() < 10) {
							// push unvisted similar movies
							if (!visited[similarMovie.first]) {
								q.push(similarMovie.first);
							}
						}
					}
				}
				break;
			}
		}
		cout << "Number of searches done: " << counter << endl;  // Print the number of doing bfs
		return result;
	}

	void Search(string keyword, int command) {
		unordered_map<string, bool> visiteddfs;
		unordered_map<string, bool> visitedbfs;
		// Initialize visited
		int v = WeightedGraph.size();
		for (auto ID : WeightedGraph) {
			visitedbfs[ID.first] = false;
			visiteddfs[ID.first] = false;
		}
		//Perform dfs
		cout << "Matching using dfs..." << setw(35) << "|  " << "Matching using bfs..." << setw(24) << endl;  // Header
		vector<string> matchingdfs = dfs(visiteddfs, keyword, command);  // Perform dfs
		vector<string> matchingbfs = bfs(visitedbfs, keyword, command);  // Perform bfs
		if (matchingdfs.size() == 0) cout << "No matching result!" << endl;
		PrintResult(matchingdfs, matchingbfs);  // Print result
		
	}
};
