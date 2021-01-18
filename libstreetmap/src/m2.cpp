#include "m2.h"
#include "m2_more.h"

// a vector[intersection_idx] storing intersection data
std::vector<intersection_info> intersections;

// a vector[streetSegment_idx] storing street_segment_data
std::vector<segment_info> streetSegments;

// a vector[features_idx] storing features_data
std::vector<feature_info> features;

// unordered_map storing poi_data
std::unordered_map<int,poi_info> POIs_entertainment;
std::unordered_map<int,poi_info> POIs_food;
std::unordered_map<int,poi_info> POIs_public_gathering;
std::unordered_map<int,poi_info> POIs_other; 

// vector[] storing all the highway_info that are major/medium/minor
std::vector<highway_info> highways_major; 
std::vector<highway_info> highways_medium;
std::vector<highway_info> highways_minor; 

// a vector[way_index] storing all the tags
std::unordered_map<OSMID, std::unordered_map<std::string,std::string>> WayID_tags;

// a vector[node_index] storing Node IDs and nodes
std::unordered_map<OSMID, const OSMNode*> NodeID_node;

// a multimap storing all the railway: subways
std::multimap<std::string, railway_info> railways_subways;

// In Degrees: max_lat, min_lat, max_lon, min_lon
// In Radians: avg_lat
double max_lat, min_lat, max_lon, min_lon, avg_lat;

// it stores the value of initial world size
ezgl::rectangle initial_world;

action_mem memory; 

// draw map loads necessary variables and calls draw_map_blank_canvas() in the end.
void draw_map () {
    draw_map_load();
    
    ezgl::application::settings settings; 
    settings.main_ui_resource =  "libstreetmap/resources/main.ui"; 
    settings.window_identifier = "MainWindow"; 
    settings.canvas_identifier = "MainCanvas";

    ezgl::application application(settings); 

    application.add_canvas("MainCanvas", 
                           draw_main_canvas,
                           initial_world,
                           ezgl::BLACK);
    application.run(initial_setup, act_on_mouse_click,
                        nullptr, act_on_key_press);
    return;
}

// load all the data structures
void draw_map_load (){
    intersections.resize(getNumIntersections());
    streetSegments.resize(getNumStreetSegments());
    features.resize(getNumFeatures());
    // POIs.resize(getNumPointsOfInterest());
    
    // double max_lat, min_lat, max_lon, min_lon, avg_lat;
    // memory.initial_world_width
    max_lat = getIntersectionPosition(0).lat();
    min_lat = max_lat;
    max_lon = getIntersectionPosition(0).lon();
    min_lon = max_lon;
    for(int i=0; i<intersections.size(); i++ ) {
        LatLon this_position  = getIntersectionPosition(i);
        max_lat = std::max(max_lat, this_position.lat());
        min_lat = std::min(min_lat, this_position.lat());
        max_lon = std::max(max_lon, this_position.lon());
        min_lon = std::min(min_lon, this_position.lon());
    }
    avg_lat=(max_lat+min_lat)/2.0 * DEGREE_TO_RADIAN;
    
    // ezgl::rectangle initial_world;
    // memory.last_visible_world
    ezgl::rectangle rhs({x_from_lon(min_lon), y_from_lat(min_lat)},
                        {x_from_lon(max_lon), y_from_lat(max_lat)});
    initial_world = rhs;
    memory.last_visible_world = rhs;
    
    // std::vector<intersection_info> intersections;
    for(int i=0; i<intersections.size(); i++ ) {
        LatLon this_position      = getIntersectionPosition(i);
        intersections[i].x_       = x_from_lon(this_position.lon());
        intersections[i].y_       = y_from_lat(this_position.lat());
        intersections[i].name     = getIntersectionName(i);
    }
    
    // std::unordered_map<OSMID, std::unordered_map<std::string,std::string>> WayID_tags;
    for (int way=0; way<getNumberOfWays(); ++way){
        const OSMWay* this_way = getWayByIndex(way);
        std::unordered_map<std::string,std::string> these_tags;
        for(int i=0;i<getTagCount(this_way); ++i)
        {
            these_tags.insert(getTagPair(this_way,i));
        }
        WayID_tags.insert(std::make_pair(this_way->id(), these_tags));
    }
    
    // std::unordered_map<OSMID, OSMNode*> NodeID_node;
    for (int node=0; node<getNumberOfNodes(); ++node){
        const OSMNode* this_node = getNodeByIndex(node);
        NodeID_node.insert(std::make_pair(this_node->id(), this_node));
    }
    
    // std::vector<highway_info> highways_major/medium/minor;
    // std::map<std::string, railway_info> railways_subways;
    for (int way=0; way<getNumberOfWays(); ++way){
        // find all the tags of this way
        const OSMWay* this_way   = getWayByIndex(way);
        OSMID         this_wayID = this_way->id();
        std::unordered_map<std::string,std::string> these_tags = WayID_tags.find(this_wayID)->second;
        // determine if is street
        if (these_tags.find("highway") != these_tags.end()){
            highway_info this_info;
            // find all the points coordinates
            std::vector<OSMID>          all_nodes  = getWayMembers(this_way);
            std::vector<ezgl::point2d>  all_coords_xy;
            for (int i = 0; i < all_nodes.size(); ++i){
                const OSMNode* this_node = NodeID_node.find(all_nodes[i])->second;
                LatLon        coords_latlon = getNodeCoords(this_node);
                ezgl::point2d coords_xy(x_from_lon(coords_latlon.lon()),
                                        y_from_lat(coords_latlon.lat()));
                all_coords_xy.push_back(coords_xy);
            }
            this_info.allPoints = all_coords_xy;
            // find one way or not
            auto oneway_iter = these_tags.find("oneway");
            if (oneway_iter != these_tags.end() && oneway_iter->second == "yes"){
                this_info.oneWay = true;
            }
            else{
                this_info.oneWay = false;
            }
            // determine major_minor
            std::string highway_tag = these_tags.find("highway")->second;
            int     major_minor = 0;
            if      (highway_tag == "motorway"      || highway_tag == "trunk"           || 
                     highway_tag == "primary"       || highway_tag == "secondary")              { major_minor = 2; }
            else if (highway_tag == "motorway_link" || highway_tag == "trunk_link"      || 
                     highway_tag == "primary_link"  || highway_tag == "secondary_link"  ||
                     highway_tag == "tertiary"      || highway_tag == "tertiary_link")          { major_minor = 1; }
            else                                                                                { major_minor = 0; }
            // store rest the data in one highway_info
            this_info.tags      = these_tags;
            this_info.wayOSMID  = this_wayID;
            // insert the struct into the variable
            if      (major_minor==2)                                                            { highways_major.push_back(this_info); }
            else if (major_minor==1)                                                            { highways_medium.push_back(this_info); }
            else                                                                                { highways_minor.push_back(this_info); }
        }
        
        // determine if is railway
        if (these_tags.find("railway") != these_tags.end()){
            // determine if is subway
            if (these_tags.find("railway")->second == "subway"){
                // find all the points coordinates
                std::vector<OSMID>          all_nodes  = getWayMembers(this_way);
                std::vector<ezgl::point2d>  all_coords_xy;
                for (int i = 0; i < all_nodes.size(); ++i){
                    const OSMNode* this_node = NodeID_node.find(all_nodes[i])->second;
                    LatLon        coords_latlon = getNodeCoords(this_node);
                    ezgl::point2d coords_xy(x_from_lon(coords_latlon.lon()),
                                            y_from_lat(coords_latlon.lat()));
                    all_coords_xy.push_back(coords_xy);
                }
                // store all the data in one railway_info
                railway_info this_info;
                this_info.allPoints = all_coords_xy;
                this_info.tags      = these_tags;
                this_info.wayOSMID  = this_wayID;
                // insert the struct into the variable
                if (these_tags.find("name") != these_tags.end()){
                    std::string railway_name = these_tags.find("name")->second;
                    railways_subways.insert(std::make_pair(railway_name, this_info));
                }
            }
        }
    }
    
    // std::vector<segment_info> streetSegments;
    for(size_t i = 0; i < streetSegments.size(); ++i) {
        InfoStreetSegment this_segment_info = getInfoStreetSegment(i);
        // store oneWay
        streetSegments[i].oneWay                = this_segment_info.oneWay;
        // store speedLimit
        streetSegments[i].speedLimit            = this_segment_info.speedLimit;
        // store XY_ of "from" in allPoints
        streetSegments[i].allPoints.push_back({ intersections[this_segment_info.from].x_,
                                                intersections[this_segment_info.from].y_});
        // store XY_ of "curve points" in allPoints
        for (int curvePnt = 0; curvePnt < this_segment_info.curvePointCount; ++curvePnt){
            LatLon this_curvePnt = getStreetSegmentCurvePoint(curvePnt,i);
            streetSegments[i].allPoints.push_back({ x_from_lon(this_curvePnt.lon()),
                                                    y_from_lat(this_curvePnt.lat())});
        }
        // store XY_ of "to" in allPoints
        streetSegments[i].allPoints.push_back({ intersections[this_segment_info.to].x_,
                                                intersections[this_segment_info.to].y_});
    }
    
    // std::unordered_map<int,poi_info> POIs_entertainment/food/public_gathering/other; 
    for(int i=0; i< getNumPointsOfInterest(); i++ ) {
        poi_info new_poi;
        LatLon this_position      = getPointOfInterestPosition(i);
        new_poi.x_   = x_from_lon(this_position.lon());
        new_poi.y_   = y_from_lat(this_position.lat());
        new_poi.name = getPointOfInterestName(i);
        std::string poiType=getPointOfInterestType(i);
        new_poi.type = poiType;
        // determine the categories it belongs to
        if(      poiType == "ferry_terminal"    || poiType == "theatre"     || 
                 poiType == "community_centre"  || poiType == "nightclub"   || 
                 poiType == "pub"               || poiType == "stripclub"   || 
                 poiType == "cinema"            || poiType == "bar"         || 
                 poiType == "cafe"              || poiType == "old_cafe"    || 
                 poiType == "social_facility"   || poiType == "lotto"       || 
                 poiType == "betting" )                                             {POIs_entertainment.insert(std::make_pair(i,new_poi));}
        else if (poiType == "fast_food"         || poiType == "restaurant"  || 
                 poiType == "ice_cream"         || poiType == "food_court"  || 
                 poiType == "vending_machine")                                      {POIs_food.insert(std::make_pair(i,new_poi));}
        else if (poiType == "school"            || poiType == "community_centre" ||
                 poiType == "parking"           || poiType == "library"          || 
                 poiType == "post_office")                                          {POIs_public_gathering.insert(std::make_pair(i,new_poi));}
        else                                                                        {POIs_other.insert(std::make_pair(i,new_poi));}
    }    
    
    // std::vector<feature_info> features;
    for (int featureid=0; featureid<featureID_featurePts.size(); featureid++) {
         int totalPtsCount = getFeaturePointCount(featureid); 
         // set all points
         for(int i=0; i<totalPtsCount; i++) {
            float xcoords =  x_from_lon((float)(featureID_featurePts[featureid][i].lon()));
            float ycoords =  y_from_lat((float)(featureID_featurePts[featureid][i].lat()));
            features[featureid].allPoints.push_back({xcoords,ycoords});
         }
         // set the closed bool
         LatLon latLonPoint0    = featureID_featurePts[featureid][0];
         LatLon latLonPointLast = featureID_featurePts[featureid][totalPtsCount-1];
         if((latLonPoint0.lat()==latLonPointLast.lat()) && 
            (latLonPoint0.lon()==latLonPointLast.lon()) && 
            totalPtsCount>1)
            features[featureid].closed = true;
         // set type
         features[featureid].type = getFeatureType(featureid);
         // set name
         features[featureid].name = getFeatureName(featureid);
         // set area
         features[featureid].area = find_feature_area(featureid);
    }
}

