#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <fstream>
#include <deque>
using namespace std;

/*
  worddice.cpp
  David Swanson
  November 19, 2015

  This program uses network flow to solve a problem where the user is given n amount of m
  sided dice and a list of words. This program tells the user if it is possible to spell
  that specific word with the dice. In the network flow algorithm, Edmonds-Karp's algorithm
  is used to find the augmenting path.
*/

/* Node class. The indicies of the boolean vector correspond to all the ascii characters. 
   This shows what letters are on the die. The adjacency list is a set so that the find 
   operation can be used.*/

class Node {
  public:
	int id;
    vector <int> booleans;
	set <int> adj;
	Node *backedge;
	int dist;

};

/* Graph class. The constructor takes a vector of strings representing the dice. A double 
   ended queue is used for Edmonds-Karp algorithm to find the augmenting path. The path 
   contains a pair of ints to tell where the edge comes from and where it is going. Various
   functions are used to assist in finding network flow.*/

class Graph {
  public:
	Graph(vector <string>);
	~Graph();
	deque <int> q;
	vector <pair<int,int> > path;
	vector <Node *> nodes;
	void reset();
	void makegraph(string);
	void findpath();
	void adjust();
	void  makepath();
	vector <string> dice;

};

/* Constructor for graph class. Initially sets the dice vector equal to the given vector so 
   the rest of the functions can access the dice. The nodes consist of a Source and Sink, 
   a node for each die, and the same amount of nodes for the words. the Source is 0 and the 
   Sink is the last element. */

Graph::Graph(vector <string> s1) {
  int i, j;
  Node *n;

  dice = s1;
  nodes.resize((2*dice.size())+2);

  /* Initialize all nodes, the source and sink boolean vectors have all true. */

  for (i = 0; i < nodes.size(); i++) {
	n = new Node;
	if (i == 0 || i == nodes.size()-1)
	  n->booleans.resize(256, 1);
	else
	  n->booleans.resize(256, 0);
	n->id = i;
	n->dist = -1;
	n->backedge = NULL;
	nodes[i] = n;
  }

  /* For the nodes corresonding to the dice, set the indices of the boolean vector to true
     if the die contains that letter. */

  for (i = 0; i < dice.size(); i++) {
    for (j = 0; j < dice[i].size(); j++) {
	  if (nodes[i+1]->booleans[dice[i][j]] == 0) {
	    nodes[i+1]->booleans[dice[i][j]] = 1;
	  }
	}
  }

  /* The source has an edge to every dice, and every letter of a given word has an edge to
     the sink. */

  for (i = 1; i < (1 + dice.size()); i++) {
	nodes[0]->adj.insert(i);
  }
  for (i = 1 + dice.size(); i < nodes.size()-1; i++) {
	nodes[i]->adj.insert(nodes.size()-1);
  }
}

/* Deconstructor deletes all nodes to prevent memory leak. */

Graph::~Graph() {
  int i;
  for (i = 0; i < nodes.size(); i++) {
	delete nodes[i];
  }
  nodes.clear();
}

/* Function used to make the graph given a string. */

void Graph::makegraph(string word) {
  int i, j;

  /* If the word has more characters than the amount of dice, then it is impossible to spell
     */

  if (word.size() > dice.size()) {
	cout << "Cannot spell " << word << endl;
	return;
  }

  /* Adds edges from dice to a letter in the word if the die contains that letter.  */

  for (i = 1; i < (1 + dice.size()); i++) {
	for (j = 0; j < word.size(); j++) {
	  if (nodes[i]->booleans[word[j]] == 1) {
		nodes[i]->adj.insert(j + (1 + dice.size()));
	  }
	}
  }
}

/* Reset is called when the program finishes processing one word and moves to the next */

void Graph::reset() {
  int i;

  /* Remove all edges, reset backedge and distance that were changed when using Edmonds-Karp
     algorithm.  */

  for (i = 0; i < nodes.size(); i++) {
	nodes[i]->adj.clear();
	nodes[i]->backedge = NULL;
	nodes[i]->dist = -1;
  }

  /* Reinsert source and sink edges. */

  for (i = 1; i < (1 + dice.size()); i++) {
	nodes[0]->adj.insert(i);
  }
  for (i = 1 + dice.size(); i < nodes.size()-1; i++) {
	nodes[i]->adj.insert(nodes.size()-1);
  }
}

