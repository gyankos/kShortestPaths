/* k Shortest Paths in O(E*log V + L*k*log k) time (L is path length)
   Implemented by Jon Graehl (jongraehl@earthling.net)
   Following David Eppstein's "Finding the k Shortest Paths" March 31, 1997 draft
   (http://www.ics.uci.edu/~eppstein/
    http://www.ics.uci.edu/~eppstein/pubs/p-kpath.ps)
   */


#include "kbest.h"
#include <strstream>
#include <iostream>

int main(int argc, char *argv[])
{
    if ( argc != 4 ) {
        std::cout << "Test of k best paths algorithm.  Supply three integer arguments (source state number, destination state number, number of paths) and a graph to stdin.  See readme.txt, sample.graph, or ktest.cc for the graph format.\n";
        return 0;
    }
    int k;
    int source, dest;
    std::istrstream sstr(argv[1]);
    std::istrstream dstr(argv[2]);
    std::istrstream kstr(argv[3]);

    if ( !((sstr >> source) && (dstr >> dest) && (kstr >> k)) ) {
        std::cerr << "Bad argument (should be integer) - aborting.\n";
        return -1;
    }

    Graph graph;
    std::cin >> graph;
    List<List<GraphArc *> > *paths = bestPaths(graph, source, dest, k);
    for ( ListIter<List<GraphArc *> > pathIter(*paths) ; pathIter ; ++pathIter ) {
        float pathWeight = 0;
        GraphArc *a;
        for ( ListIter<GraphArc *> arcIter(pathIter.data()) ; arcIter ; ++arcIter ) {
            a = arcIter.data();
            pathWeight += a->weight;
            std::cout << *a << ' ';
        }
        std::cout << pathWeight << '\n';
    }
    //  cout << graph;
    return 0;
}