// draws main canvas and all relevant features
// call all the draw functions
void draw_main_canvas (ezgl::renderer *g){
    out_of_bound_prevention(g); 
    draw_features(g); 
    draw_all_highways(g);
    if (memory.layer_railway_subway){
        draw_all_railways(g);
    }
    draw_intersections(g); 
    draw_points_of_interests(g); 
    draw_street_segments_directions (g);
    draw_street_names(g);
    draw_path_found(g);
    if (memory.path_finding_intersections.size() == 2){
        print_path_found();
    }
}

// this set the visible back to initial state 
// when visible world is smaller than initial world
void out_of_bound_prevention(ezgl::renderer *g) {
    ezgl::rectangle current_visible_world = g->get_visible_world();
    if (current_visible_world.width() > initial_world.width() && current_visible_world.height() > initial_world.height()){
        g -> set_visible_world(initial_world);
        memory.last_visible_world = initial_world;
    }/*
    else if (current_visible_world.right()  > initial_world.right() ||
             current_visible_world.left()   < initial_world.left()  ||
             current_visible_world.top()    > initial_world.top()   ||
             current_visible_world.bottom() < initial_world.bottom()){
        g -> set_visible_world(memory.last_visible_world);
    }
    else{
        memory.last_visible_world = current_visible_world;
    }*/
    return;
}

// draws all intersections
// drawn only when zoomed in
void draw_intersections (ezgl::renderer *g){
    int visible_width = g->get_visible_world().width();
    //if (visible_width > 2000) return;
    float radius = 1;
    for (size_t i = 0; i < intersections.size(); ++i) {
        float x = intersections[i].x_;
        float y = intersections[i].y_;
        if (intersections[i].highlight) {
            g->set_color(ezgl::RED);
        } 
        else {
            g->set_color(ezgl::GREY_55);
        }
        g->fill_arc({x, y}, radius, 0, 360);
    }
}

