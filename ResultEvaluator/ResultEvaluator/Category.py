
import sys
from math import cos,radians
import csv



# Define the path to the input and output files
input_file_path = 'ExTextfile.txt'
output_file_path = 'results.csv'

# The keyword to search for
keyword = 'bielefeld'

# Open the input file to read and the output file to write
with open(input_file_path, 'r') as infile, open(output_file_path, 'w', newline='') as outfile:
    # Create a csv writer object for the output file
    writer = csv.writer(outfile)
    
    # Write the header row in the output CSV file
    writer.writerow(['Line Number', 'Contains Keyword'])
    
    # Read the input file line by line
    for line_number, line in enumerate(infile, start=1):
        # Check if the keyword is in the current line
        contains_keyword = keyword in line
        
        # Write the result to the CSV file
        writer.writerow([line_number, contains_keyword])
        # Optional: Print the result to the console
        print(f"Line {line_number}: {contains_keyword}")