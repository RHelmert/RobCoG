from ast import Import

import glob
import csv
import re


def extract_timestamp(line):
    """
    Extracts the timestamp from a given line of text.
    
    Parameters:
        line (str): A string containing the log entry with a timestamp.
        
    Returns:
        float: The extracted timestamp as a float. Returns None if no timestamp is found.
    """
    # Regular expression to find the timestamp
    timestamp_pattern = re.compile(r'\[(\d+\.\d+)\]')  # Matches floating point number inside brackets
    match = timestamp_pattern.search(line)
    
    if match:
        return float(match.group(1))
    else:
        return None

def get_line_content(file_path, line_number):
    """
    This function returns the content of a specific line_number from a file.
    """
    with open(file_path, 'r') as file:
        for current_line, content in enumerate(file, start=1):
            if current_line == line_number:
                return content.strip()
    return None  # Return None if the line_number is not found

def calculate_time_difference(timestamp1, timestamp2):
    """
    Calculates the difference between two timestamps.
    
    Parameters:
        timestamp1 (float): The first timestamp in seconds.
        timestamp2 (float): The second timestamp in seconds.
        
    Returns:
        float: The difference between the two timestamps in seconds.
    """
    return abs(timestamp2 - timestamp1)  # Return the absolute difference

def find_files_by_keyword(base_pattern, keyword):
    """
    Returns a list of file paths that contain the keyword in their filenames.
    """
    # Create a pattern to match any files that include the keyword
    keyword_pattern = f"{base_pattern}/IntroductionTest/*{keyword}*.log"
    return glob.glob(keyword_pattern)


#Adds to the ouput file IF the final_seach_keyword is in the file. Also calculates timing till then and file as well as line number
def process_files(files,pathToKeep,start_keyword, final_search_keyword, output_file_path):
    """
    Process the list of files, checking each line for the search_keyword and writing results to a CSV file.
    """
    with open(output_file_path, 'w', newline='') as outfile:
        writer = csv.writer(outfile)
        writer.writerow(['File', 'Line Number', 'SolveTime'])
        count = 0
        for file_path in files:
            timestamp1 = 0
            timestamp2 = 0
            count +=1
            with open(file_path, 'r') as infile:
                for line_number, line in enumerate(infile, start=1):
                    if  start_keyword in line:
                        timestamp1 = extract_timestamp(line)
                    contains_keyword = final_search_keyword in line
                    if contains_keyword:
                        executionTime = calculate_time_difference(timestamp1,extract_timestamp(line))
                        writer.writerow([f"File {count} {file_path[-pathToKeep:]}", line_number, executionTime])
                    print(f"Processed line {line_number} in {file_path}")


def checkintro():
    files = find_files_by_keyword(base_pattern, 'Introduction')
    process_files(files,41,'Button pressed successfully', 'Tutorial Completed', 'results_with_Introduction.csv')

# Base pattern for the files (directory path)
base_pattern = 'C:/Users/Robin H/source/repos/ResultEvaluator/ResultEvaluator'
# Specific substring to look for in filenames
filename_keyword = 'Introduction'
# Find and process files
# files = find_files_by_keyword(base_pattern, filename_keyword)



# Keyword to search within the content of the files
search_keyword = 'Tutorial Completed'
# Output CSV file path
output_file_path = 'results_with_Introduction.csv'
# Find and process files
#process_files(files, search_keyword, output_file_path)

checkintro()

print(f"Files processed. Results are saved in '{output_file_path}'")
