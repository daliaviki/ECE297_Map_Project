/***********************************************
 * Put every extra function inside this header *
 ***********************************************/

#pragma once //protects against multiple inclusions of this header file
#include "structs_and_classes.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include <map>

// it converts a pair of LatLon to a pair of XY_
std::pair<XY_,XY_> LatLon_to_XY(std::pair<LatLon,LatLon> points);
std::vector<XY_> LatLon_to_XY(std::vector<LatLon> points);

// a vector[IntersectionIndex], each intersection vector stores StreetSegID ints
extern std::vector<std::vector<int>> intersection_street_segments;

// a vector[StreetIndex], each street vector stores streetSegmentIDs
extern std::vector<std::vector<int>> street_street_segments;

// a vector[StreetSegIndex], each stores streetIndex
extern std::vector<int> street_segments_street;

// a vector[StreetIndex], each street vector stores IntersectionIDs
extern std::vector<std::vector<int>> street_intersections;

// a vector[streetSegIndex], each element stores distance
extern std::vector<double> streetSeg_length;

// a vector[streetSegIndex], each element stores travel time
extern std::vector<double> streetSeg_time;

// a map <Street_Name,StreetIndex> for all streets
extern std::multimap<std::string, int> streetID_streetName;

// a vector[streetIndex], each element stores distance
extern std::vector<double> streetID_streetLength;

// a variable used to store all possible all features
extern std::vector<std::vector<LatLon>> featureID_featurePts;

// a map, each stores <node ID, node>
extern std::unordered_map<OSMID, const OSMNode*> NodeID_Node;

// a map, each stores <way ID, way>
extern std::unordered_map<OSMID, const OSMWay*> WayID_Way;

// a map, each stores <way ID, way length>
extern std::unordered_map<OSMID, double> WayID_length;

// the maximum speed_limit in the city
extern double max_speed_limit ;

int find_closest_POI(LatLon my_position);

void pathFind_load();

// a vector[Node_idx] storing Nodes
// Node_idx is the same as intersection idx
extern std::vector<Node> nodes;

// a vector[Edge_idx] storing Edges
// Edge_idx is not the same as segment idx
extern std::vector<Edge> edges;

// a vector[Node_idx] storing Nodes for walking
// Node_idx is the same as intersection idx
extern std::vector<Node> nodes_w;

// a vector[Edge_idx] storing Edges for walking
// Edge_idx is not the same as segment idx
extern std::vector<Edge> edges_w;