// draw all the streets using pre-loaded data stuctures
// different amount of details are shown according to zoom level
// minor streets are shown only when zoomed in
// major streets have thicker line than minor ones
void draw_all_highways(ezgl::renderer *g){
    //draw all the lines 
    g->set_line_cap(ezgl::line_cap::round); // round ends
    g->set_line_dash(ezgl::line_dash::none); // Solid line
    float visible_w = g->get_visible_world().width();
    float visible_h = g->get_visible_world().height();
    float initial_w = initial_world.width();
    float initial_h = initial_world.height();
    float zoom_factor = std::min(visible_w/initial_w, visible_h/initial_h);
    // zoom factor: 1, 0.6, 0.36
    if ( zoom_factor > 0.3 ){
        // major highways
        g->set_line_width(1);
        for(size_t i = 0; i < highways_major.size(); ++i){
            if (highways_major[i].highlight){
                g->set_color(ezgl::ORANGE);
            }
            else{
                g->set_color(255, 255, 255, 255);
            }
            std::vector<ezgl::point2d> these_points = highways_major[i].allPoints;
            for (int pnt = 0; pnt < (these_points.size()-1); ++pnt){
                //get coordinate of two points 
                ezgl::point2d point1 = these_points[pnt  ];
                ezgl::point2d point2 = these_points[pnt+1];
                g->draw_line(point1, point2);
            }
        }
    }
    // zoom factor: 0.22
    else if ( zoom_factor > 0.2 ){
        // medium highways    
        g->set_line_width(0.5);
        g->set_color(50, 50, 50, 255);
        for(size_t i = 0; i < highways_medium.size(); ++i){
            std::vector<ezgl::point2d> these_points = highways_medium[i].allPoints;
            for (int pnt = 0; pnt < (these_points.size()-1); ++pnt){
                //get coordinate of two points and draw
                ezgl::point2d point1 = these_points[pnt  ];
                ezgl::point2d point2 = these_points[pnt+1];
                g->draw_line(point1, point2);
            }
        }
        // major highways
        g->set_line_width(1);
        for(size_t i = 0; i < highways_major.size(); ++i){
            if (highways_major[i].highlight){
                g->set_color(ezgl::ORANGE);
            }
            else{
                g->set_color(255, 255, 255, 255);
            }
            std::vector<ezgl::point2d> these_points = highways_major[i].allPoints;
            for (int pnt = 0; pnt < (these_points.size()-1); ++pnt){
                //get coordinate of two points and draw
                ezgl::point2d point1 = these_points[pnt  ];
                ezgl::point2d point2 = these_points[pnt+1];
                g->draw_line(point1, point2);
            }
        }
    }
    // zoom factor: 0.12
    else if ( zoom_factor > 0.1 ){
        // medium highways 
        g->set_line_width(0.5);
        g->set_color(100, 100, 100, 255);
        for(size_t i = 0; i < highways_medium.size(); ++i){
            std::vector<ezgl::point2d> these_points = highways_medium[i].allPoints;
            for (int pnt = 0; pnt < (these_points.size()-1); ++pnt){
                //get coordinate of two points and draw
                ezgl::point2d point1 = these_points[pnt  ];
                ezgl::point2d point2 = these_points[pnt+1];
                g->draw_line(point1, point2);
            }
        }
        // major highways
        g->set_line_width(1);
        for(size_t i = 0; i < highways_major.size(); ++i){
            if (highways_major[i].highlight){
                g->set_color(ezgl::ORANGE);
            }
            else{
                g->set_color(255, 255, 255, 255);
            }
            std::vector<ezgl::point2d> these_points = highways_major[i].allPoints;
            for (int pnt = 0; pnt < (these_points.size()-1); ++pnt){
                //get coordinate of two points and draw
                ezgl::point2d point1 = these_points[pnt  ];
                ezgl::point2d point2 = these_points[pnt+1];
                g->draw_line(point1, point2);
            }
        }
    }
    // zoom factor: 0.07
    else if ( zoom_factor > 0.05 ){
        // medium highways 
        g->set_line_width(0.5);
        g->set_color(155, 155, 155, 255);
        for(size_t i = 0; i < highways_medium.size(); ++i){
            std::vector<ezgl::point2d> these_points = highways_medium[i].allPoints;
            for (int pnt = 0; pnt < (these_points.size()-1); ++pnt){
                //get coordinate of two points and draw
                ezgl::point2d point1 = these_points[pnt  ];
                ezgl::point2d point2 = these_points[pnt+1];
                g->draw_line(point1, point2);
            }
        }
        // major highways
        g->set_line_width(1);
        for(size_t i = 0; i < highways_major.size(); ++i){
            if (highways_major[i].highlight){
                g->set_color(ezgl::ORANGE);
            }
            else{
                g->set_color(255, 255, 255, 255);
            }
            std::vector<ezgl::point2d> these_points = highways_major[i].allPoints;
            for (int pnt = 0; pnt < (these_points.size()-1); ++pnt){
                //get coordinate of two points and draw
                ezgl::point2d point1 = these_points[pnt  ];
                ezgl::point2d point2 = these_points[pnt+1];
                g->draw_line(point1, point2);
            }
        }
    }
    // zoom factor: 0.046
    else if ( zoom_factor > 0.04 ){
        // minor highways
        g->set_line_width(1);
        g->set_color(50, 50, 50, 255);
        for(size_t i = 0; i < highways_minor.size(); ++i){
            std::vector<ezgl::point2d> these_points = highways_minor[i].allPoints;
            for (int pnt = 0; pnt < (these_points.size()-1); ++pnt){
                //get coordinate of two points and draw
                ezgl::point2d point1 = these_points[pnt  ];
                ezgl::point2d point2 = these_points[pnt+1];
                g->draw_line(point1, point2);
            }
        }
        // medium highways
        g->set_line_width(1);
        g->set_color(150, 150, 150, 255);
        for(size_t i = 0; i < highways_medium.size(); ++i){
            std::vector<ezgl::point2d> these_points = highways_medium[i].allPoints;
            for (int pnt = 0; pnt < (these_points.size()-1); ++pnt){
                //get coordinate of two points and draw
                ezgl::point2d point1 = these_points[pnt  ];
                ezgl::point2d point2 = these_points[pnt+1];
                g->draw_line(point1, point2);
            }
        }
        // major highways
        g->set_line_width(2);
        for(size_t i = 0; i < highways_major.size(); ++i){
            if (highways_major[i].highlight){
                g->set_color(ezgl::ORANGE);
            }
            else{
                g->set_color(255, 255, 255, 255);
            }
            std::vector<ezgl::point2d> these_points = highways_major[i].allPoints;
            for (int pnt = 0; pnt < (these_points.size()-1); ++pnt){
                //get coordinate of two points and draw
                ezgl::point2d point1 = these_points[pnt  ];
                ezgl::point2d point2 = these_points[pnt+1];
                g->draw_line(point1, point2);
            }
        }
    }
    // zoom factor: 0.027
    else if ( zoom_factor > 0.02 ){
        // minor highways
        g->set_line_width(2);
        g->set_color(100, 100, 100, 255);
        for(size_t i = 0; i < highways_minor.size(); ++i){
            std::vector<ezgl::point2d> these_points = highways_minor[i].allPoints;
            for (int pnt = 0; pnt < (these_points.size()-1); ++pnt){
                //get coordinate of two points and draw
                ezgl::point2d point1 = these_points[pnt  ];
                ezgl::point2d point2 = these_points[pnt+1];
                g->draw_line(point1, point2);
            }
        }
        // medium highways
        g->set_line_width(2);
        g->set_color(200, 200, 200, 255);
        for(size_t i = 0; i < highways_medium.size(); ++i){
            std::vector<ezgl::point2d> these_points = highways_medium[i].allPoints;
            for (int pnt = 0; pnt < (these_points.size()-1); ++pnt){
                //get coordinate of two points and draw
                ezgl::point2d point1 = these_points[pnt  ];
                ezgl::point2d point2 = these_points[pnt+1];
                g->draw_line(point1, point2);
            }
        }
        // major highways
        g->set_line_width(5);
        for(size_t i = 0; i < highways_major.size(); ++i){
            if (highways_major[i].highlight){
                g->set_color(ezgl::ORANGE);
            }
            else{
                g->set_color(255, 255, 255, 255);
            }
            std::vector<ezgl::point2d> these_points = highways_major[i].allPoints;
            for (int pnt = 0; pnt < (these_points.size()-1); ++pnt){
                //get coordinate of two points and draw
                ezgl::point2d point1 = these_points[pnt  ];
                ezgl::point2d point2 = these_points[pnt+1];
                g->draw_line(point1, point2);
            }
        }        
    }
    // zoom factor: 0.6^8 (0.016)
    else if ( zoom_factor > 0.016 ){
        // minor highways
        g->set_line_width(3);
        g->set_color(150, 150, 150, 255);
        for(size_t i = 0; i < highways_minor.size(); ++i){
            std::vector<ezgl::point2d> these_points = highways_minor[i].allPoints;
            for (int pnt = 0; pnt < (these_points.size()-1); ++pnt){
                //get coordinate of two points and draw
                ezgl::point2d point1 = these_points[pnt  ];
                ezgl::point2d point2 = these_points[pnt+1];
                g->draw_line(point1, point2);
            }
        }
        // medium highways
        g->set_line_width(3);
        g->set_color(200, 200, 200, 255);
        for(size_t i = 0; i < highways_medium.size(); ++i){
            std::vector<ezgl::point2d> these_points = highways_medium[i].allPoints;
            for (int pnt = 0; pnt < (these_points.size()-1); ++pnt){
                //get coordinate of two points and draw
                ezgl::point2d point1 = these_points[pnt  ];
                ezgl::point2d point2 = these_points[pnt+1];
                g->draw_line(point1, point2);
            }
        }
        // major highways
        g->set_line_width(10);
        for(size_t i = 0; i < highways_major.size(); ++i){
            if (highways_major[i].highlight){
                g->set_color(ezgl::ORANGE);
            }
            else{
                g->set_color(255, 255, 255, 255);
            }
            std::vector<ezgl::point2d> these_points = highways_major[i].allPoints;
            for (int pnt = 0; pnt < (these_points.size()-1); ++pnt){
                //get coordinate of two points and draw
                ezgl::point2d point1 = these_points[pnt  ];
                ezgl::point2d point2 = these_points[pnt+1];
                g->draw_line(point1, point2);
            }
        }
    }
    else {
        // minor highways
        g->set_line_width(10);
        g->set_color(150, 150, 150, 255);
        for(size_t i = 0; i < highways_minor.size(); ++i){
            std::vector<ezgl::point2d> these_points = highways_minor[i].allPoints;
            for (int pnt = 0; pnt < (these_points.size()-1); ++pnt){
                //get coordinate of two points and draw
                ezgl::point2d point1 = these_points[pnt  ];
                ezgl::point2d point2 = these_points[pnt+1];
                g->draw_line(point1, point2);
            }
        }
        // medium highways
        g->set_line_width(10);
        g->set_color(200, 200, 200, 255);
        for(size_t i = 0; i < highways_medium.size(); ++i){
            std::vector<ezgl::point2d> these_points = highways_medium[i].allPoints;
            for (int pnt = 0; pnt < (these_points.size()-1); ++pnt){
                //get coordinate of two points and draw
                ezgl::point2d point1 = these_points[pnt  ];
                ezgl::point2d point2 = these_points[pnt+1];
                g->draw_line(point1, point2);
            }
        }
        // zoom factor: major highways
        g->set_line_width(10);
        for(size_t i = 0; i < highways_major.size(); ++i){
            if (highways_major[i].highlight){
                g->set_color(ezgl::ORANGE);
            }
            else{
                g->set_color(255, 255, 255, 255);
            }
            std::vector<ezgl::point2d> these_points = highways_major[i].allPoints;
            for (int pnt = 0; pnt < (these_points.size()-1); ++pnt){
                //get coordinate of two points and draw
                ezgl::point2d point1 = these_points[pnt  ];
                ezgl::point2d point2 = these_points[pnt+1];
                g->draw_line(point1, point2);
            }
        }
    }
}

