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
#include <chrono>

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

	void ReadFile() {
		ifstream file("./imdb_movies_with_embeddings.csv");
		if (!file.is_open()) cout << "Error opening file! " << endl;

		string line;
		string token;
		bool flag = false;
		int counter = 1;
		while (getline(file, line) && counter++ < 2000) {  // Start reading the first 2000 movies ~ 1M edges
			flag = false;
			Movie movie;
			if (getline(file, line, ',')) {
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

	void CreateGraph() { 
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

	//Criteria: 1 - By ID, 2 - By genre, 3 - By name
	// Maximum return results is 10
	// dfs algorithm
	unordered_map<string, vector<string>> dfs(unordered_map<string, bool>& visited, string keyword, int command) {
		// Create stack and pick first node
		stack<string> s;
		unordered_map<string, vector<string>> result;
		
		int keyLength = keyword.length();
		// Search by movie name: Linear Search until having 5 movies matching the search Term or iterating all graph
		string startMovie = WeightedGraph.begin()->first;
		s.push(startMovie);
		int counter = 1;
		int matched = 1;  // Number of matching movies so far
		while (!s.empty() && matched < 10) {
			string currentID = s.top();
			s.pop();
			counter++;
			// Search changes based on which criteria to search
			switch (command) {
			case (1):  // Search by movie ID
				// Check search term in movie name
				if (!visited[currentID]) {
					if (currentID.find(keyword) != string::npos && !visited[currentID] && matched < 10) {  // if key match completely
						result[keyword].push_back(currentID);
						matched++;
					}
					else {
						for (int i = 1; i < keyLength / 2; i++) {  // If part of the keyword match the criteria
							string key = keyword.substr(i, keyLength - i);
							if (currentID.find(key) != string::npos && !visited[currentID] && result.size() < 5) {  //true if match
								result[key].push_back(currentID);
								break;
							}
						}
					}
					visited[currentID] = true;  // mark visited
					for (auto similarMovie : WeightedGraph[currentID]) {
						if (WeightedGraph[currentID][similarMovie.first] >= 0.9 && matched < 10) {
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
					if (MovieGraph[currentID].genre.find(keyword) != string::npos && !visited[currentID] && matched < 10) {  // if key match completely
						result[keyword].push_back(currentID);
						matched++;
					}
					else {
						for (int i = 1; i < keyLength / 2; i++) {  // If part of the keyword match the criteria --> still recommend
							string key = keyword.substr(i, keyLength - i);
							if (MovieGraph[currentID].genre.find(key) != string::npos && !visited[currentID] && result.size() < 5) {
								result[key].push_back(currentID);
								break;
							}
						}
					}
					visited[currentID] = true;  // mark visited
					for (auto similarMovie : WeightedGraph[currentID]) {
						if (WeightedGraph[currentID][similarMovie.first] >= 0.9 && matched < 10) {
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
					if (MovieGraph[currentID].title.find(keyword) != string::npos && !visited[currentID] && matched < 10) {  // if key match completely
						result[keyword].push_back(currentID);
						matched++;
					}
					else {
						for (int i = 1; i < keyLength / 2; i++) {  // If part of the keyword match the criteria --> still recommend
							string key = keyword.substr(i, keyLength - i);
							if (MovieGraph[currentID].title.find(key) != string::npos && result.size() < 5) {
								result[key].push_back(currentID);
								break;
							}
						}
					}
					visited[currentID] = true;// mark visited
					for (auto similarMovie : WeightedGraph[currentID]) {

						if (WeightedGraph[currentID][similarMovie.first] >= 0.9 && matched < 10) {
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
	unordered_map<string,vector<string>> bfs(unordered_map<string, bool>& visited, string keyword, int command) {
		int keyLength = keyword.length();
		// Create stack and pick first node
		queue<string> q;
		unordered_map<string, vector<string>> result;
		// Search by movie name: Linear Search until having 5 movies matching the search Term or iterating all graph
		string startMovie = WeightedGraph.begin()->first;
		q.push(startMovie);
		int counter = 1;
		int matched = 1; //number of matching movies so far
		while (!q.empty() && matched < 10) {
			string currentID = q.front();
			q.pop();
			counter++;
			// Search changes based on which criteria to search
			switch (command) {
			case (1):  // Search by movie ID
				// Check search term in movie name
				if (!visited[currentID]) {
					if (currentID.find(keyword) != string::npos && !visited[currentID] && matched < 10) {  // if key match completely
						result[keyword].push_back(currentID);
						matched++;
					}
					else {
						for (int i = 1; i < keyLength / 2; i++) {  // If part of the keyword match the criteria --> still recommend
							string key = keyword.substr(i, keyLength - i);
							if (currentID.find(key) != string::npos && !visited[currentID] && result.size() < 5) {  //true if match
								result[key].push_back(currentID);
								break;
							}
						}
					}
					visited[currentID] = true;  // mark visited
					for (auto similarMovie : WeightedGraph[currentID]) {
						if (WeightedGraph[currentID][similarMovie.first] >= 0.9 && matched < 10) {
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
					if (MovieGraph[currentID].genre.find(keyword) != string::npos && !visited[currentID] && matched < 10) {  // if key match completely
						result[keyword].push_back(currentID);
						matched++;
					}
					else {
						for (int i = 1; i < keyLength / 2; i++) {  // If part of the keyword match the criteria --> still recommend
							string key = keyword.substr(i, keyLength - i);
							if (MovieGraph[currentID].genre.find(key) != string::npos && !visited[currentID] && result.size() < 5) {
								result[key].push_back(currentID);
								break;
							}
						}
					}
					visited[currentID] = true;  // mark visited
					for (auto similarMovie : WeightedGraph[currentID]) {
						if (WeightedGraph[currentID][similarMovie.first] >= 0.9 && matched < 10) {
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
					if (MovieGraph[currentID].title.find(keyword) != string::npos && !visited[currentID] && matched < 10) {  // if key match completely
						result[keyword].push_back(currentID);
						matched++;
					}
					else {
						for (int i = 1; i < keyLength / 2; i++) {  // If part of the keyword match the criteria --> still recommend
							string key = keyword.substr(i, keyLength - i);
							if (MovieGraph[currentID].title.find(key) != string::npos && result.size() < 5) {  // If match
								result[key].push_back(currentID);
								break;
							}
						}
					}
					visited[currentID] = true;  // mark visited
					for (auto similarMovie : WeightedGraph[currentID]) {
						if (WeightedGraph[currentID][similarMovie.first] >= 0.9 && matched < 10) {
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

	// Print results after searching
	void Print(vector<string> dfs, vector<string> bfs, int& order) {
		
		for (int i = 1; i < dfs.size() + 1; i++) {
			if (order++ > 10) { break; }
			Movie moviedfs = MovieGraph[dfs[i - 1]];
			Movie moviebfs = MovieGraph[bfs[i - 1]];
			cout << "ID:            " << moviedfs.ID << setw(41 - moviedfs.ID.length()) << "|  " << moviebfs.ID << endl;
			// If dfs & bfs title too long
			if (moviedfs.title.length() >= 30 && moviebfs.title.length() >= 30) {
				cout << "Title:         " << moviedfs.title.substr(0, 30) << "..." << setw(8) << "|  " << moviebfs.title.substr(0, 30) << "..." << endl;
			}
			// if dfs title too long
			else if (moviedfs.title.length() >= 30) {
				cout << "Title:         " << moviedfs.title.substr(0, 30) << "..." << setw(8) << "|  " << moviebfs.title << endl;
			}
			// If bfs title too long
			if (moviebfs.title.length() >= 30) {
				cout << "Title:         " << moviedfs.title << setw(41 - moviedfs.title.length()) << "|  " << moviebfs.title.substr(0, 30) << "..." << endl;
			}
			else {
				cout << "Title:         " << moviedfs.title << setw(41 - moviedfs.title.length()) << "|  " << moviebfs.title << endl;
			}

			cout << "Release year:  " << moviedfs.year_released << setw(41 - moviedfs.year_released.length()) << "|  " << moviebfs.year_released << endl;
			cout << "Duration:      " << moviedfs.runtime << setw(41 - moviedfs.runtime.length()) << "|  " << moviebfs.runtime << endl;
			cout << "Genre:         " << moviedfs.genre << setw(41 - moviedfs.genre.length()) << "|  " << moviebfs.genre << endl;
			cout << endl;
		}
		
	}
	void PrintResult(unordered_map<string,vector<string>> dfs, unordered_map<string,vector<string>> bfs, string keyword) {
		cout << string(30 * 3, '-') << endl;
		int order = 1;
		// Print completely matching movies first
		if (dfs.find(keyword) != dfs.end()) {
			Print(dfs[keyword], bfs[keyword], order);
		}
		// Print nearly similar movies next
		if (order <= 10) {
			vector<string> remainingdfs;
			vector<string> remainingbfs;
			for (auto movie : dfs) { if (movie.first != keyword) remainingdfs.insert(remainingdfs.end(), movie.second.begin(), movie.second.end());}
			for (auto movie : bfs) { if (movie.first != keyword) remainingbfs.insert(remainingbfs.end(), movie.second.begin(), movie.second.end());}
			Print(remainingdfs, remainingbfs, order);
		}	
	}

public:
	Graph() {
		ReadFile();
		CreateGraph();
	};  // Constructor

	//  Function interacts with the main
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
		auto start_time = chrono::high_resolution_clock::now();
		unordered_map<string, vector<string>> matchingdfs = dfs(visiteddfs, keyword, command);  // Perform dfs
		auto end_time = chrono::high_resolution_clock::now();  // Record the end time
		auto elapsed_time_dfs = chrono::duration_cast<chrono::microseconds>(end_time - start_time);
		
		start_time = chrono::high_resolution_clock::now();
		
		unordered_map<string,vector<string>> matchingbfs = bfs(visitedbfs, keyword, command);  // Perform bfs
		end_time = chrono::high_resolution_clock::now();  // Record the end time
		auto elapsed_time_bfs = chrono::duration_cast<chrono::microseconds>(end_time - start_time);
		int  time_dfs = elapsed_time_dfs.count();
		int time_bfs = elapsed_time_bfs.count();

		cout << "DFS Total Elapsed time: " << time_dfs << " microseconds" << setw(19 - to_string(time_dfs).length()) << "|  " << "BFS Total Elapsed time: " << time_bfs << " microseconds" << endl;
		if (matchingdfs.size() == 0) cout << "No matching result!" << endl;
		PrintResult(matchingdfs, matchingbfs, keyword);  // Print result		
	}

	~Graph() {};  // Destructor

};
