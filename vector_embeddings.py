import pandas as pd
from gensim.models import Word2Vec

# Load the processed CSV file
df = pd.read_csv('imdb_movies_processed.csv')

# Tokenize movie titles using a simple whitespace tokenizer
tokenized_titles = [title.lower().split() for title in df['primaryTitle']]

# Train Word2Vec model
word2vec_model = Word2Vec(sentences=tokenized_titles, vector_size=100, window=5, min_count=1, workers=4)

# Function to get the average vector for a movie title
def get_average_vector(title, model):
    tokens = title.lower().split()
    vectors = [model.wv[token] for token in tokens if token in model.wv]
    if vectors:
        return sum(vectors) / len(vectors)
    else:
        return [0] * model.vector_size

# Add a new column with the average vector for each movie title
df['title_embedding'] = df['primaryTitle'].apply(lambda x: get_average_vector(x, word2vec_model))

# Save the DataFrame with embeddings to a new CSV file
df.to_csv('imdb_movies_with_embeddings.csv', index=False)