// Lthis function draws all the features.
// It draws different amount of features according to zoom factor
void draw_features(ezgl::renderer *g) {
    g->set_line_cap(ezgl::line_cap::round); // round ends
    g->set_line_dash(ezgl::line_dash::none); 
    // find the zoom factor
    float visible_w = g->get_visible_world().width();
    float visible_h = g->get_visible_world().height();
    float initial_w = initial_world.width();
    float initial_h = initial_world.height();
    float zoom_factor = std::min(visible_w/initial_w, visible_h/initial_h);
    float visible_area = g->get_visible_world().area();
    // iterate and draw
    for (int feature_id=0; feature_id<features.size(); ++feature_id) {
        feature_info this_feature = features[feature_id];
        // zoom factor: 1, 0.6
        if (zoom_factor > 0.5){
            if (this_feature.closed && this_feature.area > visible_area/50000){
                // light blue
                if (this_feature.type==Lake) {
                    g->set_color(100, 150, 200, 255);
                    g->fill_poly(this_feature.allPoints);
                }
                // dark green
                if (this_feature.type==Park) {
                    g->set_color(100, 130, 100, 255);
                    g->fill_poly(this_feature.allPoints);
                }
                // black
                if (this_feature.type==Island) {
                    g->set_color(ezgl::BLACK);
                    g->fill_poly(this_feature.allPoints);
                }
                // when water body button is on
                // dark blue
                if (this_feature.type==River && memory.layer_water_body){
                        g->set_color(100, 150, 200, 255);
                        g->fill_poly(this_feature.allPoints);
                }
            }
            // when water body button is on
            else if ( !this_feature.closed && memory.layer_water_body){
                for(int pts=0; pts<this_feature.allPoints.size()-1; pts++) {
                    ezgl::point2d position1 = this_feature.allPoints[pts  ];
                    ezgl::point2d position2 = this_feature.allPoints[pts+1];
                    g->set_line_width(1);
                    g->set_color(100, 150, 200, 255);
                    g->draw_line(position1, position2);
                }
            }
        }
        // zoom factor: 0.6^2
        else if (zoom_factor > 0.3) {
            if (this_feature.closed && this_feature.area > visible_area/100000) {
                // dark blue
                if (this_feature.type==Lake) {
                    g->set_color(100, 150, 200, 255);
                    g->fill_poly(this_feature.allPoints);
                }
                // dark green
                if (this_feature.type==Park) {
                    g->set_color(100, 130, 100, 255);
                    g->fill_poly(this_feature.allPoints);
                }
                // black
                if (this_feature.type==Island) {
                    g->set_color(ezgl::BLACK);
                    g->fill_poly(this_feature.allPoints);
                }
                // dark bluish green
                if (this_feature.type==Greenspace) {
                    g->set_color(75, 130, 100, 255);
                    g->fill_poly(this_feature.allPoints);
                }
            }
            // when water body button is on
            else if ( !this_feature.closed && memory.layer_water_body){
                for(int pts=0; pts<this_feature.allPoints.size()-1; pts++) {
                    ezgl::point2d position1 = this_feature.allPoints[pts  ];
                    ezgl::point2d position2 = this_feature.allPoints[pts+1];
                    g->set_line_width(2);
                    g->set_color(100, 150, 200, 255);
                    g->draw_line(position1, position2);
                }
            }
        }
        // zoom factor: 0.6^3, 0.6^4
        else if (zoom_factor > 0.1){
            if (this_feature.closed){
                // dark blue
                if (this_feature.type==Lake) {
                    g->set_color(100, 150, 200, 255);
                    g->fill_poly(this_feature.allPoints);
                }
                // reddish green
                if (this_feature.type==Golfcourse) {
                    g->set_color(120, 130, 100, 255);
                    g->fill_poly(this_feature.allPoints);
                }
                // dark green
                if (this_feature.type==Park) {
                    g->set_color(100, 130, 100, 255);
                    g->fill_poly(this_feature.allPoints);
                }
                // black
                if (this_feature.type==Island) {
                    g->set_color(ezgl::BLACK);
                    g->fill_poly(this_feature.allPoints);
                }
                // bluish green
                if (this_feature.type==Greenspace) {
                    g->set_color(75, 130, 100, 255);
                    g->fill_poly(this_feature.allPoints);
                }
                // dark blue
                if (this_feature.type==River) {
                    g->set_color(100, 150, 200, 255);
                    g->fill_poly(this_feature.allPoints);
                }
            }
            // when water body button is on
            else if ( !this_feature.closed && memory.layer_water_body){
                for(int pts=0; pts<this_feature.allPoints.size()-1; pts++) {
                    ezgl::point2d position1 = this_feature.allPoints[pts  ];
                    ezgl::point2d position2 = this_feature.allPoints[pts+1];
                    g->set_line_width(2);
                    g->set_color(100, 150, 200, 255);
                    g->draw_line(position1, position2);
                }
            }
        }
        // zoom factor: 0.6^5
        else if (zoom_factor > 0.05){
             if (this_feature.closed){
                // dark blue
                if (this_feature.type==Lake) {
                    g->set_color(100, 150, 200, 255);
                    g->fill_poly(this_feature.allPoints);
                }
                // reddish green
                if (this_feature.type==Golfcourse) {
                    g->set_color(120, 130, 100, 255);
                    g->fill_poly(this_feature.allPoints);
                }
                // dark green
                if (this_feature.type==Park) {
                    g->set_color(100, 130, 100, 255);
                    g->fill_poly(this_feature.allPoints);
                }
                // black
                if (this_feature.type==Island) {
                    g->set_color(ezgl::BLACK);
                    g->fill_poly(this_feature.allPoints);
                }
                // dark bluish green
                if (this_feature.type==Greenspace) {
                    g->set_color(75, 130, 100, 255);
                    g->fill_poly(this_feature.allPoints);
                }
                // dark blue
                if (this_feature.type==River) {
                    g->set_color(100, 150, 200, 255);
                    g->fill_poly(this_feature.allPoints);
                }
                // dark gray
                if (this_feature.type==Building) {
                    g->set_color(50, 50, 50, 255);
                    g->fill_poly(this_feature.allPoints);
                }
            }
            // non-closed
            else{
                for(int pts=0; pts<this_feature.allPoints.size()-1; pts++) {
                    ezgl::point2d position1 = this_feature.allPoints[pts  ];
                    ezgl::point2d position2 = this_feature.allPoints[pts+1];
                    g->set_line_width(2);
                    g->set_color(100, 150, 200, 255);
                    g->draw_line(position1, position2);
                }
            }
        }
        // zoom factor: 0.6^6, 0.6^7
        else if (zoom_factor > 0.02){
             if (this_feature.closed){
                // dark blue
                if (this_feature.type==Lake) {
                    g->set_color(100, 150, 200, 255);
                    g->fill_poly(this_feature.allPoints);
                }
                // reddish green
                if (this_feature.type==Golfcourse) {
                    g->set_color(120, 130, 100, 255);
                    g->fill_poly(this_feature.allPoints);
                }
                // dark green
                if (this_feature.type==Park) {
                    g->set_color(100, 130, 100, 255);
                    g->fill_poly(this_feature.allPoints);
                }
                // black
                if (this_feature.type==Island) {
                    g->set_color(ezgl::BLACK);
                    g->fill_poly(this_feature.allPoints);
                }
                // dark bluish green
                if (this_feature.type==Greenspace) {
                    g->set_color(75, 130, 100, 255);
                    g->fill_poly(this_feature.allPoints);
                }
                // dark blue
                if (this_feature.type==River) {
                    g->set_color(100, 150, 200, 255);
                    g->fill_poly(this_feature.allPoints);
                }
                // lighter gray
                if (this_feature.type==Building) {
                    g->set_color(75, 75, 75, 255);
                    g->fill_poly(this_feature.allPoints);
                }
                // brown
                if (this_feature.type==Beach) {
                    g->set_color(125, 100, 75, 255);
                    g->fill_poly(this_feature.allPoints);
                }
            }
            // non-closed
            else{
                for(int pts=0; pts<this_feature.allPoints.size()-1; pts++) {
                    ezgl::point2d position1 = this_feature.allPoints[pts  ];
                    ezgl::point2d position2 = this_feature.allPoints[pts+1];
                    g->set_line_width(5);
                    g->set_color(100, 150, 200, 255);
                    g->draw_line(position1, position2);
                }
            }
        }
        // zoom factor: 0.6^8, 0
        else{
            // closed
            if (this_feature.closed){
                // dark blue
                if (this_feature.type==Lake) {
                    g->set_color(100, 150, 200, 255);
                    g->fill_poly(this_feature.allPoints);
                }
                // reddish green
                if (this_feature.type==Golfcourse) {
                    g->set_color(120, 130, 100, 255);
                    g->fill_poly(this_feature.allPoints);
                }
                // dark green
                if (this_feature.type==Park) {
                    g->set_color(100, 130, 100, 255);
                    g->fill_poly(this_feature.allPoints);
                }
                // black
                if (this_feature.type==Island) {
                    g->set_color(ezgl::BLACK);
                    g->fill_poly(this_feature.allPoints);
                }
                // dark bluish green
                if (this_feature.type==Greenspace) {
                    g->set_color(75, 130, 100, 255);
                    g->fill_poly(this_feature.allPoints);
                }
                // dark blue
                if (this_feature.type==River) {
                    g->set_color(100, 150, 200, 255);
                    g->fill_poly(this_feature.allPoints);
                }
                // lighter gray
                if (this_feature.type==Building) {
                    g->set_color(100, 100, 100, 255);
                    g->fill_poly(this_feature.allPoints);
                }
                // brown
                if (this_feature.type==Beach) {
                    g->set_color(125, 100, 75, 255);
                    g->fill_poly(this_feature.allPoints);
                } 
            }
            // non-closed 
            else{
                for(int pts=0; pts<this_feature.allPoints.size()-1; pts++) {
                    ezgl::point2d position1 = this_feature.allPoints[pts  ];
                    ezgl::point2d position2 = this_feature.allPoints[pts+1];
                    g->set_line_width(10);
                    g->set_color(100, 150, 200, 255);
                    g->draw_line(position1, position2);
                }
            }
        }
    }
}

