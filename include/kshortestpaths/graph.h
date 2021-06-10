#include <cmath>
#include <iostream>
#include <cassert>

//#include <new.h>
#define CUSTOMNEW
#include <kshortestpaths/2heap.h>
#include <kshortestpaths/list.h>

#ifndef kShortestPaths_graph
#define kShortestPaths_graph

struct GraphArc {
  int source;
  int dest;
  float weight;
  void *data;
};

struct GraphState {
  List<GraphArc> arcs;
};

struct Graph {
  GraphState *states;
  int nStates;
};

Graph reverseGraph(Graph g);
void dfsRec(int state, int pred);
void depthFirstSearch(Graph graph, int startState, bool* visited, void (*func)(int state, int pred));



void pushTopo(int state, int pred);
List<int> *topologicalSort(Graph g);

// Dijikstra's single source shortest path tree algorithm

struct DistToState {
  // when used in a dumb packed binary heap, this structure
  // keeps track of where each state's distance is in the heap
  int state;
  static DistToState **stateLocations;
  static float *weights;
  static float unreachable;
  operator float() const { return weights[state]; }
  void operator = (DistToState rhs);
  
  bool operator < (DistToState rhs) {
  return DistToState::weights[state] > DistToState::weights[rhs.state];
}

bool operator == (DistToState rhs) {
  return DistToState::weights[state] == DistToState::weights[rhs.state];
}

bool operator == (float rhs) {
  return DistToState::weights[state] == rhs;
}
};





// fills dist[state] with the distance from state to dest
// returns a graph containing only the edges along the shortest
// paths tree.  the GraphArc.data field in the return tree
// is a pointer to the GraphArc in the original graph
Graph shortestPathTree(Graph g, int dest, float *dist);

// rudimentary graph I/O (no error checking)

std::ostream & operator << (std::ostream &o, GraphArc &a);
std::istream & operator >> (std::istream &istr, GraphArc &a);
std::istream & operator >> (std::istream &istr, GraphState &s);
std::istream & operator >> (std::istream &istr, Graph &g);
std::ostream & operator << (std::ostream &out, Graph g);


#endif