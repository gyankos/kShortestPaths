/* k Shortest Paths in O(E*log V + L*k*log k) time (L is path length)
   Implemented by Jon Graehl (jongraehl@earthling.net)
   Following David Eppstein's "Finding the k Shortest Paths" March 31, 1997 draft
   (http://www.ics.uci.edu/~eppstein/
    http://www.ics.uci.edu/~eppstein/pubs/p-kpath.ps)
   */
#include "graph.h"
#include "2heap.h"

#ifndef kShortestPaths_kbest
#define kShortestPaths_kbest

struct pGraphArc {
  GraphArc *p;
  GraphArc * operator ->() const { return p; }
  operator GraphArc *() const { return p; }
  
  int operator < (const pGraphArc r) ;

};


// pGraphArc must be used rather than GraphArc * because in order to overload operator < "`operator <(const GraphArc *, const GraphArc *)' must have an argument of class or enumerated type"

struct GraphHeap {
  static List<GraphHeap *> usedBlocks;
  static GraphHeap *freeList;
  static const int newBlocksize;
  GraphHeap *left, *right;	// for balanced heap  
  int nDescend;
  GraphArc *arc;		// data at each vertex, or cross edge 
  pGraphArc *arcHeap;		// binary heap of sidetracks originating from a state
  int arcHeapSize;
  void *operator new(size_t s)
  {
    size_t dummy = s;
    dummy = dummy;
    GraphHeap *ret, *max;
    if (freeList) {
      ret = freeList;
      freeList = freeList->left;
      return ret;
    }
    freeList = (GraphHeap *)::operator new(newBlocksize * sizeof(GraphHeap));
    usedBlocks.push(freeList);
    freeList->left = NULL;
    max = freeList + newBlocksize - 1;
    for ( ret = freeList++; freeList < max ; ret = freeList++ )
      freeList->left = ret;
    return freeList--;
  }
  void operator delete(void *p) 
  {
    GraphHeap *e = (GraphHeap *)p;
    e->left = freeList;
    freeList = e;
  }
  static void freeAll();
  
  int operator < ( const GraphHeap &r) {
  return arc->weight > r.arc->weight;
}
};




struct EdgePath {
  GraphHeap *node;
  int heapPos;			// -1 if arc is GraphHeap.arc
  EdgePath *last;
  float weight;
  
  int operator < (const EdgePath &r) {
  return weight > r.weight;
}
};



Graph sidetrackGraph(Graph sourceG, Graph subtractG, float *dist);
void buildSidetracksHeap(int state, int pred);
void printTree(GraphHeap *t, int n);

void insertShortPath(int source, int dest, ListIter<GraphArc *> &path);

List<List<GraphArc *> > *bestPaths(Graph graph, int source, int dest, int k);

// a sidetrack from a given state in an arc originating from any state along the shortest path to the destination, that is not in the shortest path tree.  Paths are uniquely determined by a sequence of sidetracks from the destination of the previous sidetrack, or the source state if there was no previous sidetrack (see Eppstein)

Graph sidetrackGraph(Graph fullGraph, Graph shortGraph, float *dist);

// see Eppstein's paper for explanation of this shared heap

void buildSidetracksHeap(int state, int pred);

// debugging print routines

void printTree(GraphHeap *t, int n);



void shortPrintTree(GraphHeap *t);

#endif kShortestPaths_kbest