// draw all points of interests
void draw_points_of_interests(ezgl::renderer *g) {
    float visible_w = g->get_visible_world().width();
    float visible_h = g->get_visible_world().height();
    float initial_w = initial_world.width();
    float initial_h = initial_world.height();
    float zoom_factor = std::min(visible_w/initial_w, visible_h/initial_h);
    // draw them only when zoomed up
    if (zoom_factor < 0.04){
        // draw entertainment POI
        for ( auto it = POIs_entertainment.begin(); it != POIs_entertainment.end(); ++it ){
            float x = it->second.x_;
            float y = it->second.y_;
            // draw icon if clicked or button toggled
            if (it->second.highlight || memory.layer_poi) {
                ezgl::surface *png_surface = ezgl::renderer::load_png("libstreetmap/resources/Fun.png");
                g->draw_surface(png_surface, {x-2500*zoom_factor, y + 2500*zoom_factor});
                ezgl::renderer::free_surface(png_surface);
            }
            // draw circle if else
            else {
                g->set_color(ezgl::YELLOW);
                g->fill_arc({x, y}, 2, 0, 360);
            }
        }
        for ( auto it = POIs_food.begin(); it != POIs_food.end(); ++it ){
            float x = it->second.x_;
            float y = it->second.y_;
            // draw icon if clicked or button toggled
            if (it->second.highlight || memory.layer_poi) {
                ezgl::surface *png_surface = ezgl::renderer::load_png("libstreetmap/resources/Food.png");
                g->draw_surface(png_surface, {x-2500*zoom_factor, y + 2500*zoom_factor});
                ezgl::renderer::free_surface(png_surface);
            }
            // draw circle if else
            else {
                g->set_color(255, 125, 0, 255);
                g->fill_arc({x, y}, 2, 0, 360);
            }
        }
        for ( auto it = POIs_public_gathering.begin(); it != POIs_public_gathering.end(); ++it ){
            float x = it->second.x_;
            float y = it->second.y_;
            if (it->second.highlight || memory.layer_poi) {
                ezgl::surface *png_surface = ezgl::renderer::load_png("libstreetmap/resources/Public.png");
                g->draw_surface(png_surface, {x-2500*zoom_factor, y + 2500*zoom_factor});
                ezgl::renderer::free_surface(png_surface);
            }
            else {
                g->set_color(68,150,255,255);
                g->fill_arc({x, y}, 2, 0, 360);
            }
        }
        for ( auto it = POIs_other.begin(); it != POIs_other.end(); ++it ){
            float x = it->second.x_;
            float y = it->second.y_;
            if (it->second.highlight || memory.layer_poi) {
                ezgl::surface *png_surface = ezgl::renderer::load_png("libstreetmap/resources/Other.png");
                g->draw_surface(png_surface, {x-2500*zoom_factor, y + 2500*zoom_factor});
                ezgl::renderer::free_surface(png_surface);
            }
            else {
                g->set_color(ezgl::GREY_75);
                g->fill_arc({x, y}, 2, 0, 360);
                g->set_color(ezgl::BLACK);
                g->set_line_width(1);
                g->draw_arc({x, y}, 2, 0, 360);
            }
        }
    }
} 

// this function draws all the street names using pre-loaded data structures
// each street name is drawn once
void draw_street_names(ezgl::renderer *g) {
    g->set_font_size(10);
    g->set_color(0, 0, 0, 255);
    float visible_width = g->get_visible_world().width();
    float initial_width = initial_world.width();
    // display only when zoomed up
    if (visible_width < 0.02 * initial_width){
        // iterate through all sreets
        for (int street=0; street < getNumStreets(); ++street){
            // find street name and its length
            std::string street_name = getStreetName(street);
            int name_size = street_name.size();
            // find the position and angle to draw the name
            std::vector<int> these_segments = street_street_segments[street];
            int this_segment_idx = these_segments[these_segments.size()/2];
            std::vector<ezgl::point2d> these_points = streetSegments[this_segment_idx].allPoints;
            ezgl::point2d point1 = these_points[these_points.size()/2 -1];
            ezgl::point2d point2 = these_points[these_points.size()/2   ];
            ezgl::point2d middle((point1.x + point2.x)*0.5, (point1.y + point2.y)*0.5);
            double angle = atan((point2.y-point1.y)/(point2.x-point1.x)) / DEGREE_TO_RADIAN;
            // draw text
            g->set_text_rotation(angle);
            // double segment_length = streetSeg_length[this_segment_idx];
            //g->draw_text(middle,street_name,std::min((double)(10*name_size),segment_length),10);
            g->draw_text(middle,street_name,(double)(10*name_size),10);
        }
    }
}

// this function draws all the subway lines using the pre-loaded data structure. 
// Different lines are in different colors
void draw_all_railways(ezgl::renderer *g){
    //std::ofstream myfile;
    //myfile.open ("subways_output.txt"); 
    // set line cap dash
    g->set_line_cap(ezgl::line_cap::round);
    g->set_line_dash(ezgl::line_dash::none);
    // set line width
    float visible_w = g->get_visible_world().width();
    float visible_h = g->get_visible_world().height();
    float initial_w = initial_world.width();
    float initial_h = initial_world.height();
    float zoom_factor = std::min(visible_w/initial_w, visible_h/initial_h);
    g->set_line_width(2/zoom_factor);
    // start drawing
    int line_tracker = 0;
    auto it = railways_subways.begin();
    // iterating through all names(lines) of subways
    while (it != railways_subways.end()){
        std::string name_key = it->first;
        //myfile << line_tracker <<"\n"<<name_key<<"\n\n";
        // select the color according to line_tracker
        switch (line_tracker) {
            case 0:  g->set_color(ezgl::LIME_GREEN);        break;
            case 1:  g->set_color(ezgl::FIRE_BRICK);        break;
            case 2:  g->set_color(ezgl::LIGHT_SKY_BLUE);    break;
            case 3:  g->set_color(ezgl::MEDIUM_PURPLE);     break;
            case 4:  g->set_color(ezgl::CORAL);             break;
            case 5:  g->set_color(ezgl::KHAKI);             break;
            case 6:  g->set_color(ezgl::PLUM);              break;
            case 7:  g->set_color(ezgl::THISTLE);           break;
            case 8:  g->set_color(ezgl::LIGHT_PINK);        break;
            case 9:  g->set_color(ezgl::YELLOW);            break;
            case 10: g->set_color(ezgl::ORANGE);            break;
            case 11: g->set_color(ezgl::GREEN);             break;
            case 12: g->set_color(ezgl::PINK);              break;
            default: g->set_color(ezgl::RED);               break;
        }
        // iterating through all ways of a name(line)
        while (it->first == name_key){
            std::vector<ezgl::point2d> these_points = it->second.allPoints;
            // iterating through all nodes of one way
            for (int pnt = 0; pnt < (these_points.size()-1); ++pnt){
                //get coordinate of two points 
                ezgl::point2d point1 = these_points[pnt  ];
                ezgl::point2d point2 = these_points[pnt+1];
                g->draw_line(point1, point2);
            }
            it++;
        }
        line_tracker++;
    }
    //myfile.close();
}

// draw street sgements directions
void draw_street_segments_directions (ezgl::renderer *g){
    // find zoom factor
    float visible_w = g->get_visible_world().width();
    float visible_h = g->get_visible_world().height();
    float initial_w = initial_world.width();
    float initial_h = initial_world.height();
    float zoom_factor = std::min(visible_w/initial_w, visible_h/initial_h);
    // set up renderer
    g->set_color(ezgl::RED);
    g->set_font_size(10);
    // display only when zoomed in
    for (int i = 0; zoom_factor < 0.04 && i < memory.streets_with_directions.size(); ++i){
        std::vector<int> Segs = street_street_segments[memory.streets_with_directions[i]];
        for (int j = 0 ; j < Segs.size(); ++j){
            std::vector<ezgl::point2d> these_points = streetSegments[Segs[j]].allPoints;
            //get coordinate of two points 
            ezgl::point2d point1 = these_points[0];
            ezgl::point2d point2 = these_points[1];
            ezgl::point2d middle((point1.x + point2.x)*0.5, (point1.y + point2.y)*0.5);
            double angle = 90 + atan2((point2.y-point1.y),(point2.x-point1.x)) / DEGREE_TO_RADIAN;
            // draw text
            g->set_text_rotation(angle);
            g->draw_text(middle,"V");
        }
    }
}

