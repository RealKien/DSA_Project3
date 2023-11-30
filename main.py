import pandas as pd

# Load the TSV file
df = pd.read_csv('title.basics.tsv', delimiter='\t', encoding='utf-8')

# Remove rows with empty values ("\N") in the 'runtimeMinutes' column
df = df[df['runtimeMinutes'] != '\\N']

df = df[df['genres'] != '\\N']

# Keep only rows where 'titleType' is 'movie'
df = df[df['titleType'] == 'movie']

# Split the 'genres' column and keep only the first genre
df['genres'] = df['genres'].str.split(',').str[0]

# Delete specified columns
df = df.drop(columns=['originalTitle', 'isAdult', 'endYear', 'titleType'])

# Keep the first 100k rows
df = df.head(100000)

# Save the modified DataFrame to a new CSV file
df.to_csv('imdb_movies_processed.csv', index=False)
