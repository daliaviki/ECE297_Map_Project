/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   m2_more.h
 * Author: wangbox2
 *
 * Created on February 17, 2020, 2:22 PM
 */

#pragma once
#include "m1.h"
#include "m1_more.h"
#include "m3.h"
#include "m3_more.h"
#include "structs_and_classes.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "math.h"
#include "rectangle.hpp"
#include "graphics.hpp"
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>    // std::sort
#include <iostream>
#include <fstream>
/************ These are the data structures that will be used (and not constantly updated) ***************/

// a vector[intersection_idx] storing intersection data
extern std::vector<intersection_info> intersections;

// a vector[streetSegment_idx] storing street_segment_data
extern std::vector<segment_info> streetSegments;

// a vector[features_idx] storing features_data
extern std::vector<feature_info> features;

// unordered_map storing poi_data
extern std::unordered_map<int,poi_info> POIs_entertainment;
extern std::unordered_map<int,poi_info> POIs_food;
extern std::unordered_map<int,poi_info> POIs_public_gathering;
extern std::unordered_map<int,poi_info> POIs_other; 

// vector[] storing all the highway_info that are major/medium/minor
extern std::vector<highway_info> highways_major; 
extern std::vector<highway_info> highways_medium;
extern std::vector<highway_info> highways_minor; 

// a vector[way_index] storing all the tags
extern std::unordered_map<OSMID, std::unordered_map<std::string,std::string>> WayID_tags;

// a vector[node_index] storing Node IDs and nodes
extern std::unordered_map<OSMID, const OSMNode*> NodeID_node;

// a multimap storing all the railway: subways
extern std::multimap<std::string, railway_info> railways_subways;

// a vector[streetSegIndex], each element stores distance. from m1_more.h
extern std::vector<double> streetSeg_length;

// a vector[IntersectionIndex], each intersection vector stores StreetSegID ints
extern std::vector<std::vector<int>> intersection_street_segments;

// a vector[StreetIndex], each street vector stores streetSegmentIDs
extern std::vector<std::vector<int>> street_street_segments;

// a vector[streetIndex], each element stores distance
extern std::vector<double> streetID_streetLength;

// a variable used to store all possible all features
extern std::vector<std::vector<LatLon>> featureID_featurePts;

// a vector[StreetIndex], each street vector stores IntersectionIDs
extern std::vector<std::vector<int>> street_intersections;

// a map <Street_Name,StreetIndex> for all streets
extern std::multimap<std::string, int> streetID_streetName;

// In Degrees: max_lat, min_lat, max_lon, min_lon
// In Radians: avg_lat
extern double max_lat, min_lat, max_lon, min_lon, avg_lat;

// it stores the value of initial world size
extern ezgl::rectangle initial_world;

/************ These are functions called in program ***************/

// it prevents zooming out of bound
void out_of_bound_prevention(ezgl::renderer *g);

// load all the data structures
void draw_map_load ();

// draw main canvas and call all the following drawing functions
void draw_main_canvas (ezgl::renderer *g);

// draw all intersection points
void draw_intersections (ezgl::renderer *g);

void draw_street_segments(int i, ezgl::renderer *g);

// draw all highways (all the streets/roads)
void draw_all_highways(ezgl::renderer *g);

// draw all railways (only subway lines)
void draw_all_railways(ezgl::renderer *g);

// draw all the features (buildings rivers streams etc)
void draw_features(ezgl::renderer *g);

// draw all the points of interests
void draw_points_of_interests(ezgl::renderer *g);

// draw all the street names
void draw_street_names(ezgl::renderer *g);

// draw street sgements directions
void draw_street_segments_directions (ezgl::renderer *g);

// draw the path found in the path finding utilities
void draw_path_found(ezgl::renderer *g);

// print the path found in the path finding utilities
void print_path_found();

// Converting latlon in degrees to x and y
float x_from_lon(float lon);
float y_from_lat(float lat);

// Converting x and y to latlon in degrees
float lat_from_y(double y);
float lon_from_x(double x);

/****** stores bunch of global variables that will be used and constantly updated ********/
struct action_mem{
    // store intersections last highlighted (those two above should not be used)
    std::vector<int> last_highlighted_intersections;
    // store last highlighted poi due to click
    int last_clicked_POI=-1;
    // entries recorder for pop up window
    std::vector<int> last_entry;
    // auto completion list recorder
    std::unordered_map<std::string, int> last_autocompletion_list;
    // last visible world, might be removed later
    ezgl::rectangle last_visible_world;
    // display railway
    bool layer_railway_subway=false;
    // display all water bodies at all zoom level
    bool layer_water_body=false;
    // display all poi
    bool layer_poi = false;
    // display directions
    std::vector<int> streets_with_directions;
    // path finding mode:
    //  0 = OFF
    //  1 = ON, driving
    //  2 = ON, walking + driving
    int path_finding_state=0;
    // path finding intersections: starting point and end point
    std::vector<int> path_finding_intersections;
    // path found
    std::vector<int> path_found;
    std::vector<int> walk_path_found;
};

extern action_mem memory; 

/****** These are callback functions when certain actions are performed ********/

// initial setup when app is opened
void initial_setup(ezgl::application *application, bool new_window);

// events triggered by mouse clicks
void act_on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y);

// events triggered by keyboard 
void act_on_key_press(ezgl::application* application, GdkEventKey* event, char *key_name);

// Find button callback function, activate pop up window
void FindButton_callback(GtkButton* widget, ezgl::application *application);

// callback function triggered by "Return" button released on pop up entry callback
void StreetsEntryReturn_callback(GtkEntry* widget, GdkEventKey* event, ezgl::application *application);

// callback functio text changed on pop up entry, it updates the auto completion
void StreetsEntryChange_callback(GtkEntry* widget, ezgl::application *application);

// it updates the global bool variable in memory, so that railways are drawn
void Railways_CheckButton_callback(GtkToggleButton* widget, ezgl::application *application);

// it updates the global bool variable in memory, so that motorway are highlighted
void Motorways_CheckButton_callback(GtkToggleButton* widget, ezgl::application *application);

// it updates the global bool variable in memory, so that water body are highlighted
void WaterBody_CheckButton_callback(GtkToggleButton* widget, ezgl::application *application);

// it updates the global bool variable in memory, so that all pois are highlighted
void POI_CheckButton_callback(GtkToggleButton* widget, ezgl::application *application);

// it updates the global bool variable in memory, so that all pois are highlighted
void Directions_CheckButton_callback(GtkToggleButton* widget, ezgl::application *application);

// done searching streets, reflect on the canvas
void IntersectionsSearchResult(std::vector<int> intersections_found, ezgl::application *application);

// it forces the auto completion to always show the correct list we stored
gboolean forced_auto_completion(GtkEntryCompletion *completion, const gchar *key, GtkTreeIter *iter, gpointer user_data);

// it updates label of the path_find button
void PathFind_Button_callback(GtkButton* widget, ezgl::application *application);

// Test callback, feel free to modify and use it for any testing
void Test_callback(GtkEntry* widget, ezgl::application *application);
