/* 
 * Copyright 2020 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated 
 * documentation files (the "Software") in course work at the University 
 * of Toronto, or for personal use. Other uses are prohibited, in 
 * particular the distribution of the Software either publicly or to third 
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "m1.h"
#include "m1_more.h"
#include <algorithm>    // std::sort

// a vector[IntersectionIndex], each intersection vector stores StreetSegID ints
std::vector<std::vector<int>> intersection_street_segments;

// a vector[StreetIndex], each street vector stores streetSegmentIDs
std::vector<std::vector<int>> street_street_segments;

// a vector[StreetSegIndex], each stores streetIndex
std::vector<int> street_segments_street;

// a vector[StreetIndex], each street vector stores IntersectionIDs
std::vector<std::vector<int>> street_intersections;

// a vector[streetSegIndex], each element stores distance
std::vector<double> streetSeg_length;

// a vector[streetSegIndex], each element stores travel time
std::vector<double> streetSeg_time;

// a map <Street_Name,StreetIndex> for all streets
std::multimap<std::string, int> streetID_streetName;

// a vector[streetIndex], each element stores distance
std::vector<double> streetID_streetLength;

// a variable used to store all possible all features
std::vector<std::vector<LatLon>> featureID_featurePts;

// a map, each stores <node ID, node>
std::unordered_map<OSMID, const OSMNode*> NodeID_Node;

// a map, each stores <way ID, way>
std::unordered_map<OSMID, const OSMWay*> WayID_Way;

// a map, each stores <way ID, way length>
std::unordered_map<OSMID, double> WayID_length;

// a vector[Node_idx] storing Nodes
// Node_idx is the same as intersection idx
std::vector<Node> nodes;

// a vector[Edge_idx] storing Edges
// Edge_idx is not the same as segment idx
std::vector<Edge> edges;

// a vector[Node_idx] storing Nodes for walking
// Node_idx is the same as intersection idx
std::vector<Node> nodes_w;

// a vector[Edge_idx] storing Edges for walking
// Edge_idx is not the same as segment idx
std::vector<Edge> edges_w;

// the maximum speed_limit in the city
double max_speed_limit;

bool load_map(std::string map_path) {
    //Indicates whether the map has loaded successfully
    bool load_successful = loadStreetsDatabaseBIN(map_path); 
    if (!load_successful) return false;
    std::string osm_path;
    for (int i=0; map_path[i]!='.'; ++i)
        osm_path+=map_path[i];
    osm_path+=".osm.bin";
    loadOSMDatabaseBIN(osm_path);
    
    intersection_street_segments.resize(getNumIntersections());
    street_street_segments.resize(getNumStreets());
    street_intersections.resize(getNumStreets());
    //streetSeg_time.resize(getNumStreetSegments());
    //streetID_streetName.resize(getNumStreets());
    featureID_featurePts.resize(getNumFeatures());
    streetID_streetLength.resize(getNumStreets(),0);
    
    // std::vector<std::vector<int>> intersection_street_segments;
    for (int intersection = 0; intersection < getNumIntersections(); ++intersection) {
        for (int i = 0; i < getIntersectionStreetSegmentCount(intersection); ++i) {
            int ss_id = getIntersectionStreetSegment(intersection, i);
            intersection_street_segments[intersection].push_back(ss_id);
        }
    }
    
    //std::vector<std::vector<int>> street_street_segments;
    for (int streetSegment = 0; streetSegment < getNumStreetSegments(); ++streetSegment) {       
        int street_id = getInfoStreetSegment(streetSegment).streetID; 
        street_street_segments[street_id].push_back(streetSegment);
        street_segments_street.push_back(street_id);
    }
    
    // std::vector<std::vector<int>> street_intersections;
    for (int street_id = 0; street_id < getNumStreets(); ++street_id){
        std::vector<int> Segments=street_street_segments[street_id];
        int NumSegments=Segments.size();
        for (int i=0; i<NumSegments;++i) {
            InfoStreetSegment streetSeg = getInfoStreetSegment(Segments[i]);
            street_intersections[street_id].push_back(streetSeg.to);
            street_intersections[street_id].push_back(streetSeg.from);
        }
        //remove duplicates
        std::sort(street_intersections[street_id].begin(),street_intersections[street_id].end());
        street_intersections[street_id].erase( 
                                                std::unique(street_intersections[street_id].begin(),
                                                            street_intersections[street_id].end()),
                                                street_intersections[street_id].end());
    }
    
    // std::vector<double> streetSeg_length;
    // std::vector<double> streetSeg_time;
    // std::vector<double> streetID_streetLength;
    for (int streetSegment = 0; streetSegment < getNumStreetSegments(); ++streetSegment){ 
        double length = find_street_segment_length(streetSegment);
        streetSeg_length.push_back(length);
        InfoStreetSegment this_Seg_info = getInfoStreetSegment(streetSegment);
        streetSeg_time.push_back(length/(this_Seg_info.speedLimit)*3.6);
        streetID_streetLength[this_Seg_info.streetID]+=length;
    }
    
    // std::multimap<std::string, int> streetID_streetName;
    for (int streetID = 0; streetID < getNumStreets(); ++streetID) {
        std::string streetName = getStreetName(streetID);
        std::string this_name;
        for (int i=0; i<streetName.size(); ++i){
            char this_char = streetName[i];
            if (this_char != ' '){
                if (islower(this_char))
                    this_char=toupper(this_char);
                this_name+=this_char;
            }
        }
        streetID_streetName.insert(std::make_pair(this_name,streetID));
        /*
        std::cout<<streetID<<std::endl;
        std::cout<<"   "<<this_name<<std::endl;
        std::cout<<"   "<<streetID_streetLength[streetID]<<std::endl;
         */
    }
    //std::vector<std::vector<LatLon>> featureID_featurePts;
    for (int feature = 0; feature < getNumFeatures(); ++feature) {
        for(int i = 0; i < getFeaturePointCount(feature); ++i) {
            LatLon featurePoint = getFeaturePoint(i, feature);       
            featureID_featurePts[feature].push_back(featurePoint);  
        }
    }
    
    // std::map<OSMID, int> OSMID_NodeIdx;
    for (int nodeCt= 0; nodeCt < getNumberOfNodes(); ++nodeCt) {
        const OSMNode* osmnode = getNodeByIndex(nodeCt);
        NodeID_Node.insert(std::make_pair(osmnode->id(), osmnode));
    }
    
    // std::map<OSMID, int> wayID_length;
    // std::unordered_map<OSMID, const OSMWay*> WayID_Way;
    for (int wayCt= 0; wayCt < getNumberOfWays(); ++wayCt) {
        const OSMWay* this_way = getWayByIndex(wayCt);
        WayID_Way.insert(std::make_pair(this_way->id(), this_way));
        std::vector<OSMID> these_node_ids = getWayMembers(this_way);
        double distance=0;
        for (int i=0; i<these_node_ids.size()-1; ++i){
            LatLon node_first  = NodeID_Node.find(these_node_ids[i])  ->second ->coords();
            LatLon node_second = NodeID_Node.find(these_node_ids[i+1])->second ->coords();
            distance += find_distance_between_two_points(std::make_pair(node_first,node_second));
        }
        WayID_length.insert(std::make_pair(this_way->id(), distance));
    }
    pathFind_load();
    return true;
}

