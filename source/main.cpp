/* k Shortest Paths in O(E*log V + L*k*log k) time (L is path length)
   Implemented by Jon Graehl (jongraehl@earthling.net)
   Following David Eppstein's "Finding the k Shortest Paths" March 31, 1997 draft
   (http://www.ics.uci.edu/~eppstein/
    http://www.ics.uci.edu/~eppstein/pubs/p-kpath.ps)
   */


#include <kshortestpaths/kbest.h>
#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[])
{
    if ( argc != 4 ) {
        std::cout << "Test of k best paths algorithm.  Supply three integer arguments (source state number, destination state number, number of paths) and a graph to stdin.  See readme.txt, sample.graph, or ktest.cc for the graph format.\n";
        return 0;
    }
    int k;
    int source, dest;

    char* p;
    source = (int)strtol(argv[1], &p, 10);
    if (*p) {
        std::cerr << "Bad source (should be integer) - aborting.\n";
    }
    dest = (int)strtol(argv[2], &p, 10);
    if (*p) {
        std::cerr << "Bad target (should be integer) - aborting.\n";
    }
    k = (int)strtol(argv[3], &p, 10);
    if (*p) {
        std::cerr << "Bad k parameter (should be integer) - aborting.\n";
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