/* Path finding algorithm that uses Edmonds-Karp */

void Graph::findpath() {
  int i;
  Node *n;
  set <int>::iterator sit;

  /* Continue until the queue is empty. Remove elements from the queue as they are processed.
     Process all edges, when the algorithm completes, the backedges from the sink will depict
	 the shortest path. */

  while(q.size() != 0) {
	n = nodes[*(q.begin())];
	q.pop_front();
	for(sit = n->adj.begin(); sit != n->adj.end(); sit++) {
	  if (nodes[(*sit)]->dist == -1) {
		nodes[(*sit)]->dist = 1 + n->dist;
		nodes[(*sit)]->backedge = n;
		q.push_back((*sit));
	  }
	}
  }
}

/* This function follows the backedges from the sink and pushes the values onto the path
   vector. The second element is the node the edge is coming from, and the first element is 
   the node where the edge is pointing. */

void Graph::makepath() {
  int j;
  Node *n;
  n = nodes[nodes.size()-1];
  while (n->backedge != NULL) {
	path.push_back(make_pair(n->id, n->backedge->id));
	n = n->backedge;
  }

}

/*  This function implements the network flow calculations. */

void Graph::adjust() {
  int i, j;
  Node *n;
  set <int>::iterator sit;

  /* The loop follows the path, removes each edge, and replaces it with an edge in the 
     opposite direction. This makes it so that the path cannot be used again. */

  for (i = 0; i < path.size(); i++) {
	n = nodes[path[i].second];
	sit = n->adj.find(path[i].first);
	if (sit != n->adj.end()) {
	  n->adj.erase(sit);
	}
	nodes[path[i].first]->adj.insert(path[i].second);
  }
}

/* Main function reads in dice and words, creates the graph, and solves network flow. */

int main(int argc, char *argv[]) {
  int i, j;						/* Induction varaibles */
  int count;					/* Keeps track of the number of paths found (flow) */
  ifstream file1, file2;		/* Used to read in dice and words */
  string line;					/* String input into vectors when reading in */
  Graph *g;						/* Graph instance pointer */
  vector <string> dice, words;	/* Vectors that hold dice and words */
  set <int>::iterator sit;		/* Traverses adjacency list */
  count = 0;

  /* Both input files are read in and stored into vectors. A new graph is made with the given
     dice. */

  file1.open(argv[1]);
  while (getline(file1, line)) {
    dice.push_back(line);
  }
  file2.open(argv[2]);
  while (getline(file2, line)) {
	words.push_back(line);
  }
  file1.close();
  file2.close();
  g = new Graph(dice);

  /* The words vector is traversed and the graph is updated for each word.  */

  for (i = 0; i < words.size(); i++) {
	g->makegraph(words[i]);
	g->nodes[0]->dist = 0;
	g->q.push_back(0);
	g->findpath();

    /* The program loops until an augmenting path is found or the flow is equal to the 
	   size of the word because since the edges all have a weight of 1, when the flow equals
	   the size of the word then the word can be spelled. The corresponding functions are 
	   called to update the graph and path, backedges, and distances are reset so the next
	   path can be found. */

	while(g->nodes[g->nodes.size()-1]->backedge != NULL  && count < words[i].size()) {
	  g->makepath();
	  g->adjust();
	  g->path.clear();
	  for (j = 0; j < g->nodes.size(); j++) {
		g->nodes[j]->backedge = NULL;
		g->nodes[j]->dist = -1;
	  }
	  g->nodes[0]->dist = 0;
	  g->q.push_back(0);
	  g->findpath();
	  count++;
	}

	/* Once the loop is done, if the count variable equals the size of the word, then it can
	   be spelled and the corresponding dice are output. If the count is not equal to the 
	   word size, then there is no combination of dice that can spell the word. */

	if (count == words[i].size()) {
	  for(j = 0; j < words[i].size(); j++) {
	    sit = g->nodes[j + (1 + dice.size())]->adj.begin();
		if (j == words[i].size() - 1) {
		  cout << (*sit) - 1 << ": ";
		}
		else {
		  cout << (*sit) - 1 << ",";
		}
	  }
	  cout << words[i] << endl;
	}
	else {
	  cout << "Cannot spell " << words[i] << endl;
	}

    /* The graph and count are reset to prep for the next word call. */

	g->reset();
	count = 0;
  }
  return 0;

}