void pathFind_load(){
    nodes.resize(getNumIntersections());
    nodes_w.resize(getNumIntersections());
    
    // std::vector<Edge> edges;
    // std::vector<Node> nodes; => outEdges only
    for(size_t i = 0; i < getNumStreetSegments(); ++i) {
        // get the info
        InfoStreetSegment this_info = getInfoStreetSegment(i);
        // declare a new edge
        Edge new_edge;
               
        //store edge travel time 
        new_edge.edgeTravelTime = streetSeg_time[i];
                
        // store one direction edge
        new_edge.idx_seg = i;
        new_edge.from    = this_info.from;
        new_edge.to      = this_info.to;        
        
        // push one direction edge
        edges.push_back(new_edge);
        
        // store into outEdges of "from" Node
        // because current edge is always the last element, edge_idx = size-1
        nodes[new_edge.from].outEdges.push_back(edges.size()-1);
        
        if (!(this_info.oneWay)){
            // store opposite direction edge
            new_edge.from = this_info.to;
            new_edge.to   = this_info.from;

            // push opposite direction edge
            edges.push_back(new_edge);
            
            // store into outEdges of "from" Node
            // because current edge is always the last element, edge_idx = size-1
            nodes[new_edge.from].outEdges.push_back(edges.size()-1);
        }
    }
    
    // std::vector<Node> nodes;
    for(size_t i = 0; i < nodes.size(); ++i) {
        nodes[i].idx_pnt = i;
        //initialize bestTime of all nodes to 100000000.00
        nodes[i].bestTime = 100000000.00; //for comparison of less travel_time
        //initial visited to false 
        //nodes[i].visited = false; 
    }
    
    // std::vector<Edge> edges_w;
    // std::vector<Node> nodes_w; => outEdges only
    for(size_t i = 0; i < getNumStreetSegments(); ++i) {
        // get the info
        InfoStreetSegment this_info = getInfoStreetSegment(i);
        // declare a new edge
        Edge new_edge;
               
        //store edge travel time 
        new_edge.edgeTravelTime = streetSeg_time[i];
                
        // store one direction edge
        new_edge.idx_seg = i;
        new_edge.from    = this_info.from;
        new_edge.to      = this_info.to;        
        
        // push one direction edge
        edges_w.push_back(new_edge);
        
        // store into outEdges of "from" Node
        // because current edge is always the last element, edge_idx = size-1
        nodes_w[new_edge.from].outEdges.push_back(edges_w.size()-1);
        
        // store opposite direction edge
        new_edge.from = this_info.to;
        new_edge.to   = this_info.from;

        // push opposite direction edge
        edges_w.push_back(new_edge);

        // store into outEdges of "from" Node
        // because current edge is always the last element, edge_idx = size-1
        nodes_w[new_edge.from].outEdges.push_back(edges_w.size()-1);
    }
    
    // std::vector<Node> nodes_w;
    for(size_t i = 0; i < nodes_w.size(); ++i) {
        nodes_w[i].idx_pnt = i;
        //initialize bestTime of all nodes to 100000000.00
        nodes_w[i].bestTime = 100000000.00; //for comparison of less travel_time
        //initial visited to false 
        //nodes[i].visited = false; 
    }
    
    //find the max (double speedLimit)speed limit (in km/h) of the city 
    //initialize max to the speed limit of streetseg[0]
    InfoStreetSegment this_info = getInfoStreetSegment(0);
    max_speed_limit = this_info.speedLimit;
    for(int i = 1; i < getNumStreetSegments(); ++i) { 
       InfoStreetSegment seg_info = getInfoStreetSegment(i);
       double this_seg_speed_limit = seg_info.speedLimit;
       if(max_speed_limit < this_seg_speed_limit){
           max_speed_limit = this_seg_speed_limit;
       }
    }
}

