//
// Created by giacomo on 10/06/21.
//

#include "graph.h"

// Depth First Search (only one search can be active at once)
Graph dfsGraph;
bool *dfsVis;
void (*dfsFunc)(int, int);
void (*dfsExitFunc)(int, int);

template<> Node<GraphArc> *Node<GraphArc>::freeList = nullptr;
template<> const int Node<GraphArc>::newBlocksize = 64;

template<> Node<int> *Node<int>::freeList = nullptr;
template<> const int Node<int>::newBlocksize = 64;

float *DistToState::weights = nullptr;
DistToState **DistToState::stateLocations = nullptr;
float DistToState::unreachable = std::numeric_limits<float>::max();


Graph reverseGraph(Graph g) {
    Graph rev;
    rev.states = new GraphState[g.nStates];
    rev.nStates = g.nStates;
    for ( int i  = 0 ; i < g.nStates ; ++i )
        for ( ListIter<GraphArc> l(g.states[i].arcs) ; l ; ++l ) {
            GraphArc r;
            r.data = &l.data();
            r.dest = i;
            r.source = l.data().dest;
            r.weight = l.data().weight;
            rev.states[r.source].arcs.push(r);
        }
    return rev;
}

void dfsRec(int state, int pred) {
    if ( dfsVis[state] )
        return;
    dfsVis[state] = true;
    if ( dfsFunc )
        dfsFunc(state, pred);
    for ( ListIter<GraphArc> l(dfsGraph.states[state].arcs) ; l ; ++l ) {
        int dest = l.data().dest;
        dfsRec(dest, state);
    }
    if ( dfsExitFunc )
        dfsExitFunc(state, pred);
}

void depthFirstSearch(Graph graph, int startState, bool *visited, void (*func)(int, int)) {
    dfsGraph = graph;
    dfsVis = visited;
    dfsFunc = func;
    dfsExitFunc = NULL;
    dfsRec(startState, -1);
}


List<int> *topSort;

void pushTopo(int state, int pred) {
    topSort->push(state);
    pred = pred;			// dummy statement to avoid warnings
}


List<int> *topologicalSort(Graph g) {
    topSort = new List<int>;
    dfsGraph = g;
    dfsVis = new bool[g.nStates];
    dfsFunc = NULL;
    dfsExitFunc = pushTopo;
    int i;
    for ( i = 0 ; i < g.nStates ; ++i )
        dfsVis[i] = 0;
    for ( i = 0 ; i < g.nStates ; ++i )
        dfsRec(i, -1);
    delete[] dfsVis;
    return topSort;
}

Graph shortestPathTree(Graph g, int dest, float *dist) {
    int nStates = g.nStates;
    GraphArc **best = new GraphArc *[nStates];

    GraphState *rev = reverseGraph(g).states;

    GraphState *pathTree = new GraphState[nStates];
    int nUnknown = nStates;

    DistToState *distQueue = new DistToState[nStates];

    float *weights = dist;
    int i;
    for ( i = 0 ; i < nStates ; ++i ) {
        weights[i] = HUGE_VAL;
    }

    DistToState **stateLocations = new DistToState *[nStates];
    DistToState::weights = weights;
    DistToState::stateLocations = stateLocations;

    weights[dest] = 0;
    for ( i = 1; i < nStates ; ++i ) {
        int fillWith;
        if ( i <= dest )
            fillWith = i-1;
        else
            fillWith = i;
        distQueue[i].state = fillWith;
        stateLocations[fillWith] = &distQueue[i];
    }
    distQueue[0].state = dest;
    stateLocations[dest] = &distQueue[0];

    for ( i = 0 ; i < nStates ; ++i )
        best[i] = NULL;

    float candidate;
    for ( ; ; ) {
        if ( (float)distQueue[0] == HUGE_VAL || nUnknown == 0 ) {
            break;
        }
        int targetState, activeState = distQueue[0].state;
        heapPop(distQueue, distQueue + nUnknown--);
        for ( ListIter<GraphArc> a = rev[activeState].arcs ; a ; ++a ) {
            // future: compare only best arc to any given state
            targetState = a.data().dest;
            if ( (candidate = a.data().weight + weights[activeState] )
                 < weights[targetState] ) {

                weights[targetState] = candidate;
                best[targetState] = (GraphArc *)a.data().data;
                heapAdjustUp(distQueue, stateLocations[targetState]);
            }
        }
    }

    for ( i = 0 ; i < nStates ; ++i )
        if ( best[i] ) {
            pathTree[i].arcs.push(*best[i]);
            pathTree[i].arcs.top().data = best[i];
        }

    delete[] stateLocations;
    delete[] distQueue;
    delete[] rev;
    delete[] best;

    Graph ret;
    ret.nStates = nStates;
    ret.states = pathTree;
    return ret;
}

void DistToState::operator=(DistToState rhs) {
    stateLocations[rhs.state] = this;
    state = rhs.state;
}


// rudimentary graph I/O (no error checking)

std::ostream & operator << (std::ostream &o, GraphArc &a)
{
  return o << '(' << a.source << ' ' << a.dest << ' ' << a.weight << ')';
}

std::istream & operator >> (std::istream &istr, GraphArc &a)
{
  char c;
  int i;
  istr >> c;			// open paren
  istr >> a.source;
  istr >> a.dest;
  istr >> a.weight;
  istr >> c;			// close paren
  a.data = NULL;
  return istr;
}

std::istream & operator >> (std::istream &istr, GraphState &s)
{
  char c;
  return istr;
}

std::istream & operator >> (std::istream &istr, Graph &g)
{
  char c;
  GraphArc a;
  istr >> g.nStates;
  if ( istr && g.nStates > 0 )
    g.states = new GraphState[g.nStates];
  else
    g.states = NULL;
  for ( ; ; ) {
    istr >> c;
    if ( !istr || c != '(')
      break;
    istr.putback(c);
    istr >> a;
    if ( !(istr && a.source >= 0 && a.source < g.nStates) )
      break;
    g.states[a.source].arcs.push(a);
  }
  return istr;
}

std::ostream & operator << (std::ostream &out, Graph g)
{
  out << g.nStates << '\n';;
  for ( int i = 0 ; i < g.nStates ; ++i ) {
    for ( ListIter<GraphArc> a(g.states[i].arcs) ; a ; ++a )
      out << a.data() << ' ';
    out << '\n';
  }
}
