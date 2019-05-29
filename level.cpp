#include <fstream>
#include <map>
#include "level.h"

void saveLevel(Level& level) {
  std::map<Point*, int> pointIds;
  std::vector<Point*> pointsById;
  int count = 0;
  for (Face* face: level.faces) {
    for (Triangle* triangle: face->triangles) {
      for (Edge* edge: triangle->edges) {
        Point* point = edge->a;
        if (pointIds.find(point) == pointIds.end()) {
          pointIds[point] = count;
          count++;
          pointsById.push_back(point);
        }
      }
    }
  }
  std::ofstream file("data/level.txt");
  file << pointIds.size() << std::endl;
  for (const auto& p: pointsById) {
    file << p->x << " " << p->y << std::endl;
  }
  file << level.faces.size() << std::endl;
  for (Face* face: level.faces) {
    file << face->textureId << " ";
    file << face->textureScale << " ";
    file << face->textureX << " ";
    file << face->textureY << " ";
    file << face->triangles.size();
    for (Triangle* triangle: face->triangles) {
      for (Edge* edge: triangle->edges) {
        Point* point = edge->a;
        file << " " << pointIds[point];
      }
    }
    file << std::endl;
  }
}

void loadLevel(Level& level) {
  std::ifstream file("data/level.txt");
  int numPoints;
  file >> numPoints;
  std::vector<Point*> points;
  for (int i=0; i<numPoints; i++) {
    int x;
    int y;
    file >> x >> y;
    points.push_back(new Point(x, y));
  }
  int numFaces;
  file >> numFaces;
  std::map<std::pair<Point*, Point*>, Edge*> edgeMap;
  for (int i=0; i<numFaces; i++) {
    int textureId;
    int numTriangles;
    float textureScale;
    float textureX;
    float textureY;
    file >> textureId >> textureScale >> textureX >> textureY >> numTriangles;
    std::vector<Point*> pointsInPoly;
    for (int j=0; j<numTriangles*3; j++) {
      int pointId;
      file >> pointId;
      pointsInPoly.push_back(points[pointId]);
    }
    Face* face = new Face;
    face->textureId = textureId;
    face->textureScale = textureScale;
    face->textureX = textureX;
    face->textureY = textureY;
    for (int triId=0; triId<numTriangles; triId++) {
      Triangle* triangle = new Triangle();
      for (int pointId=0; pointId<3; pointId++) {
        Point* a = pointsInPoly[triId*3 + pointId];
        Point* b;
        if (pointId == 2) {
          b = pointsInPoly[triId*3]; // wrap back to start
        }
        else {
          b = pointsInPoly[triId*3 + pointId + 1];
        }
        auto key = std::make_pair(a, b);
        Edge* edge;
        auto it = edgeMap.find(key);
        if (it == edgeMap.end()) {
          edge = new Edge(*a, *b);
          edgeMap.insert(std::make_pair(key, edge));
        }
        else {
          edge = it->second;
          edge->shared = true;
        }
        triangle->edges.push_back(edge);
      }
      face->triangles.push_back(triangle);
    }
    level.faces.push_back(face);
  }
}


