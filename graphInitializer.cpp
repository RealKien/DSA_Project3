#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

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
	void ReadFile() {
		ifstream file("./imdb_movies_with_embeddings.csv");
		if (!file.is_open()) cout << "Error opening file! " << endl;

		string line;
		string token;
		bool flag = false;
		int counter = 1; //Use for debugging, counter < 200 for fast debugging
		while (getline(file, line), counter++ < 1000) {  // Start reading
			flag = false;
			Movie movie; 
			getline(file, line, ',');  // Read ID
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
			getline(file, line, ']');
			string temp = line.substr(2, line.size());
			istringstream  iss2(temp);
			double value;
			while (iss2 >> value) {  //stream each value to vector
				
				movie.title_embedding.push_back(value);
			}
			// Add to containers to prepare graph creating
			movieContainer.push_back(movie);
			movieIDs.push_back(movie.title);
			movieNameEmbeddings.push_back(movie.title_embedding);
			
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
		for (auto i = 0; i < vec1.size(); i++) {
			dotProduct += vec1[i] * vec2[i];
			mag1 += vec1[i] * vec1[i];
			mag2 += vec2[i] * vec2[i];
		}
		if (mag1 == 0 || mag2 == 0) return -1; // Error
		return (dotProduct / (sqrt(mag1) * sqrt(mag2)));
	}

	void CreateGraph() {  // Adjacency Matrix
		int counter = 1;
		cout << "Length" << movieIDs.size() << endl;
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
	void PrintGraph() {
		int counter = 1;
		for (auto token: MovieGraph) {
			Movie movie = token.second;
			cout << "ID: " << movie.ID << endl;
			cout << "Title: " << movie.title << endl;
			cout << "Year of release: " << movie.year_released << endl;
			cout << "Duration: " << movie.runtime << endl;
			cout << "Genre: " << movie.genre << endl;
			cout << "Similarity: " << endl;
			for (auto adjacent : WeightedGraph[token.first]) {
				pair<string, double> temp = adjacent;
				cout << adjacent.first << " " << adjacent.second << endl;
			}
			cout << endl;
			// For debugging
			if (counter > 10) break;
			counter++;

		}
	}
};
// Simple main
int main() {
	Graph movie;
	movie.ReadFile();
	movie.CreateGraph();
	movie.PrintGraph();
}

