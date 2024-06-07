# Duplicate File Finder

## Overview
This document outlines the functionality and usage of a command-line tool designed to find duplicate files within a specified directory. The tool utilizes multithreading for efficient file processing and provides options for recursive directory search and duplicate deletion.

## Features
- **Multithreaded Processing:** Utilizes multithreading to concurrently compare files for duplicate content, enhancing performance.
- **Recursive Search:** Supports recursive searching of subdirectories to identify duplicates across the entire directory tree.
- **Duplicate Deletion:** Optionally deletes duplicate files, keeping only one instance of each duplicate.
- **File Type Statistics:** Provides statistics on the number of files of each supported file type (.txt, .jpg, .pdf, .py, and others).

## Usage
```sh
./duplicate_finder [options] <directory>
```
### Options
- `-d`: Delete duplicates and keep one instance.
- `-r`: Recursively search subdirectories.

### Example
```sh
./duplicate_finder -d -r /path/to/directory
```

## Implementation Details
The tool is implemented in C and utilizes POSIX threads for parallel processing. Below are key components of the implementation:

### File Comparison
- Files are compared using a chunk-based approach to efficiently handle large files.
- Content comparison is performed to identify duplicates based on file content.

### Multithreading
- Utilizes POSIX threads to process files concurrently, improving performance.
- Implements thread-safe data structures and synchronization using mutex locks.

### Directory Traversal
- Recursively traverses directories to process all files within the specified directory and its subdirectories.
- Handles directory and file operations using standard C library functions.

### Logging
- Logs duplicate file search progress and results to a specified log file (`duplicate_files.log`).

## File Type Statistics
The tool provides statistics on the number of files of each supported file type encountered during the duplicate search process.

## Output
- Displays groups of duplicate files, indicating their paths and content comparison results.
- Optionally deletes duplicate files if the `-d` option is specified.
- Outputs total size of paths before and after removing duplicates.
- Provides a breakdown of file type statistics.

## Error Handling
- Handles errors related to file operations, memory allocation, and thread creation gracefully.
- Logs error messages to stderr for troubleshooting purposes.

## Conclusion
The duplicate file finder tool offers an efficient solution for identifying and managing duplicate files within a directory. Its multithreaded design enables fast processing of large file collections, while its options provide flexibility in duplicate handling. Users can leverage this tool to optimize storage space and organize file repositories effectively.
