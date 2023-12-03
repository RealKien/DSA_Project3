#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <stack>

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
		int counter = 1; //Use for debugging, counter < 200 for fast debugging
		while (getline(file, line) && counter++< 100) {  // Start reading the first 1000 movies
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
					string temp = line.substr(2, line.size()-2);
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
		cout << "End of reading" << endl;
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
	void PrintResult(vector<string> IDs) {
		int counter = 1;
		
		cout << "Matching result: " << endl;
		for (auto id: IDs) {
			Movie movie = MovieGraph[id];
			cout << counter++ << ". ";
			cout << "ID: " << movie.ID << endl;
			cout << "Title: " << movie.title << endl;
			cout << "Year of release: " << movie.year_released << endl;
			cout << "Duration: " << movie.runtime << endl;
			cout << "Genre: " << movie.genre << endl;
			cout << endl;
			
			

		}
	}

	//Criteria: 1 - By movie name, 2 - By genre, 3 - By rating, 4 - By description 
	vector<string> dfs(unordered_map<string, bool>& visited, string keyword, int command) {

		// Create stack and pick first node
		stack<string> s;
		vector<string> result;
		// Search by movie name: Linear Search until having 5 movies matching the search Term or iterating all graph
		string startMovie = WeightedGraph.begin()->first;
		s.push(startMovie);
		int counter = 1;
		while (!s.empty() && result.size() <10) {
			string currentID = s.top();
			s.pop();
			// mark visited
			
			cout << "Repeat" << counter++ << endl;
		
			// Search changes based on which criteria to search
			switch (command) {
			case (1):  // Search by movie ID
				// Check search term in movie name
				if (currentID.find(keyword) != string::npos && !visited[currentID] && result.size() < 10) {  //true if match
					result.push_back(currentID);
					visited[currentID] = true;
				}
					
				for (auto similarMovie : WeightedGraph[currentID]) {
					
					if (similarMovie.first.find(keyword) != string::npos && WeightedGraph[currentID][similarMovie.first] >= 0.9 && result.size() < 10) {
						result.push_back(similarMovie.first);
					}
					// push unvisted
					if (!visited[similarMovie.first]) {
						s.push(similarMovie.first);
					}
					
				}
				break;
			case (2):  // Search by genre
				if (MovieGraph[currentID].genre.find(keyword) != string::npos && !visited[currentID] && result.size() < 10) {
					result.push_back(currentID);
					visited[currentID] = true;
				}
				for (auto similarMovie : WeightedGraph[currentID]) {
					
					if (MovieGraph[similarMovie.first].genre.find(keyword) != string::npos && WeightedGraph[currentID][similarMovie.first] >= 0.9 && result.size() < 10) {
						result.push_back(similarMovie.first);
					}
					// push unvisted
					if (!visited[similarMovie.first]) {
						s.push(similarMovie.first);
					}
		
				}
				break;
			case(3):  // Search by movie name
				if (MovieGraph[currentID].title.find(keyword) != string::npos && !visited[currentID] && result.size() < 10) {
					result.push_back(currentID);
					visited[currentID] = true;
				}
				for (auto similarMovie : WeightedGraph[currentID]) {
					
					if (MovieGraph[similarMovie.first].title.find(keyword) != string::npos && WeightedGraph[currentID][similarMovie.first] >= 0.9 && result.size() < 10) {
						// push unvisted
						if (!visited[similarMovie.first]) {
							result.push_back(similarMovie.first);
							s.push(similarMovie.first);
						}
					}
					
					
				}
				break;
			}
			
		}
		return result;
	}

	void dfsSearch(string keyword, int command) {
		unordered_map<string, bool> visited;
		// Initialize visited
		int v = WeightedGraph.size();
		for (auto ID : WeightedGraph) {
			visited[ID.first] = false;
		}
		//Perform dfs
		vector<string> matching = dfs(visited, keyword, command);
		cout << "Searching for " << keyword << endl;
		PrintResult(matching);
	}
};


