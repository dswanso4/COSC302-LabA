# COSC302-LabA
Letter-Dice network flow problem using Edmonds-Karp's algorithm

# worddice.cpp
This file uses a graph structure to solve a network flow problem. The user inputs 2 
text files; one for the dice and one for the words. The dice text file contains a list 
of strings. The length of the string determines how many sides the die has and the 
characters are the letters on each side of the die. The words text file contains a list 
words to spell. This program determines which words can be spelled using network flow
and Edmonds-Karp's algorithm

# Usage

./worddice dice.txt words.txt
