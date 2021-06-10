//
// Created by giacomo on 10/06/21.
//

#include <kshortestpaths/kbest.h>

template<>Node<GraphHeap *> *Node<GraphHeap *>::freeList = nullptr;
template<> const int Node<GraphHeap *>::newBlocksize = 64;

template<> Node<GraphArc *> *Node<GraphArc *>::freeList = nullptr;
template<> const int Node<GraphArc *>::newBlocksize = 64;

template<> Node<List<GraphArc *> > *Node<List<GraphArc *> >::freeList = nullptr;
template<> const int Node<List<GraphArc *> >::newBlocksize = 64;


List<GraphHeap *> GraphHeap::usedBlocks;
GraphHeap * GraphHeap::freeList = NULL;
const int GraphHeap::newBlocksize = 64;



Graph sidetracks;
GraphHeap **pathGraph;
GraphState *shortPathTree;

int pGraphArc::operator < ( const pGraphArc r) {
  return p->weight > r->weight;
}

void GraphHeap::freeAll() {
    while ( usedBlocks.notEmpty() ) {
        ::operator delete((void *)usedBlocks.top());
        usedBlocks.pop();
    }
    freeList = NULL;
}


void insertShortPath(int source, int dest, ListIter<GraphArc *> &path) {
    GraphArc *taken;
    for ( int iState = source ; iState != dest; iState = taken->dest ) {
        taken = &shortPathTree[iState].arcs.top();
        path.insert((GraphArc *)taken->data);
    }
}

List<List<GraphArc *> > *bestPaths(Graph graph, int source, int dest, int k) {
    int nStates = graph.nStates;
    assert(nStates > 0 && graph.states);
    assert(source >= 0 && source < nStates);
    assert(dest >= 0 && dest < nStates);

    List<List<GraphArc *> > *paths = new List<List<GraphArc *> >;
    ListIter<List<GraphArc *> > insertHere(*paths); // append rather than push so best path comes first in list

    float *dist = new float[nStates];
    Graph shortPathGraph = shortestPathTree(graph, dest, dist);
    shortPathTree = shortPathGraph.states;

    if ( shortPathTree[source].arcs.notEmpty() || source == dest ) {

        ListIter<GraphArc *> path(insertHere.insert(List<GraphArc *>()));
        insertShortPath(source, dest, path);

        if ( k > 1 ) {
            GraphHeap::freeAll();
            List<List<GraphArc *> > graphPaths;
            Graph revPathTree = reverseGraph(shortPathGraph);
            pathGraph = new GraphHeap *[nStates];
            sidetracks = sidetrackGraph(graph, shortPathGraph, dist);
            bool *visited = new bool[nStates];
            for ( int i = 0 ; i < nStates ; ++i ) visited[i] = 0;
            depthFirstSearch(revPathTree, dest, visited, buildSidetracksHeap);
            if ( pathGraph[source] ) {
                //    for ( int i = 0 ; i < nStates ; ++i )
                //      printTree(pathGraph[i], 0);
                EdgePath *pathQueue = new EdgePath[4 * (k+1)];	// out-degree is at most 4
                EdgePath *endQueue = pathQueue;
                EdgePath *retired = new EdgePath[k+1];
                EdgePath *endRetired = retired;
                EdgePath newPath;
                newPath.weight = pathGraph[source]->arc->weight;
                newPath.heapPos = -1;
                newPath.node = pathGraph[source];
                newPath.last = NULL;
                heapAdd(pathQueue, endQueue++, newPath);
                while ( heapSize(pathQueue, endQueue) && --k ) {
                    EdgePath *top = pathQueue;
                    GraphArc *cutArc;
                    List<GraphArc *> shortPath;
                    //      cout << top->weight;
                    if ( top->heapPos == -1 )
                        cutArc = top->node->arc;
                    else
                        cutArc = top->node->arcHeap[top->heapPos];
                    shortPath.push( cutArc);
                    //      cout << ' ' << *cutArc;
                    EdgePath *last;
                    while ( (last = top->last) ) {
                        if ( !((last->heapPos == -1 && (top->heapPos == 0 || top->node == last->node->left || top->node == last->node->right )) || (last->heapPos >= 0 && top->heapPos != -1 )) ) { // got to p on a cross edge
                            if ( last->heapPos == -1 )
                                cutArc = last->node->arc;
                            else
                                cutArc = last->node->arcHeap[last->heapPos];
                            shortPath.push(cutArc);
                            //	  cout << ' ' << *cutArc;
                        }
                        top = last;
                    }
                    //      cout << '\n';
                    ListIter<GraphArc *> fullPath(insertHere.insert(List<GraphArc *>()));
                    int sourceState = source;
                    for ( ListIter<GraphArc *> cut(shortPath) ; cut ; ++cut ) {
                        insertShortPath(sourceState, cut.data()->source, fullPath);
                        sourceState = cut.data()->dest;
                        fullPath.insert((GraphArc *)cut.data()->data);
                    }
                    insertShortPath(sourceState, dest, fullPath);
                    *endRetired = pathQueue[0];
                    newPath.last = endRetired++;
                    heapPop<EdgePath>(pathQueue, endQueue--);
                    int lastHeapPos = newPath.last->heapPos;
                    GraphArc *spawnVertex;
                    GraphHeap *from = newPath.last->node;
                    float lastWeight = newPath.last->weight;
                    if ( lastHeapPos == -1 ) {
                        spawnVertex = from->arc;
                        newPath.heapPos = -1;
                        if ( from->left ) {
                            newPath.node = from->left;
                            newPath.weight = lastWeight + (newPath.node->arc->weight - spawnVertex->weight);
                            heapAdd(pathQueue, endQueue++, newPath);
                        }
                        if ( from->right ) {
                            newPath.node = from->right;
                            newPath.weight = lastWeight + (newPath.node->arc->weight - spawnVertex->weight);
                            heapAdd(pathQueue, endQueue++, newPath);
                        }
                        if ( from->arcHeapSize ) {
                            newPath.heapPos = 0;
                            newPath.node = from;
                            newPath.weight = lastWeight + (newPath.node->arcHeap[0]->weight - spawnVertex->weight);
                            heapAdd(pathQueue, endQueue++, newPath);
                        }
                    } else {
                        spawnVertex = from->arcHeap[lastHeapPos];
                        newPath.node = from;
                        int iChild = 2 * lastHeapPos + 1;
                        if ( from->arcHeapSize > iChild  ) {
                            newPath.heapPos = iChild;
                            newPath.weight = lastWeight + (newPath.node->arcHeap[iChild]->weight - spawnVertex->weight);
                            heapAdd(pathQueue, endQueue++, newPath);
                            if ( from->arcHeapSize > ++iChild ) {
                                newPath.heapPos = iChild;
                                newPath.weight = lastWeight + (newPath.node->arcHeap[iChild]->weight - spawnVertex->weight);
                                heapAdd(pathQueue, endQueue++, newPath);
                            }
                        }
                    }
                    if ( pathGraph[spawnVertex->dest] ) {
                        newPath.heapPos = -1;
                        newPath.node = pathGraph[spawnVertex->dest];
                        newPath.heapPos = -1;
                        newPath.weight = lastWeight + newPath.node->arc->weight;
                        heapAdd(pathQueue, endQueue++, newPath);
                    }
                }
                delete[] pathQueue;
                delete[] retired;
            }
            GraphHeap::freeAll();
            delete[] pathGraph;
            delete[] visited;
            delete[] revPathTree.states;
            delete[] sidetracks.states;
        }
    }

    delete[] graph.states;
    delete[] shortPathGraph.states;
    delete[] dist;

    return paths;
}

