CanzeRead is a simple program to sort and make more readable the output stored in the log files outputted by the android program canZE: 
https://canze.fisch.lu/

It scans log file for available variables, prompts the user to choose which variables to put in individual files, and then creates these files.
Output files are ASCI files with ADF extension, whose structure is straightforward: 
- the first row contains a comment with date-time information
- the second row contains the names ov variables (names are chosen to be without spaces to allow programs which separate tokens through spaces to sort them effectively)
- then several two-column rows containing time (in s) and the variables

Individual files are needed because each CanZE output sample carries a timestamp of its own.

Usage of the program is straightforward: just drop the file to be processed, select the variables to be put on individual files and click ok. the output files will be put in the same folder cntaining the input file.

CanzeRead is written in Qt. Because of this, even though it is currently compiled only for Windows, porting to other OS's should be straightforward.