// draw the path found in the path finding utilities
void draw_path_found(ezgl::renderer *g){
    // Draw all walk path_found
    std::vector<int>* walk_path_vector = &(memory.walk_path_found);
    for (int i = 0; i < walk_path_vector->size(); ++i){
        int seg_idx    = (*walk_path_vector)[i];
        int street_idx = getInfoStreetSegment(seg_idx).streetID;
        std::string street_name = getStreetName(street_idx);
        // draw out this segment
        g->set_color(ezgl::BLUE);
        g->set_line_width(5);
        std::vector<ezgl::point2d> these_points = streetSegments[seg_idx].allPoints;
        for (int pnt = 0; pnt < (these_points.size()-1); ++pnt){
            //get coordinate of two points 
            ezgl::point2d point1 = these_points[pnt  ];
            ezgl::point2d point2 = these_points[pnt+1];
            g->draw_line(point1, point2);
        }
    }
    // Draw all drive path_found
    std::vector<int>* path_vector = &(memory.path_found);
    for (int i = 0; i < path_vector->size(); ++i){
        int seg_idx    = (*path_vector)[i];
        int street_idx = getInfoStreetSegment(seg_idx).streetID;
        std::string street_name = getStreetName(street_idx);
        // draw out this segment
        g->set_color(ezgl::RED);
        g->set_line_width(5);
        std::vector<ezgl::point2d> these_points = streetSegments[seg_idx].allPoints;
        for (int pnt = 0; pnt < (these_points.size()-1); ++pnt){
            //get coordinate of two points 
            ezgl::point2d point1 = these_points[pnt  ];
            ezgl::point2d point2 = these_points[pnt+1];
            g->draw_line(point1, point2);
        }
    }
}

// print the path found in the path finding utilities
void print_path_found(){
    std::cout<<"\n**********************";
    
    // print starting point
    std::cout<<"\n[Starting Point:]\n";
    std::string start_name = getIntersectionName(memory.path_finding_intersections[0]);
    std::cout << start_name << "\n";
    // print all walk path_found
    std::vector<int>* walk_path_vector = &(memory.walk_path_found);
    int prev_street_idx = -1;
    if (walk_path_vector->size() != 0 ){
        float walk_time_minute = compute_path_walking_time(*walk_path_vector, 1.4, 15)/60;
        std::cout<<"\n[Walking path: Total "<<walk_time_minute<<" minutes ]\n";
    }
    for (int i = 0; i < walk_path_vector->size(); ++i){
        int seg_idx    = (*walk_path_vector)[i];
        int street_idx = getInfoStreetSegment(seg_idx).streetID;
        std::string street_name = getStreetName(street_idx);
        // print out the instruction
        if (street_idx != prev_street_idx && street_name !="<unknown>"){
            std::cout << "Turn to " << street_name << "\n";
            std::cout << "Drive down " << street_name << "\n";
        }
        prev_street_idx = street_idx;
    }
    // print all drive path_found
    std::vector<int>* path_vector = &(memory.path_found);
    prev_street_idx = -1;
    if (path_vector->size() != 0 ){
        float drive_time_minute = compute_path_travel_time(*path_vector, 15)/60;
        std::cout<<"\n[Driving path: Total "<<drive_time_minute<<" minutes ]\n";
    }
    for (int i = 0; i < path_vector->size(); ++i){
        int seg_idx    = (*path_vector)[i];
        int street_idx = getInfoStreetSegment(seg_idx).streetID;
        std::string street_name = getStreetName(street_idx);
        // print out the instruction
        if (street_idx != prev_street_idx && street_name !="<unknown>"){
            std::cout << "Turn to " << street_name << "\n";
            std::cout << "Drive down " << street_name << "\n";
        }
        prev_street_idx = street_idx;
    }
    // print starting point
    std::cout<<"\n[Destination:]\n";
    std::string end_name = getIntersectionName(memory.path_finding_intersections[1]);
    std::cout << end_name << "\n";

    std::cout<<"**********************\n";
}

void draw_street_segments(int i, ezgl::renderer *g){
    g->set_line_width(5);
    std::vector<ezgl::point2d> these_points = streetSegments[i].allPoints;
    for (int pnt = 0; pnt < (these_points.size()-1); ++pnt){
        //get coordinate of two points 
        ezgl::point2d point1 = these_points[pnt  ];
        ezgl::point2d point2 = these_points[pnt+1];
        g->draw_line(point1, point2);
    }
}

/*************************Callback Functions*************************/

