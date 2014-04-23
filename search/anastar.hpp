
#include "search.hpp"
#include "anyprof/profile.hpp"
#include "../structs/binheap.hpp"
#include "../utils/pool.hpp"
#include <limits>
#include <vector>
#include <cmath>
#include <cerrno>
#include <string>

void dfrowhdr(FILE*, const char*, unsigned int ncols, ...);
void dfrow(FILE*, const char*, const char*, ...);

template <class D> struct Anastar : public SearchAlgorithm<D> {
  typedef typename D::State State;
  typedef typename D::PackedState PackedState;
  typedef typename D::Cost Cost;
  typedef typename D::Oper Oper;

  struct Node {
    ClosedEntry<Node, D> closedent;
    int openind;
    Node *parent;
    PackedState state;
    Oper op, pop;
    Cost g, h; 
    double potential;

    Node() : openind(-1){
    }

    static ClosedEntry<Node, D> &closedentry(Node *n) {
      return n->closedent;
    }

    static PackedState &key(Node *n) {
      return n->state;
    }

    static void setind(Node *n, int i) {
      n->openind = i;
    }

    static int getind(const Node *n) {
      return n->openind;
    }

    static bool pred(Node *a, Node *b) {
      //      printf("%f %f %f vs ", (double)a->g, (double)a->h, a->potential);
      //      printf("%f %f %f\n", (double)b->g, (double)b->h, b->potential);
      if (a->potential == b->potential)
        return a->h < b->h;
      return a->potential < b->potential;
    }
  };

  Anastar(int argc, const char *argv[]) :
    SearchAlgorithm<D>(argc, argv), closed(30000001), cost(-1) {
    nodes = new Pool<Node>();
    if(cost == -1){
      cost = std::numeric_limits<double>::infinity(); // per skiesel
    }
  }

  ~Anastar() {
    delete nodes;
  }

  static void updatePotential(Node *a, double cost){
    if(cost == std::numeric_limits<double>::infinity()){
      a->potential = a->h;
      //      printf("setting potential to a number: %f\n", (double)a->h);
    }else if(a->g != cost){
      //      printf("setting potential to a number: %f %f %f", cost, (double) a->h, (double) a->g);
        a->potential = a->h / (cost - a->g);
        //printf("%f\n", a->potential);
      }else{
      //printf("setting potential to 0!\n");
      a->potential = 0;
      }
  }

  // rowhdr outputs the incumbent solution row header line.
  void rowhdr() {
    dfrowhdr(stdout, "incumbent", 7, "num", "nodes expanded",
             "nodes generated", "weight", "solution bound", "solution cost",
             "wall time");
  }

  // row outputs an incumbent solution row.
  void row(unsigned long n, double epsprime) {
    dfrow(stdout, "incumbent", "uuugggg", n, this->res.expd,
          this->res.gend, 1, epsprime, cost,
          walltime() - this->res.wallstart);
  }


  void search(D &d, typename D::State &s0) {
    this->rowhdr();
    this->start();
    closed.init(d);
    unsigned long numSols = 0;
    Node *n0 = init(d, s0);
    closed.add(n0);
    open.push(n0);

    while (!open.empty() && !SearchAlgorithm<D>::limit()) {
      Node *n = *open.pop();
      State buf, &state = d.unpack(buf, n->state);
      if(n->g >= cost) continue; // just skip nodes that are no better than incumbent

      if (d.isgoal(state)) {
        numSols++;
        solpath<D, Node>(d, n, this->res);
        cost = n->g; // Update incumbent cost appropriately
        this->row(numSols, cost);
        for(long i = 0; i < open.size(); i++){ // update all potential values
          Node *n = open.at(i);
          updatePotential(n,cost);
        }

        open.reinit(); // re-heapify according to new potential values

      }else{
        //no reason to expand past a goal state, so put expand here
        expand(d, n, state);
      }
    }
    this->finish();
  }

  virtual void reset() {
    SearchAlgorithm<D>::reset();
    open.clear();
    closed.clear();
    delete nodes;
    nodes = new Pool<Node>();
  }

  //lifted from wA*, maybe I want this to be more like ARA*? JTT
  virtual void output(FILE *out) {
    SearchAlgorithm<D>::output(out);
    closed.prstats(stdout, "closed ");
    dfpair(stdout, "open list type", "%s", "binary heap");
    dfpair(stdout, "node size", "%u", sizeof(Node));
  }

  void expand(D &d, Node *n, State &state) {
    SearchAlgorithm<D>::res.expd++;

    typename D::Operators ops(d, state);
    for (unsigned int i = 0; i < ops.size(); i++) {
      if (ops[i] == n->pop)
        continue;
      SearchAlgorithm<D>::res.gend++;
      considerkid(d, n, state, ops[i]);
    }
  }

  void considerkid(D &d, Node *parent, State &state, Oper op) {
    Node *kid = nodes->construct();
    typename D::Edge e(d, state, op);
    kid->g = parent->g + e.cost;
    d.pack(kid->state, e.state);

    unsigned long hash = kid->state.hash(&d);
    Node *dup = static_cast<Node*>(closed.find(kid->state, hash));
    if (dup) {
      this->res.dups++;
      if (kid->g >= dup->g) {
        nodes->destruct(kid);
        return;
      }
      if (dup->openind < 0)
        this->res.reopnd++;
      
      updatePotential(dup, cost);
      dup->g = kid->g;
      dup->parent = parent;
      dup->op = op;
      dup->pop = e.revop;
      open.pushupdate(dup, dup->openind);
      nodes->destruct(kid);
    } else {
      typename D::Cost h = d.h(e.state);
      kid->h = h;
      kid->parent = parent;
      kid->op = op;
      kid->pop = e.revop;
      closed.add(kid, hash);
      updatePotential(kid, cost);
      open.push(kid);
    }
  }

  Node *init(D &d, State &s0) {
    Node *n0 = nodes->construct();
    d.pack(n0->state, s0);
    n0->g = Cost(0);
    typename D::Cost h = d.h(s0);
    n0->h = h;
    updatePotential(n0, cost);
    n0->op = n0->pop = D::Nop;
    n0->parent = NULL;
    return n0;
  }

  BinHeap<Node, Node*> open;
  ClosedList<Node, Node, D> closed;
  Pool<Node> *nodes;
  double cost;	// solution cost, defines node potential
};
