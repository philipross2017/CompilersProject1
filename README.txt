This Code is By Philip Ross. Last edited 3/26/2021.
This code is a stylechecker for SI204, Intro to Programming
seen here: https://www.usna.edu/Users/cs/wcbrown/courses/S20SI204/resources/style.html
This code does not yet support libraries.
This code DOES NOT check your comment quality, it only counts them. Make sure you still have good comments.

INSTRUCTIONS

In order to clean up your code to the greatest extent possible, you must run this code twice.

1. First run it into a different file to correct formatting.
	ex.
		./StyleChecker test1.cpp --> output.cpp

2. Next run it again on the output file, this time with no other output, to fix naming conventions.
	ex.
		./StyleChecker output.cpp

This code will then correct any naming convention issues in your code.

AREAS FOR FUTURE IMPROVEMENT:

1. Make it so the code corrects both in one pass

2. Make the code ignore the AST's from libraries

3. integrate some more commenting stuff into the code as well.
	ex.
		make sure the code has a comment header.

4. Include a dictionary and heuristics for smarter Name Suggestions
