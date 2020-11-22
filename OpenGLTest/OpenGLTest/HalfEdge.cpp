//
// 
//

#include <cassert>
#include <fstream>
#include <vector>
#include <map>
#include <iterator>
#include "HalfEdge.h"

using namespace std;

tuple<Vertex*, Face*, Solid*> mvfs(const Coord& coord)
{
    auto solid = new Solid;
    auto face = new Face;
    auto loop = new Loop;
    auto vertex = new Vertex;

    solid->faces.push_back(face);
    face->parentSolid = solid;

    face->outerLoop = loop;
    loop->parentFace = face;

    vertex->coord = coord;
    solid->vertices.push_back(vertex);

    return make_tuple(vertex, face, solid);
}

tuple<Edge*, Vertex*> mev(Vertex* vertex0, Loop* loop, Coord coord)
{
    auto vertex1 = new Vertex;
    auto edge = new Edge;
    auto halfEdge0 = new HalfEdge;
    auto halfEdge1 = new HalfEdge;

    vertex1->coord = coord;
    halfEdge0->vertex = vertex0;
    halfEdge1->vertex = vertex1;

    halfEdge0->next = halfEdge1;
    halfEdge1->pre = halfEdge0;

    halfEdge0->parentEdge = edge;
    halfEdge1->parentEdge = edge;
    halfEdge0->parentLoop = loop;
    halfEdge1->parentLoop = loop;

    edge->halfEdge0 = halfEdge0;
    edge->halfEdge1 = halfEdge1;

    if (loop->leadingHalfEdge == nullptr) {
        loop->leadingHalfEdge = halfEdge0;
        halfEdge0->pre = halfEdge1;
        halfEdge1->next = halfEdge0;
    }
    else {
        auto tmp = loop->find(vertex0);
        halfEdge0->pre = tmp->pre;
        halfEdge1->next = tmp;

        halfEdge0->pre->next = halfEdge0;
        halfEdge1->next->pre = halfEdge1;
    }

    auto solid = loop->parentFace->parentSolid;
    solid->edges.push_back(edge);
    solid->vertices.push_back(vertex1);

    return make_tuple(edge, vertex1);
}

tuple<Edge*, Face*> mef(Loop* loop0, Vertex* vertex0, Vertex* vertex1)
{
    auto solid = loop0->parentFace->parentSolid;
    auto face = new Face;
    auto loop1 = new Loop;
    auto edge = new Edge;
    auto halfEdge0 = new HalfEdge;
    auto halfEdge1 = new HalfEdge;

    // set face
    solid->faces.push_back(face);
    face->parentSolid = loop0->parentFace->parentSolid;
    face->outerLoop = loop1;
    // set edge
    solid->edges.push_back(edge);
    edge->halfEdge0 = halfEdge0;
    edge->halfEdge1 = halfEdge1;
    // set loop1
    loop1->parentFace = face;

    // find tmp half edges
    auto tmphe0 = loop0->find(vertex0);
    auto tmphe1 = loop0->find(vertex1);

    // link two new half edges to loop
    halfEdge0->next = tmphe1;
    halfEdge0->pre = tmphe0->pre;
    halfEdge1->next = tmphe0;
    halfEdge1->pre = tmphe1->pre;

    tmphe0->pre->next = halfEdge0;
    tmphe1->pre->next = halfEdge1;
    tmphe0->pre = halfEdge1;
    tmphe1->pre = halfEdge0;

    // set two half edges
    halfEdge0->parentLoop = loop0;
    halfEdge0->parentEdge = edge;
    halfEdge1->parentLoop = loop1;
    halfEdge1->parentEdge = edge;
    halfEdge0->vertex = vertex0;
    halfEdge1->vertex = vertex1;
    // maintain two loop
    loop1->leadingHalfEdge = halfEdge1;
    loop0->leadingHalfEdge = halfEdge0;
    return make_tuple(edge, face);
}

