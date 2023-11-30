import pandas as pd

# Load the processed CSV file
df = pd.read_csv('imdb_movies_processed.csv')

# Count the number of rows
row_count = len(df)

# Print the result
print(f"The number of rows in imdb_movies_processed.csv is: {row_count}")
