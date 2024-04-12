from ast import Import

import glob
import csv
import re
import os


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
    keyword_pattern = f"{base_pattern}*{keyword}*.log"
    return glob.glob(keyword_pattern)

def process_files_Pointing(files,pathToKeep,start_keyword, final_search_keyword, output_file_path):
    """
    Process the list of files, checking each line for the search_keyword and writing results to a CSV file.
    """
    pattern = re.compile(r"Pointed at: Target[1-6]?")
    failedpattern = re.compile(r'Pointed at: (?!\s*Target[1-6]?\s*;).+;')
    
    with open(output_file_path, 'w', newline='') as outfile:
        writer = csv.writer(outfile)
        writer.writerow(['File', 'Target 1', 'Target 2', 'Target 3', 'Target 4', 'Target 5', 'Target 6','Mean_Distance_To_Center','fails'])
        count = 0
        for file_path in files:
    
            targetTimings = [0, 0, 0, 0, 0, 0]
            targetDistances = [100, 100, 100, 100, 100, 100]
            fails = 0
            targetcount = 0
            timestamp1 = 0
            timestamp2 = 0
            count +=1
            with open(file_path, 'r') as infile:
                for line_number, line in enumerate(infile, start=1):
                    if  start_keyword in line:
                        timestamp1 = extract_timestamp(line)
                    
                    #check if this was a fail
                    if failedpattern.search(line):
                        fails +=1

                    #check if this was a success
                    if pattern.search(line):
                        if targetcount > 5:
                            writer.writerow([f"ERROR too many targets"])
                            continue
                        
                        timestamp2 = extract_timestamp(line)
                        executionTime = calculate_time_difference(timestamp1,timestamp2)
                        targetTimings[targetcount] = executionTime
                        timestamp1 = timestamp2

                        numbers = re.findall(r"[-+]?\d*\.\d+|\d+", line)
                        targetDistances[targetcount] = float(numbers[-1])
                        targetcount += 1
                       
                    # check if all targets were hit  
                    if "All Targets Successfully hit" in line:
                        mean = (targetDistances[0]+targetDistances[1]+targetDistances[2]+targetDistances[3]+targetDistances[4]+targetDistances[5])/6
                        writer.writerow([f"File {count} {file_path[-pathToKeep:]}", targetTimings[0], targetTimings[1],targetTimings[2],targetTimings[3],targetTimings[4],targetTimings[5],mean,fails])
                        break
                    print(f"Processed line {line_number} in {file_path}")

def process_files_Rating(files,pathToKeep, output_file_path):
    """
    Process the list of files, checking each line for the search_keyword and writing results to a CSV file.
    """
    
    with open(output_file_path, 'w', newline='') as outfile:
        writer = csv.writer(outfile)
        writer.writerow(['File', 'GoodRatingTime', 'GoodRatingFails', 'BadRatingTime', 'BadRatingFails', 'AverageRatingTime', 'AverageRatingFails'])
        count = 0
        for file_path in files:
    
            # Variables to store the count of ratings between subtasks
            good_rating_count = 0
            bad_rating_count = 0
            average_rating_count = 0

            good_rating_time = 0
            bad_rating_time = 0
            average_rating_time = 0

            # Flags to indicate the current subtask being counted
            in_good_rating = False
            in_bad_rating = False
            in_average_rating = False

            targetcount = 0
            timestamp1 = 0
            timestamp2 = 0
            count +=1
            with open(file_path, 'r') as infile:
                for line_number, line in enumerate(infile, start=1):


                    if "Rating Task Started" in line:
                        timestamp1 = extract_timestamp(line)
                        in_good_rating = True  # Start counting for Good rating
                    elif "Completed Good rating" in line:
                        timestamp2 = extract_timestamp(line)
                        good_rating_time = calculate_time_difference(timestamp1,timestamp2)
                        timestamp1 = timestamp2
                        in_good_rating = False
                        in_bad_rating = True  # Start counting for Bad rating
                    elif "Completed bad rating" in line:
                        timestamp2 = extract_timestamp(line)
                        bad_rating_time = calculate_time_difference(timestamp1,timestamp2)
                        timestamp1 = timestamp2
                        in_bad_rating = False
                        in_average_rating = True  # Start counting for Average rating
                    elif "Completed average rating" in line:
                        timestamp2 = extract_timestamp(line)
                        average_rating_time = calculate_time_difference(timestamp1,timestamp2)
                        timestamp1 = timestamp2
                        in_average_rating = False  # End counting for Average rating
                    
                    # Count the [Rating] lines within the respective subtasks
                    if "[Rating]" in line or "[Pointing]" in line:
                        if in_good_rating:
                            good_rating_count += 1
                        elif in_bad_rating:
                            bad_rating_count += 1
                        elif in_average_rating:
                            average_rating_count += 1    
                        targetcount += 1
                       
                    # check if all targets were hit  
                    if "[Task]Completed average rating" in line:
                        writer.writerow([f"File {count} {file_path[-pathToKeep:]}", good_rating_time, good_rating_count-1,bad_rating_time,bad_rating_count-1,average_rating_time,average_rating_count-1])
                        break
                    print(f"Processed line {line_number} in {file_path}")


def process_files_cutting(files,pathToKeep,countKeyword,output_file_path):
     with open(output_file_path, 'w', newline='') as outfile:
        writer = csv.writer(outfile)
        writer.writerow(['File', 'CuttingCount', 'SolveTime'])
        filecount = 0
        
        for file_path in files:
            filecount += 1
            count = 0
            timestamp = 0
            with open(file_path, 'r') as infile:
                for line_number, line in enumerate(infile, start=1):
                    if  countKeyword in line:
                        count +=1
                        timestamp = extract_timestamp(line)
                    print(f"Processed line {line_number} in {file_path}")     
                #no lines here
                if count != 0:
                    writer.writerow([f"File {filecount} {file_path[-pathToKeep:]}", count, timestamp])


#Adds to the ouput file IF the final_seach_keyword is in the file. Also calculates timing till then and file as well as line number
def process_files_intro(files,pathToKeep,start_keyword, final_search_keyword, output_file_path):
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
                        timestamp2 = extract_timestamp(line)
                        executionTime = calculate_time_difference(timestamp1,timestamp2)
                        timestamp1 = timestamp2
                        writer.writerow([f"File {count} {file_path[-pathToKeep:]}", line_number, executionTime])
                    print(f"Processed line {line_number} in {file_path}")


def checkintro():
    files = find_files_by_keyword(f"{base_pattern}/IntroductionTest/", 'Introduction')
    process_files_intro(files,41,'Button pressed successfully', 'Tutorial Completed', 'results_with_Introduction.csv')
    
def checkpointandrate():
    files = find_files_by_keyword(f"{base_pattern}/PointingRatingSceneTest/", 'Pointing_Rating')
    process_files_Pointing(files,45,'Spawn pointing targets', 'Completed Task', 'results_with_Pointing.csv')
    process_files_Rating(files,45,'results_with_Rate.csv')

def checkcutting():
    files = find_files_by_keyword(f"{base_pattern}/Cutting/", 'CuttingScene')
    process_files_cutting(files,40,"was cut",'results_with_Cutting.csv')


# Base pattern for the files (directory path)
base_pattern = 'C:/Users/Robin H/source/repos/ResultEvaluator/ResultEvaluator'
base_pattern = "C:/Users/helme/Documents/Unreal Projects/RobCogFork11_23/ResultEvaluator/ResultEvaluator"
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
checkpointandrate()
checkcutting()