void close_map() {   
    //Clean-up your map related data structures here  
         intersection_street_segments.clear();
         street_street_segments.clear();
         street_intersections.clear();
         streetSeg_time.clear();
         streetID_streetName.clear();
         streetID_streetLength.clear();
         featureID_featurePts.clear();
         NodeID_Node.clear();
         WayID_Way.clear();
         WayID_length.clear();
    closeStreetDatabase(); 
    closeOSMDatabase();
}

//
double find_distance_between_two_points(std::pair<LatLon, LatLon> points){
    std::pair<XY_,XY_> points_XY=LatLon_to_XY(points);
    return EARTH_RADIUS_METERS * std::sqrt(std::pow(points_XY.first.x_-points_XY.second.x_,2)+std::pow(points_XY.first.y_-points_XY.second.y_,2));
}

double find_street_segment_length(int street_segment_id){
    InfoStreetSegment infoStreetSeg=getInfoStreetSegment(street_segment_id);
    IntersectionIndex from = infoStreetSeg.from;
    IntersectionIndex to = infoStreetSeg.to;
    int curvePointCount = infoStreetSeg.curvePointCount;
    
    double total_length=0;
    std::pair<LatLon, LatLon> two_points;
    
    if (curvePointCount){
        // distance between "from" and "0'th point"
        two_points.first = getIntersectionPosition(from);
        two_points.second = getStreetSegmentCurvePoint(0, street_segment_id);
        total_length += find_distance_between_two_points(two_points);
        // distance between "0th" and "curvePointCount-1'th point"
        for (int i=0; i<(curvePointCount-1); i++){
            two_points.first = getStreetSegmentCurvePoint(i, street_segment_id);
            two_points.second = getStreetSegmentCurvePoint(i+1, street_segment_id);
            total_length += find_distance_between_two_points(two_points);
        }
        //distance between "curvePointCount-1'th point" and "to"
        two_points.first = getStreetSegmentCurvePoint(curvePointCount-1, street_segment_id);
        two_points.second = getIntersectionPosition(to);
        total_length += find_distance_between_two_points(two_points);
    } else {
        two_points.first = getIntersectionPosition(from);
        two_points.second = getIntersectionPosition(to);
        total_length += find_distance_between_two_points(two_points);
    }
    return total_length;
}

