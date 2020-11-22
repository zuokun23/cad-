#pragma once
//
//
//

#ifndef UNTITLED_HALFEDGE_H
#define UNTITLED_HALFEDGE_H

#pragma once
#include <list>
#include <array>
#include <tuple>
#include <string>

using namespace std;

struct Solid;
struct Face;
struct Loop;
struct HalfEdge;
struct Edge;
struct Vertex;

using U32 = unsigned int;
using I32 = int;

struct Coord
{
    float x, y, z;
};

struct Function
{
    float x, y, z, s;
};

struct Vertex
{
    //	HalfEdge* halfEdge = nullptr;
    Coord coord;
};

struct Edge
{
    HalfEdge* halfEdge0 = nullptr;
    HalfEdge* halfEdge1 = nullptr;
};

struct HalfEdge
{

    Loop* parentLoop = nullptr;
    Edge* parentEdge = nullptr;
    Vertex* vertex = nullptr;

    HalfEdge* next = nullptr;
    HalfEdge* pre = nullptr;

    HalfEdge* mate();
};

struct Loop
{
    Face* parentFace = nullptr;
    HalfEdge* leadingHalfEdge = nullptr;

    HalfEdge* find(Vertex* vertex);
};

struct Face
{
    Solid* parentSolid = nullptr;
    Loop* outerLoop = nullptr;
    list<Loop*>	innerLoops;
    //	Function			function;
};

struct Solid
{
    list<Face*>	faces;
    list<Edge*>	edges;
    list<Vertex*>	vertices;
};

tuple<Vertex*, Face*, Solid*> mvfs(const Coord& coord);
// may change the leading halfedge of loop
tuple<Edge*, Vertex*> mev(Vertex* vertex0, Loop* loop, Coord coord);
tuple<Edge*, Face*> mef(Loop* loop, Vertex* vertex0, Vertex* vertex1);
tuple<Loop*> kemr(Vertex* vertex0, Vertex* vertex1, Loop* loop0);
void kfmrh(Loop* outerLoop, Loop* innerLoop);
void sweep(Face* face, Coord dir);
void Print(Solid* solid, const std::string& path);


#endif //UNTITLED_HALFEDGE_H