tuple<Loop*> kemr(Vertex* vertex0, Vertex* vertex1, Loop* loop0)
{
    auto solid = loop0->parentFace->parentSolid;
    auto loop1 = new Loop;

    // find corresponding half edges
    HalfEdge* halfEdge0 = loop0->leadingHalfEdge;
    HalfEdge* halfEdge1 = halfEdge0->next;
    while (halfEdge0->vertex != vertex0 || halfEdge1->vertex != vertex1) {
        halfEdge0 = halfEdge1;
        halfEdge1 = halfEdge0->next;
    }

    // set the original loop
    loop0->leadingHalfEdge = halfEdge1->next;
    halfEdge0->pre->next = halfEdge1->next;
    halfEdge1->next->pre = halfEdge0->pre;
    // set new loop
    loop1->parentFace = loop0->parentFace;
    loop1->parentFace->innerLoops.push_back(loop1);
    // delete edge
    auto edge = halfEdge0->parentEdge;
    solid->edges.remove(edge);
    delete edge;
    // delete two halfedges
    delete halfEdge0;
    delete halfEdge1;
    // return the new loop
    return std::make_tuple(loop1);
}

void kfmrh(Loop* outerLoop, Loop* innerLoop)
{
    auto solid = outerLoop->parentFace->parentSolid;
    outerLoop->parentFace->innerLoops.push_back(innerLoop);
    delete innerLoop->parentFace;
    solid->faces.remove(innerLoop->parentFace);
    innerLoop->parentFace = outerLoop->parentFace;
}

Coord operator + (Coord c0, Coord c1)
{
    c0.x += c1.x;
    c0.y += c1.y;
    c0.z += c1.z;
    return c0;
}

void sweep(Face* face, Coord dir)
{
    auto extrude = [](Loop* loop, Coord dir)
    {
        auto endVertex = loop->leadingHalfEdge->vertex;
        auto he = loop->leadingHalfEdge;

        auto oriRet = mev(he->vertex, loop, he->vertex->coord + dir);
        he = he->next;
        auto lastRet = oriRet;
        while (he->vertex != endVertex) {
            auto ret = mev(he->vertex, loop, he->vertex->coord + dir);
            mef(loop, std::get<1>(lastRet), std::get<1>(ret));
            lastRet = ret;
            he = he->next;
        }
        mef(loop, std::get<1>(lastRet), std::get<1>(oriRet));
    };

    extrude(face->outerLoop, dir);
    for (auto loop : face->innerLoops)
        extrude(loop, dir);
}

void Print(Solid* solid, const std::string& path)
{
    ofstream ofs{ path };


    vector<Vertex*> vertVec;
    for (auto vertex : solid->vertices) {
        vertVec.push_back(vertex);
    }
    vector<Loop*> loopVec;
    for (auto face : solid->faces) {
        loopVec.push_back(face->outerLoop);
        for (auto loop : face->innerLoops)
            loopVec.push_back(loop);
    }

    // header
    ofs << "brp" << std::endl;
    // size
    /*
    ofs << vertVec.size() << ' '
        << loopVec.size() << ' '
        << solid->faces.size() << ' '
        << 1 << std::endl;
      */
    // vertices
    for (auto itr = vertVec.begin(); itr != vertVec.end(); itr++) {
        auto vert = *itr;
        ofs << vert->coord.x << ' '
            << vert->coord.y << ' '
            << vert->coord.z
            << std::endl;
    }
    // loops
    ofs << "loop" << std::endl;

    for (auto itr = loopVec.begin(); itr != loopVec.end(); itr++) {
        auto loop = *itr;

        auto he = loop->leadingHalfEdge;
        ofs << distance(vertVec.begin(), std::find(vertVec.begin(), vertVec.end(), he->vertex)) << ' ';
        he = he->next;
        while (he != loop->leadingHalfEdge) {
            ofs << distance(vertVec.begin(), std::find(vertVec.begin(), vertVec.end(), he->vertex)) << ' ';
            he = he->next;
        }
        ofs << endl;
    }
    // faces
    /*
    for (auto face : solid->faces) {
        ofs << std::distance(loopVec.begin(), std::find(loopVec.begin(), loopVec.end(), face->outerLoop)) << ' '
            << face->innerLoops.size() << ' ';
        for (auto loop : face->innerLoops) {
            ofs << std::distance(loopVec.begin(), std::find(loopVec.begin(), loopVec.end(), loop)) << ' ';
        }
        ofs << 0 << std::endl;
    }*/

}

HalfEdge* Loop::find(Vertex* vertex)
{
    auto he = leadingHalfEdge;
    while (he) {
        if (he->vertex == vertex) return he;
        he = he->next;
        if (he == leadingHalfEdge) break;
    }
    return nullptr;
}

HalfEdge* HalfEdge::mate()
{
    return parentEdge->halfEdge0 == this ? parentEdge->halfEdge1 : parentEdge->halfEdge0;
}