double find_street_segment_travel_time(int street_segment_id){
    return streetSeg_time[street_segment_id];
}

int find_closest_intersection(LatLon my_position){
    double closest_distance= 2000000;
    int closest_intersection= -1;
    std::pair<LatLon, LatLon> two_points;
    two_points.first=my_position;
    for (int i=0; i<getNumIntersections(); ++i){
        two_points.second = getIntersectionPosition(i);
        double current_distance = find_distance_between_two_points(two_points);
        if (current_distance<closest_distance && current_distance < 10){
            closest_distance=current_distance;
            closest_intersection=i;
        }
    }
    return closest_intersection;
}

int find_closest_POI(LatLon my_position){
    double closest_distance= 2000000;
    int closest_poi= -1;
    std::pair<LatLon, LatLon> two_points;
    two_points.first=my_position;
    for (int i=0; i<getNumPointsOfInterest(); ++i){
        two_points.second = getPointOfInterestPosition(i);
        double current_distance = find_distance_between_two_points(two_points);
        if (current_distance<closest_distance && current_distance < 10){
            closest_distance=current_distance;
            closest_poi=i;
        }
    }
    return closest_poi;
}

std::vector<int> find_street_segments_of_intersection(int intersection_id){
    return intersection_street_segments[intersection_id];
}

std::vector<std::string> find_street_names_of_intersection(int intersection_id){
    std::vector<std::string> street_names;
    std::string streetName;
    std::vector<int> street_segments_ids = intersection_street_segments[intersection_id];
    for(int i = 0; i < street_segments_ids.size(); i++) {
        streetName = getStreetName(getInfoStreetSegment(street_segments_ids[i]).streetID);
        street_names.push_back(streetName);
    }
    return street_names;
}

//Returns true if you can get from intersection_ids.first to intersection_ids.second using a single 
//street segment (hint: check for 1-way streets too)
bool are_directly_connected(std::pair<int, int> intersection_ids){
   //for the intersections find the street segments
    int streetSegmentCount1 = getIntersectionStreetSegmentCount(intersection_ids.first);//use size?
    std::vector<int> streetSegmentId1 = intersection_street_segments[intersection_ids.first];
    
    //corner case: an intersection is considered to be connected to itself  
    if(intersection_ids.first == intersection_ids.second)
        return true;
    
    for(int i = 0; i < streetSegmentCount1; ++i){
         IntersectionIndex from = getInfoStreetSegment(streetSegmentId1[i]).from;
         IntersectionIndex to = getInfoStreetSegment(streetSegmentId1[i]).to;
                          
            if((from == intersection_ids.first) && (to == intersection_ids.second) )
                    return true; 
            if((from == intersection_ids.second) && (to == intersection_ids.first) ){
                   if(getInfoStreetSegment(streetSegmentId1[i]).oneWay){
                        return false; 
                   }
                   else 
                       return true;
            }
    }
    
    return false;
} 
    

//Returns all intersections reachable by traveling down one street segment 
//from given intersection (hint: you can't travel the wrong way on a 1-way street)
//the returned vector should NOT contain duplicate intersections
std::vector<int> find_adjacent_intersections(int intersection_id){
    //find all street segments of the intersection 
    std::vector<int> adjacentIntersections;
    StreetSegmentIndex streetSegment_id;
    IntersectionIndex from;
    IntersectionIndex to;
    bool oneway;
    for (int i = 0; i < getIntersectionStreetSegmentCount(intersection_id); ++i){
        streetSegment_id = getIntersectionStreetSegment(intersection_id, i);
        from = getInfoStreetSegment(streetSegment_id).from;
        to = getInfoStreetSegment(streetSegment_id).to;
        oneway = getInfoStreetSegment(streetSegment_id).oneWay;
        if(oneway) {
            if(from == intersection_id) {
                adjacentIntersections.push_back(to);
            }
        } else {
            if(from != intersection_id)
                adjacentIntersections.push_back(from);
            else
                adjacentIntersections.push_back(to);
        }
        
    //remove duplicate intersections
     std::sort(adjacentIntersections.begin(), adjacentIntersections.end());
     adjacentIntersections.erase( std::unique(adjacentIntersections.begin(), adjacentIntersections.end()), adjacentIntersections.end());
        
    } 
    return adjacentIntersections;
}

