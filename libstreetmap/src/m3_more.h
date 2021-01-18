/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   m3_more.h
 * Author: wangbox2
 *
 * Created on March 12, 2020, 10:32 AM
 */

#pragma once //protects against multiple inclusions of this header file
#include "structs_and_classes.h"
#include "m1.h"
#include "m1_more.h"
#include "m2.h"
#include "m2_more.h"
#include <tuple>
#include <queue> //std::priority_queue
#include <unordered_set>
#include <map>
#include <chrono>
#include <thread>
/************ These are the data structures that will be used ***************/

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

// a vector[node_idx] storing nodes
// node idx == intersection indx
extern std::vector<intersection_info> intersections;

// a vector[edge_idx] storing edges
// edge idx != segment indx
extern std::vector<segment_info> streetSegments;

// a vector[streetSegIndex], each element stores distance. from m1_more.h
extern std::vector<double> streetSeg_length;

// a vector[streetSegment_idx] storing street_segment_data
extern std::vector<segment_info> streetSegments;

// street segment and time
extern std::vector<double> streetSeg_time;

// the maximum speed_limit in the city
extern double max_speed_limit ;

/************ These are functions called in program ***************/

std::vector<StreetSegmentIndex> find_path_between_intersections_visualized(
		          const IntersectionIndex intersect_id_start, 
                          const IntersectionIndex intersect_id_end,
                          const double turn_penalty,
                          ezgl::application *application);

std::pair<std::vector<StreetSegmentIndex>, std::vector<StreetSegmentIndex>> 
         find_path_with_walk_to_pick_up_visualized(
                          const IntersectionIndex start_intersection, 
                          const IntersectionIndex end_intersection,
                          const double turn_penalty,
                          const double walking_speed, 
                          const double walking_time_limit,
                          ezgl::application *application);
        
// This function is similar to find_path_between_intersections() except it takes
// multiple starting point instead of one.
// It returns a pair of a vector and an int
// The vector is the shortest path (route) between all the start intersections 
// and intersect_id_end
// The int is one of the start_intersection that correlated to the path returned.
std::pair<std::vector<StreetSegmentIndex>,int> find_path_between_intersections_multi_starts(
                                                const std::vector<int> intersect_ids_start, 
                                                const IntersectionIndex intersect_id_end,
                                                const double turn_penalty,
                                                ezgl::application *application);

// This function is similar to find_path_between_intersections() except it takes
// multiple end points instead of one.
// It returns a multimap of < a double, <a vector, an int, a double>>
// multimap<travel_time, tuple<path, end_inter_idx, end_deliv_or_depot_idx, pORd, delivORdepot>>
std::multimap<double, std::tuple<std::vector<StreetSegmentIndex>,int,int,bool,bool>> find_path_between_intersections_multi_ends(
                                                const IntersectionIndex intersect_id_start, 
                                                const std::unordered_multimap<int, std::tuple<int,bool,bool>> intersect_ids_end,
                                                const double turn_penalty);

// This function is similar to find_path_between_intersections() except it takes
// multiple start points end points instead of one.
// multimap<travel_time, tuple<path, end_inter_idx, end_deliv_idx, pORd, start_inter_idx, delivORdepot>>
std::multimap<double, std::tuple<std::vector<StreetSegmentIndex>,int,int,bool,int,bool>> find_path_between_intersections_multi_starts_ends(
                                                const std::vector<int> intersect_ids_start, 
                                                const std::unordered_multimap<int, std::tuple<int,bool,bool>> intersect_ids_end,
                                                const double turn_penalty);

// it traces back all the street segments that are driven through starting from 
// the intersect_id_end.
// The function returns the indicis of the street segments, the starting
// point correlated to the path, and the total time of the path.
std::tuple<std::vector<StreetSegmentIndex>,int,double> path_search_result(const IntersectionIndex intersect_id_end);

// it traces back all the street segments that are walked through starting from 
// the intersect_id_end.
// The function returns the indicis of the street segments
std::vector<StreetSegmentIndex>                path_walk_search_result(const IntersectionIndex intersect_id_end);


// This function uses Dijkstra algorithm to find all the shortest path to find
// all the shortest paths  from the parameter start_intersection to all the 
// walkable intersections within the distance = walking_speed * walking_time_limit. 
// The function returns the vector of indicis of those walkable intersections.
 std::vector<int>               find_walkable_inters( const IntersectionIndex start_intersection,
                                                      const double turn_penalty,
                                                      const double walking_speed,
                                                      const double walking_time_limit,
                                                      ezgl::application *application);