Graph sidetrackGraph(Graph fullGraph, Graph shortGraph, float *dist) {
    //  subtracts shortGraph from fullGraph, arcs' data member points to arc in fullGraph
    assert(fullGraph.nStates == shortGraph.nStates);
    int nStates = fullGraph.nStates;
    GraphState *sub = new GraphState[nStates];
    for ( int i = 0 ; i < nStates ; ++i )
        if ( dist[i] != HUGE_VAL )
            for ( ListIter<GraphArc> l(fullGraph.states[i].arcs) ; l ; ++l ) {
                assert(i == l.data().source);
                int isShort = 0;
                for ( ListIter<GraphArc> r(shortGraph.states[i].arcs) ; r ; ++r )
                    if ( r.data().data == &l.data() ) { // arcs in shortest path tree have data pointing to the arc corresponding to it in the original graph
                        isShort = 1;
                        break;
                    }
                if ( !isShort )
                    if ( dist[l.data().dest] != HUGE_VAL ) {
                        GraphArc w = l.data();
                        w.weight = w.weight - (dist[i] - dist[w.dest]);
                        w.data = &l.data();
                        sub[i].arcs.push(w);
                    }
            }
    Graph ret;
    ret.nStates = fullGraph.nStates;
    ret.states = sub;
    return ret;
}

void buildSidetracksHeap(int state, int pred)
{
    GraphHeap *prev;

    if ( pred == -1 )
        prev = NULL;
    else
        prev = pathGraph[pred];

    ListIter<GraphArc> s(sidetracks.states[state].arcs);
    if ( s ) {
        int heapSize = 0;
        GraphArc *min;
        min = &s.data();
        while ( ++s ) {
            if ( s.data().weight < min->weight )
                min = &s.data();
            ++heapSize;
        }
        pathGraph[state] = new GraphHeap;
        pathGraph[state]->arc = min;
        pathGraph[state]->arcHeapSize = heapSize;
        if ( heapSize ) {
            pGraphArc *heapStart = pathGraph[state]->arcHeap = new pGraphArc[heapSize];
            pGraphArc *heapI = heapStart;
            for ( ListIter<GraphArc> gArc(sidetracks.states[state].arcs) ; gArc ; ++gArc )
                if ( &gArc.data() != min )
                    (heapI++)->p = &gArc.data();
            assert(heapI == heapStart + heapSize);
            heapBuild(heapStart, heapStart + heapSize);
        } else
            pathGraph[state]->arcHeap = NULL;
        pathGraph[state] = newTreeHeapAdd(prev, pathGraph[state]);
    } else
        pathGraph[state] = prev;
}

void printTree(GraphHeap *t, int n) {
    int i;
    for ( i = 0 ; i < n ; ++i ) std::cout << ' ';
    if ( !t ) {
        std::cout << "-\n";
        return;
    }
    std::cout << *t->arc;
    std::cout << " [";
    pGraphArc *heap = t->arcHeap;
    for ( i = 0 ; i < t->arcHeapSize ; ++i ) {
        std::cout << *heap[i].p;
    }
    std::cout << "]\n";
    if ( !t->left && !t->right )
        return;
    printTree(t->left, n+1);
    printTree(t->right, n+1);
}

void shortPrintTree(GraphHeap *t)
{
    std::cout << *t->arc;
    if ( !t->left && !t->right )
        return;
    std::cout << " (";
    if ( t->left) {
        shortPrintTree(t->left);
        if ( t->right ) {
            std::cout << ' ';
            shortPrintTree(t->right);
        }
    } else
    if ( t->right )
        shortPrintTree(t->right);
    std::cout << ')';
}