std::vector<int> find_street_segments_of_street(int street_id){
    return street_street_segments[street_id];
}
//  Leo
std::vector<int> find_intersections_of_street(int street_id){
    return street_intersections[street_id];
}

std::vector<int> find_intersections_of_two_streets(std::pair<int, int> street_ids){
    std::vector<int> intsersections_first =find_intersections_of_street(street_ids.first);
    std::vector<int> intsersections_second=find_intersections_of_street(street_ids.second);
    std::vector<int> intsersections_result;
    bool done_this; 
    for (int i=0; i<intsersections_first.size(); ++i){
        done_this=false;
        for (int j=0; j<intsersections_second.size() && !done_this; ++j){
            if (intsersections_first[i]==intsersections_second[j]){
                intsersections_result.push_back(intsersections_first[i]);
                done_this=true;
            }
        }
    }
    return intsersections_result;
}

std::vector<int> find_street_ids_from_partial_street_name(std::string street_prefix){
    std::vector<int> result_d;
    std::string Prefix;
    for (int i=0; i<street_prefix.size(); ++i){
        char this_char = street_prefix[i];
        if (this_char != ' '){
            if (islower(this_char))
                this_char=toupper(this_char);
            Prefix+=this_char;
        }
    }
    std::string Prefix_after = Prefix;
    ++Prefix_after[Prefix_after.length() - 1];
    auto start_ptr = streetID_streetName.lower_bound(Prefix);
    auto end_ptr   = streetID_streetName.lower_bound(Prefix_after);
    for (auto ptr=start_ptr; ptr != end_ptr; ++ptr)
        result_d.push_back(ptr->second);
    return result_d;
}

double find_feature_area(int feature_id){
    // check if closed 
    
    int totalPtsCount = getFeaturePointCount(feature_id);
    LatLon latLonPoint0= featureID_featurePts[feature_id][0];
    LatLon latLonPointLast= featureID_featurePts[feature_id][totalPtsCount-1];
    double sum = 0.0;
    if((latLonPoint0.lat()==latLonPointLast.lat()) && (latLonPoint0.lon()==latLonPointLast.lon())) {
        std::vector<XY_> featurePts=LatLon_to_XY(featureID_featurePts[feature_id]); 
        for (int i=0; i<= totalPtsCount-2; ++i) {
            sum+=(featurePts[i].x_) * EARTH_RADIUS_METERS * (featurePts[i+1].y_) * EARTH_RADIUS_METERS - 
                 (featurePts[i].y_) * EARTH_RADIUS_METERS * (featurePts[i+1].x_) * EARTH_RADIUS_METERS;
        }
        
        sum=fabs(sum)/2.0;
        return sum;
    }
    else {
        return 0;
    }
}

double find_way_length(OSMID way_id)
{
    return WayID_length.find(way_id)->second;
}

//send in a pair of LatLon, give out a pair of XY(int pair))
std::pair<XY_,XY_> LatLon_to_XY(std::pair<LatLon,LatLon> points){
    double lat_first = DEGREE_TO_RADIAN * points.first.lat();
    double lon_first = DEGREE_TO_RADIAN * points.first.lon();
    double lat_second = DEGREE_TO_RADIAN * points.second.lat();
    double lon_second = DEGREE_TO_RADIAN * points.second.lon();
    double lat_avg=(lat_first+lat_second)/2;
    std::pair<XY_,XY_> points_XY;
    points_XY.first.x_=lon_first * std::cos(lat_avg);
    points_XY.first.y_=lat_first;
    points_XY.second.x_=lon_second * std::cos(lat_avg);
    points_XY.second.y_=lat_second;
    return points_XY;
}


std::vector<XY_> LatLon_to_XY(std::vector<LatLon> points){
    double lat_sum=0, lat_avg=0;
    int vector_size=points.size();
    std::vector<XY_> points_XY;
    //find lat_avg in radian
    for (int i=0; i<vector_size; ++i) lat_sum += DEGREE_TO_RADIAN * points[i].lat();
    lat_avg=lat_sum/vector_size;
    //push them into result
    XY_ one_point;
    for (int i=0; i<vector_size; ++i){
        one_point.x_ = points[i].lon() * DEGREE_TO_RADIAN * std::cos(lat_avg);
        one_point.y_ = points[i].lat() * DEGREE_TO_RADIAN;
        points_XY.push_back(one_point);
    }
    return points_XY;
}