// set up the window of the application
// link all the possible callback functions with corresponding signals
void initial_setup(ezgl::application *application, bool new_window){
    // Update the status bar message
    application->update_message("EZGL Application"); 
    
    // Update the title bar
    GtkWindow* main_window = (GtkWindow *)application->get_object("MainWindow");
    gtk_window_set_title(main_window, "ece297 cd021 Amazing Map");
    
    // link the find button with callback function
    // type casting the object you get with the corresponding object
    GtkButton* find_button = (GtkButton *) application->get_object("FindButton");
    g_signal_connect(find_button, "clicked", G_CALLBACK(FindButton_callback), application);
    
    // Initialize the intersections search window.
    // Link the deletion of popup window with callback function
    GtkWindow* pop_window = (GtkWindow *) application->get_object("IntersectionsSearch");
    gtk_window_set_title(pop_window, "Streets Search");
    // the second parameter is the signal that triggers the callback function, the fourth parameter is used as user data (the one i want to pass into the callback function)
    g_signal_connect(pop_window, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
    
    // Link pop entry key press "Return" with callback function
    GtkEntry* pop_entry = (GtkEntry *) application->get_object("Popup_Entry");
    g_signal_connect(pop_entry, "key-release-event", G_CALLBACK(StreetsEntryReturn_callback), application);
    
    // // editing-done doesn't work on GtkEntry! It's a lie!
    // g_signal_connect((GtkCellEditable*)pop_entry, "editing-done", G_CALLBACK(Test_callback), application);
    
    // Link pop entry text change with callback function
    g_signal_connect(pop_entry, "changed", G_CALLBACK(StreetsEntryChange_callback), application);
    
    // force the completion to show
    GtkEntryCompletion* completion = gtk_entry_get_completion(pop_entry);
    gtk_entry_completion_set_match_func (completion, forced_auto_completion, NULL, NULL);
    gtk_entry_completion_set_text_column(completion, 1);
    
    // Initialize text in sidebar
    GtkLabel* SideBar_Info = (GtkLabel *) application->get_object("SideBar_Info");
    gtk_label_set_text (SideBar_Info, "Use the Search button to find information about one street or intersections between two streets");
    
    // Link railway layer button with callback function
    GtkToggleButton* railways_checkbutton = (GtkToggleButton *) application->get_object("Layer_Railways");
    g_signal_connect(railways_checkbutton, "toggled", G_CALLBACK(Railways_CheckButton_callback), application);
    
    // Link railway layer button with callback function
    GtkToggleButton* motorways_checkbutton = (GtkToggleButton *) application->get_object("Layer_Motorways");
    g_signal_connect(motorways_checkbutton, "toggled", G_CALLBACK(Motorways_CheckButton_callback), application);
    
    // Link water body layer button with callback function
    GtkToggleButton* waterbody_checkbutton = (GtkToggleButton *) application->get_object("Layer_Waterbodies");
    g_signal_connect(waterbody_checkbutton, "toggled", G_CALLBACK(WaterBody_CheckButton_callback), application);
    
    // Link water body layer button with callback function
    GtkToggleButton* poi_checkbutton = (GtkToggleButton *) application->get_object("Layer_POIs");
    g_signal_connect(poi_checkbutton, "toggled", G_CALLBACK(POI_CheckButton_callback), application);
    
    // Link path finding button with callback function
    GtkButton* path_button = (GtkButton *) application->get_object("Path_find_button");
    g_signal_connect(path_button, "clicked", G_CALLBACK(PathFind_Button_callback), application);
}

// action when mouse is clicked on canvas
void act_on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y) {
    //std::cout << "Mouse clicked at (" << x << "," << y << ")\n";
    LatLon pos = LatLon(lat_from_y(y), lon_from_x(x));
    
    // find closedPOIid, -1 means too far away to any poi
    int id_POI=find_closest_POI(pos);
    // un-highlight the last poi
    if(memory.last_clicked_POI !=-1) {
        // go through all four to locate the poi
        auto iter_entertainment = POIs_entertainment.find(memory.last_clicked_POI);
        if (iter_entertainment != POIs_entertainment.end()){
            iter_entertainment -> second.highlight=false;
        }
        auto iter_food = POIs_food.find(memory.last_clicked_POI);
        if (iter_food != POIs_food.end()){
            iter_food -> second.highlight=false;
        }
        auto iter_public_gathering = POIs_public_gathering.find(memory.last_clicked_POI);
        if (iter_public_gathering != POIs_public_gathering.end()){
            iter_public_gathering -> second.highlight=false;
        }
        auto iter_other = POIs_other.find(memory.last_clicked_POI);
        if (iter_other != POIs_other.end()){
            iter_other -> second.highlight=false;
        }
    }
    // set the value of last_clicked global variable
    memory.last_clicked_POI = id_POI;
    // highlight current clicked 
    if (id_POI != -1) {
        // declare an object to store the poi if find one
        std::string poi_name;
        // go through all four categories to locate the poi
        auto iter_entertainment = POIs_entertainment.find(id_POI);
        if (iter_entertainment != POIs_entertainment.end()){
            iter_entertainment -> second.highlight=true;
            poi_name = iter_entertainment->second.name;
        }
        auto iter_food = POIs_food.find(id_POI);
        if (iter_food != POIs_food.end()){
            iter_food -> second.highlight=true;
            poi_name = iter_food->second.name;
        }
        auto iter_public_gathering = POIs_public_gathering.find(id_POI);
        if (iter_public_gathering != POIs_public_gathering.end()){
            iter_public_gathering -> second.highlight=true;
            poi_name = iter_public_gathering->second.name;
        }
        auto iter_other = POIs_other.find(id_POI);
        if (iter_other != POIs_other.end()){
            iter_other -> second.highlight=true;
            poi_name = iter_other->second.name;
        }
        std::string status_message = "Closest POI: " + poi_name;
        app->update_message(status_message);
    }
    
    
    
    //find closest intersection_id, -1 means too far away to any intersections
    int id_inter = find_closest_intersection(pos);
    //un-highlight the last highlighted intersection
    std::vector<int>* last_intersections = &(memory.last_highlighted_intersections);
    for (int i = 0; i < last_intersections->size(); ++i){
        intersections[(*last_intersections)[i]].highlight = false;
    }
    last_intersections->clear();
    // set new value of last highlighted if not -1
    if (id_inter !=-1){
        last_intersections->push_back(id_inter);
    }
    // highlight current clicked
    if (id_inter != -1) {
        intersections[id_inter].highlight = true;
        std::string status_message = "Closest Intersection: " + intersections[id_inter].name;
        app->update_message(status_message);
    }
    
    
    // path finding
    std::vector<int>* starting_end_points = &(memory.path_finding_intersections);
    // ON: driving
    if (memory.path_finding_state == 1 ){
        // get starting point
        if      (starting_end_points -> size() == 0){
            if (id_inter != -1){
                starting_end_points -> push_back(id_inter);
            }
        }
        // get end point
        else if (starting_end_points -> size() == 1){
            if (id_inter != -1){
                starting_end_points -> push_back(id_inter);
                memory.path_found = find_path_between_intersections_visualized
                                                                    ((*starting_end_points)[0], 
                                                                    (*starting_end_points)[1],
                                                                    15,
                                                                    app);
            }
        }
        // get the starting point again, since vector is full
        else if (starting_end_points -> size() == 2){
            if (id_inter != -1){
                starting_end_points -> clear();
                starting_end_points -> push_back(id_inter);
                memory.path_found.clear();
            }
        }
    }
    else if ( memory.path_finding_state == 2){
        // get starting point
        if      (starting_end_points -> size() == 0){
            if (id_inter != -1){
                starting_end_points -> push_back(id_inter);
            }
        }
        // get end point
        else if (starting_end_points -> size() == 1){
            if (id_inter != -1){
                starting_end_points -> push_back(id_inter);
                std::pair<std::vector<int>,std::vector<int>> paths_pair
                                    = find_path_with_walk_to_pick_up_visualized((*starting_end_points)[0], 
                                                                    (*starting_end_points)[1],
                                                                    15, 1.4, 300, app);
                memory.walk_path_found = paths_pair.first;
                memory.path_found = paths_pair.second;
            }
        }
        // get the starting point again, since vector is full
        else if (starting_end_points -> size() == 2){
            if (id_inter != -1){
                starting_end_points -> clear();
                starting_end_points -> push_back(id_inter);
                memory.path_found.clear();
                memory.walk_path_found.clear();
            }
        }
    }
    
    app->refresh_drawing();
}

// use key_release_event for signals instead of key_press
void act_on_key_press(ezgl::application *application, GdkEventKey */*event*/, char *key_name)
{
    std::string key_pressed(key_name);
    /*
    application->update_message("Key Pressed");

    std::cout << key_name << " key is pressed" << std::endl;
    */
    
    return;
}

// initialize the pop up window and present it
void FindButton_callback(GtkButton* /*widget*/, ezgl::application *application){
    // initialize entry (empty)
    GtkEntry* entry = (GtkEntry*)application->get_object("Popup_Entry");
    gtk_entry_set_text (entry, "");
    gtk_widget_show((GtkWidget*)entry);
    // initialize label ("first street name")
    GtkLabel* label = (GtkLabel*)application->get_object("Popup_Label");
    gtk_label_set_text (label, "Please enter the first street name");
    // initialize the StatusBar
    GtkWidget* status = (GtkWidget*)application->get_object("Popup_StatusBar");
    gtk_widget_show(status);
    // present the window
    GtkWindow* popup = (GtkWindow*)application->get_object("IntersectionsSearch");
    gtk_window_present (popup);
    // initialize global var.
    memory.streets_with_directions.clear();
    memory.last_autocompletion_list.clear();
    memory.last_entry.clear();
}

// callback function triggered by "Return" button released on pop up entry callback
void StreetsEntryReturn_callback(GtkEntry* widget, GdkEventKey* event, ezgl::application *application){
    // get key pressed
    std::string key_released(gdk_keyval_name(event->keyval));
    if ( key_released == "Return"){
        // get the side bar label
        GtkLabel* sidebar = (GtkLabel *) application->get_object("SideBar_Info");
        // get the StatusBar
        GtkStatusbar* status = (GtkStatusbar*)application->get_object("Popup_StatusBar");
        // get the label
        GtkLabel* label = (GtkLabel*)application->get_object("Popup_Label");
        // get the entry text
        std::string entryText(gtk_entry_get_text(widget));
        // get the global variable list store
        std::unordered_map<std::string, int>* last_list_mem = &(memory.last_autocompletion_list);
        auto foundMatch = last_list_mem->find(entryText);
        // if entry is valid proceed
        if (foundMatch != last_list_mem->end()){
            // street id found
            int street_id = foundMatch->second;
            // re-initialize entry and status bar
            gtk_entry_set_text (widget,"");
            gtk_statusbar_push (status,0,"");
            // insert the entry into memory
            std::vector<int>* entry_mem = &(memory.last_entry);
            entry_mem->push_back(street_id);
            // if entry_mem has a size of 0, asking for first street name
            if      (entry_mem->size() == 0){
                gtk_label_set_text (label, "Please enter the first street name");
            }
            // if entry_mem has a size of 1, asking for second street name
            else if (entry_mem->size() == 1){
                gtk_label_set_text (label, "Please enter the second street name");
                // output entered street info to side bar
                std::string street_info_text;
                street_info_text =  "Street name:\n" + getStreetName(street_id) + "\n\n";
                street_info_text += "Street length:\n" + std::to_string(streetID_streetLength[street_id]) + "\n\n";
                if (streetSegments[street_street_segments[street_id][0]].oneWay){
                    street_info_text += "One-way travel only\n";
                }
                else{
                    street_info_text += "Two-way travel allowed\n";
                }
                const char* street_info_chararray = street_info_text.c_str();
                gtk_label_set_text (sidebar, street_info_chararray);
                // find the focus point
                std::vector<int> intersection_ids = street_intersections[street_id];
                int this_intersection_id = intersection_ids[intersection_ids.size()/2];
                ezgl::point2d focus_point(intersections[this_intersection_id].x_,
                                          intersections[this_intersection_id].y_);
                // change the visible to the street
                ezgl::rectangle visible_world = application->get_renderer()->get_visible_world();
                float visible_width  = visible_world.width();
                float visible_height = visible_world.height();
                float desired_width  = 100;
                float desired_height = 100/visible_width*visible_height;
                ezgl::point2d starting_point(focus_point.x - desired_width/2,
                                             focus_point.y - desired_height/2);
                ezgl::rectangle new_world_view(starting_point, desired_width, desired_height);
                application->get_renderer()->set_visible_world(new_world_view);
                // turn directions drawing on
                memory.streets_with_directions.push_back(street_id);
                application->refresh_drawing();
            }
            // if entry_mem has a size of 2, double check both entries
            else if (entry_mem->size() == 2){
                // same ids 
                if ((*entry_mem)[0] == (*entry_mem)[1]){
                    // go back to asking for first street name
                    gtk_statusbar_push (status,0,"Invalid entry: Please enter different street names");
                    gtk_label_set_text (label, "Please enter the first street name");
                    // initialize entry_mem
                    entry_mem->clear();
                }
                // different ids
                else{
                    //std::cout<<"Search done\n";  
                    // call the function to find intersections of two streets
                    std::vector<int> intersections_found = find_intersections_of_two_streets(std::make_pair((*entry_mem)[0],(*entry_mem)[1]));
                    // initialize entry_mem
                    entry_mem->clear();
                    // if empty, no intersections
                    if (intersections_found.empty()){
                        gtk_label_set_text (label,"Search done:\nNo intersections between two streets");
                        gtk_widget_hide((GtkWidget*)status);
                        gtk_widget_hide((GtkWidget*)widget);
                    }
                    // if not empty, hide window, highlight intersections
                    else{
                        // directions drawing on
                        memory.streets_with_directions.push_back(street_id);
                        // get and hide the window
                        GtkWidget* window = (GtkWidget*)application->get_object("IntersectionsSearch");
                        gtk_widget_hide(window);
                        // call the function present the intersections found
                        IntersectionsSearchResult(intersections_found, application);
                    }
                }
            }
        }
        // if entry not valid, update status bar
        else{
            gtk_statusbar_push (status,0,"Invalid entry: Please follow the format in the dropdown menu");
        }
    }
}

// callback functio text changed on pop up entry, it updates the auto completion
void StreetsEntryChange_callback(GtkEntry* widget, ezgl::application *application){
    // intialize status bar
    GtkStatusbar* status = (GtkStatusbar*)application->get_object("Popup_StatusBar");
    gtk_statusbar_push (status,0,"");
    // get the auto completion and the auto completion list
    GtkEntryCompletion* completion = gtk_entry_get_completion(widget);
    std::unordered_map<std::string, int>* last_list_mem = &(memory.last_autocompletion_list);
    GtkListStore*       list       = (GtkListStore*) gtk_entry_completion_get_model(completion);
    // get the entry
    std::string entryText(gtk_entry_get_text(widget));
    //std::cout<<entryText<<std::endl;
    // if a bracket exist, stop updating the auto completion
    if (entryText.find('(') != std::string::npos){
        return;
    }
    // get all the auto completions (if no bracket)
    std::vector<int> streets_idxs = find_street_ids_from_partial_street_name(entryText);
    // clear and the widget list store and global variable list store
    gtk_list_store_clear (list);
    last_list_mem->clear();
    // fill the widget list store and global variable list store
    GtkTreeIter iter;
    int i;
    for(i=0; i<streets_idxs.size(); i++) {
        gtk_list_store_append(list, &iter);
        int street_idx = streets_idxs[i];
        std::string street_name = getStreetName(street_idx);
        street_name += " (";
        street_name += std::to_string(street_idx);
        street_name += ')';
        const char* street_name_idx = street_name.c_str();
        gtk_list_store_set(list, &iter, 0, street_idx, 1, street_name_idx, -1);
        last_list_mem->insert(std::make_pair(street_name_idx, street_idx));
    }
}

// it updates the global bool variable in memory, so that railways are drawn
void Railways_CheckButton_callback(GtkToggleButton* widget, ezgl::application *application){
    bool active=gtk_toggle_button_get_active(widget);
    if (active){
        memory.layer_railway_subway = true;
    }
    else{
        memory.layer_railway_subway = false;
    }
    application->refresh_drawing();
}

// it updates the global bool variable in memory, so that motorway are highlighted
void Motorways_CheckButton_callback(GtkToggleButton* widget, ezgl::application *application){
    bool active=gtk_toggle_button_get_active(widget);
    if (active){
        for (int i = 0; i < highways_major.size(); ++i){
            if (highways_major[i].tags.find("highway")->second == "motorway"){
                highways_major[i].highlight = true;
            }
        }
    }
    else{
        for (int i = 0; i < highways_major.size(); ++i){
            if (highways_major[i].tags.find("highway")->second == "motorway"){
                highways_major[i].highlight = false;
            }
        }
    }
    application->refresh_drawing();
}

// it updates the global bool variable in memory, so that water body are highlighted
void WaterBody_CheckButton_callback(GtkToggleButton* widget, ezgl::application *application){
    bool active=gtk_toggle_button_get_active(widget);
    if (active){
        memory.layer_water_body = true;
    }
    else{
        memory.layer_water_body = false;
    }
    application->refresh_drawing();
}

// it updates the global bool variable in memory, so that all pois are highlighted
void POI_CheckButton_callback(GtkToggleButton* widget, ezgl::application *application){
    bool active=gtk_toggle_button_get_active(widget);
    if (active){
        memory.layer_poi = true;
    }
    else{
        memory.layer_poi = false;
    }
    application->refresh_drawing();
}

// it forces the auto completion to always show the correct list we stored
gboolean forced_auto_completion(GtkEntryCompletion */*completion*/, const gchar */*key*/, GtkTreeIter */*iter*/, gpointer /*user_data*/){
    return true;
}

// done searching streets, reflect on the canvas
void IntersectionsSearchResult(std::vector<int> intersections_found, ezgl::application *application){
    //un-highlight the last highlighted intersection
    for (int i = 0; i < memory.last_highlighted_intersections.size(); ++i){
        intersections[memory.last_highlighted_intersections[i]].highlight = false;
    }
    memory.last_highlighted_intersections.clear();
    // set new value of last highlighted
    for (int i = 0; i < intersections_found.size(); ++i){
        memory.last_highlighted_intersections.push_back(intersections_found[i]);
    }
    // highlight currently found intersections
    for (int i = 0; i < intersections_found.size(); ++i){
        intersections[intersections_found[i]].highlight = true;
        //print intersection info
        //std::cout << intersections_found[i] << "  " << intersections[intersections_found[i]].name <<std::endl;
    }
    // change the visible to the point highlighted
    ezgl::rectangle visible_world = application->get_renderer()->get_visible_world();
    float visible_width  = visible_world.width();
    float visible_height = visible_world.height();
    float desired_width  = 100;
    float desired_height = 100/visible_width*visible_height;
    ezgl::point2d starting_point(intersections[intersections_found[0]].x_ - desired_width/2,
                                 intersections[intersections_found[0]].y_ - desired_height/2);
    ezgl::rectangle new_world_view(starting_point, desired_width, desired_height);
    application->get_renderer()->set_visible_world(new_world_view);
    application->refresh_drawing();
    return;
}

void PathFind_Button_callback(GtkButton* widget, ezgl::application *application){
    if      (memory.path_finding_state == 0){
        // update the label and path finding state
        gtk_button_set_label((GtkButton*)widget , "Path Find:\ndriving");
        memory.path_finding_state = 1;
        memory.path_finding_intersections.clear();
        memory.path_found.clear();
        memory.walk_path_found.clear();
    }
    else if (memory.path_finding_state == 1){
        // update the label and path finding state
        gtk_button_set_label((GtkButton*)widget , "Path Find:\nwalking+driving");
        memory.path_finding_state = 2;
        memory.path_finding_intersections.clear();
        memory.path_found.clear();
        memory.walk_path_found.clear();
    }
    else if (memory.path_finding_state == 2){
        // update the label and path finding state
        gtk_button_set_label((GtkButton*)widget , "Path Find:\nOFF");
        memory.path_finding_state = 0;
        memory.path_finding_intersections.clear();
        memory.path_found.clear();
        memory.walk_path_found.clear();
    }
    application->refresh_drawing();
    return;
}

// Test callback, feel free to modify and use it for any testing
void Test_callback(GtkEntry* /*widget*/, ezgl::application */*application*/){
    std::cout<<"it's alive!!!\n";
}

/*************************Helper Functions*************************/

// uses global variable avg_lat (in radians)
// uses parameter lon (in degrees)
float lon_from_x(double x) {
    float lon = x / EARTH_RADIUS_METERS / std::cos(avg_lat) / DEGREE_TO_RADIAN; 
    return lon; 
}

// uses parameter lat (in degrees)
float lat_from_y(double y) {
    float lat = y / DEGREE_TO_RADIAN / EARTH_RADIUS_METERS;
    return lat; 
}

// uses global variable avg_lat (in radians)
// uses parameter lon (in degrees)
float x_from_lon(float lon) {
    float x = lon * DEGREE_TO_RADIAN * std::cos(avg_lat) * EARTH_RADIUS_METERS;
    return x;
}

// uses parameter lat (in degrees)
float y_from_lat(float lat) {
    float y = lat * DEGREE_TO_RADIAN * EARTH_RADIUS_METERS;
    return y;
}
