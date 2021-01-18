#include "m3.h"
#include "m3_more.h"
#include <queue> //std::priority_queue
#define NO_EDGE -1

// Returns the time required to travel along the path specified, in seconds.
// The path is given as a vector of street segment ids, and this function can
// assume the vector either forms a legal path or has size == 0.  The travel
// time is the sum of the length/speed-limit of each street segment, plus the
// given turn_penalty (in seconds) per turn implied by the path.  If there is
// no turn, then there is no penalty. Note that whenever the street id changes
// (e.g. going from Bloor Street West to Bloor Street East) we have a turn.
double compute_path_travel_time(const std::vector<StreetSegmentIndex>& path, 
                                const double turn_penalty){
    //double temp = 0.0;
    //return temp;
    double travel_time = 0.0;
    double seg_travel_time = 0.0;
    int turns = 0; 
    //empty vector
    if(path.size() == 0){
        return 0;
    }
    
    //count turns in the path (whenever the street id changes) 
    for(int i=0; i< (path.size()-1); ++i){
        int this_seg_id = path[i];
        int next_seg_id = path[i+1];
        //calculate turns by comparing street id with its the next street id. 
        if(street_segments_street[this_seg_id] != street_segments_street[next_seg_id]){
            turns = turns +1;
        }
    }
     
    //adds up travel time for all the segs in the path
    for(int segCount=0; segCount< path.size(); ++segCount){ 
        // DAVE's COMMENT: don't calculate the division here, go to M1, 
        // there is already a function find_street_segment_travel_time()
        // and a data structure streetSeg_time[]
        // use either one of them to substitute the division.
        seg_travel_time += streetSeg_time[path[segCount]];
    }    
        
    //The travel time is the sum of the length/speed-limit of each street segment, plus the
    // given turn_penalty (in seconds) per turn implied by the path. 
    travel_time =  seg_travel_time + (turns*turn_penalty);
    //std::cout<<"\ntravel_time"<<travel_time;
    return travel_time;
}

// Returns the time required to "walk" along the path specified, in seconds.
// The path is given as a vector of street segment ids. The vector can be of
// size = 0, and in this case, it the function should return 0. The walking
// time is the sum of the length/<walking_speed> for each street segment, plus
// the given turn penalty, in seconds, per turn implied by the path. If there
// is no turn, then there is no penalty.  As mentioned above, going from Bloor
// Street West to Bloor street East is considered a turn
double compute_path_walking_time(const std::vector<StreetSegmentIndex>& path, 
                                 const double walking_speed, 
                                 const double turn_penalty){  
    double walking_time = 0.0;
    double seg_walking_time = 0.0;
    int turns = 0; 
   //empty vector
    if(path.size() == 0){
        return 0.0;
    }
       
    //count turns in the path (whenever the street id changes) 
    for(int i=0; i< (path.size()-1); ++i){
        int this_seg_id = path[i];
        int next_seg_id = path[i+1];
        //calculate turns by comparing street id with its the next street id. 
        if(street_segments_street[this_seg_id] != street_segments_street[next_seg_id]){
            turns = turns +1;
        }
    }
       
    //adds up walking time for all the segs in the path
    for(int segCount=0; segCount< path.size(); ++segCount){       
        seg_walking_time += (streetSeg_length[path[segCount]] / walking_speed);
    }    
        
    //The walking time is the sum of the length/<walking_speed> for each street segment, plus
    // the given turn penalty, in seconds, per turn implied by the path
    walking_time =  seg_walking_time + (turns*turn_penalty);
    return walking_time;
}

// Returns a path (route) between the start intersection and the end
// intersection, if one exists. This routine should return the shortest path
// between the given intersections, where the time penalty to turn right or
// left is given by turn_penalty (in seconds).  If no path exists, this routine
// returns an empty (size == 0) vector.  If more than one path exists, the path
// with the shortest travel time is returned. The path is returned as a vector
// of street segment ids; traversing these street segments, in the returned
// order, would take one from the start to the end intersection.
std::vector<StreetSegmentIndex> find_path_between_intersections(
		          const IntersectionIndex intersect_id_start, 
                  const IntersectionIndex intersect_id_end,
                  const double turn_penalty){
   std::vector<StreetSegmentIndex> empty_path; 
   //std::cout<<"\n start:"<<intersect_id_start<<"\n end:"<<intersect_id_end<<std::endl;
    //reset all elements 
    for(int i=0; i < nodes.size(); ++i){
        //reset reachingEdge to NO_EDGE
        nodes[i].reachingEdge= NO_EDGE;
        //reset bestTime of all nodes to 100000000.00
        nodes[i].bestTime = 100000000.00;
    }
    //define waveFront with root node with least travel time. 
    std::priority_queue<WaveElem, std::vector<WaveElem>, compareEstimateTravelTime> waveFront; //Nodes to explore next
   
    /************ A***************/
    
    
 if(max_speed_limit != 0){      
    //distance from source to destination 
    LatLon  start_position = getIntersectionPosition(intersect_id_start);
    LatLon  end_position = getIntersectionPosition(intersect_id_end);
    std::pair<LatLon, LatLon> points = std::make_pair(start_position, end_position);
    //Returns the distance between two coordinates in meters
    double distance_start_to_end_m = find_distance_between_two_points(points);
    double distance_start_to_end_km = distance_start_to_end_m/1000;
    
    //estimateTravelTime for source ( travel time + distance/ max speed limit of the city)
    double source_estimateTravelTime = 0.0;
    source_estimateTravelTime = 0.0 + (distance_start_to_end_km / max_speed_limit)*3600;
    /************ A***************/
    
    
    waveFront.push(WaveElem(&(nodes[intersect_id_start]), NO_EDGE, 0.0, source_estimateTravelTime)); //source node
    
    while(!waveFront.empty()){
        WaveElem current_node = waveFront.top();
        //remove node from waveFront
        waveFront.pop();
        
        //update the edge with shorter travel time
        if(current_node.travelTime < current_node.node->bestTime){
            current_node.node->bestTime = current_node.travelTime;
            current_node.node->reachingEdge = current_node.edgeID;
            
            //reach destination
            if(current_node.node->idx_pnt == intersect_id_end){ 
                return std::get<0>(path_search_result(intersect_id_end));     
            }

            //go through all the outEdges of the current_node
            for(int i=0; i< (current_node.node->outEdges.size()); ++i){
                //get to_node of one outEdge
                Node* to_node = &(nodes[ edges[((current_node.node)->outEdges)[i]].to ]);                                       

                //update the travel time & estimateTravelTime
                double totalTravelTime = 0.0;
                
                //outEdge[i] edge travel time 
                double this_edgeTravelTime = edges[((current_node.node)->outEdges)[i]].edgeTravelTime;
                
                //check if turn
                //compare street id of this_seg with reaching_edge of current_node
                int turn = 0;
                if (current_node.edgeID != NO_EDGE){
                    int this_seg_id = edges[current_node.edgeID].idx_seg;
                    int next_seg_id = edges[current_node.node->outEdges[i]].idx_seg;
                    //std::cout << "\n\n\n -------------------\n"<<this_seg_id<<"\n"<<current_node.edgeID<<"\n -------------------\n\n\n";
                    //calculate turns by comparing street id with its the next street id. 
                    if(street_segments_street[this_seg_id] != street_segments_street[next_seg_id]){
                            turn = 1;
                    }
                }
                totalTravelTime = (current_node.node->bestTime) + this_edgeTravelTime + (turn*turn_penalty);

                
                /************ A***************/
                //calculate estimateTravelTime for outEdge[i]
                int to_node_id = to_node->idx_pnt;
                
                //distance from to_node to end_node
                LatLon  to_node_position = getIntersectionPosition(to_node_id);
                std::pair<LatLon, LatLon> points_new = std::make_pair(to_node_position, end_position);
                //the distance between two coordinates in meters
                double distance_toNode_to_end_m = find_distance_between_two_points(points_new);
                double distance_toNode_to_end_km = distance_toNode_to_end_m/1000;
                
                //estimateTravelTime for source ( travel time + distance/ max speed limit of the city)
                double toNode_estimateTravelTime = (distance_toNode_to_end_km / max_speed_limit)*3600;
                double total_estimateTravelTime = 0.0;
                total_estimateTravelTime = totalTravelTime + toNode_estimateTravelTime;
                /************ A***************/
                
                //update waveFront 
                waveFront.push(WaveElem(to_node, current_node.node->outEdges[i], totalTravelTime, total_estimateTravelTime)); 
            }   
        }                       
    }
    return empty_path;
 }
    return empty_path;
}



std::vector<StreetSegmentIndex> find_path_between_intersections_visualized(
		          const IntersectionIndex intersect_id_start, 
                          const IntersectionIndex intersect_id_end,
                          const double turn_penalty,
                          ezgl::application *application){
   std::vector<StreetSegmentIndex> empty_path; 
   //std::cout<<"\n start:"<<intersect_id_start<<"\n end:"<<intersect_id_end<<std::endl;
    //reset all elements 
    for(int i=0; i < nodes.size(); ++i){
        //reset reachingEdge to NO_EDGE
        nodes[i].reachingEdge= NO_EDGE;
        //reset bestTime of all nodes to 100000000.00
        nodes[i].bestTime = 100000000.00;
    }
    //define waveFront with root node with least travel time. 
    std::priority_queue<WaveElem, std::vector<WaveElem>, compareEstimateTravelTime> waveFront; //Nodes to explore next
   
    /************ A***************/
    
    
 if(max_speed_limit != 0){      
    //distance from source to destination 
    LatLon  start_position = getIntersectionPosition(intersect_id_start);
    LatLon  end_position = getIntersectionPosition(intersect_id_end);
    std::pair<LatLon, LatLon> points = std::make_pair(start_position, end_position);
    //Returns the distance between two coordinates in meters
    double distance_start_to_end_m = find_distance_between_two_points(points);
    double distance_start_to_end_km = distance_start_to_end_m/1000;
    
    //estimateTravelTime for source ( travel time + distance/ max speed limit of the city)
    double source_estimateTravelTime = 0.0;
    source_estimateTravelTime = 0.0 + (distance_start_to_end_km / max_speed_limit)*3600;
    /************ A***************/
    
    
    waveFront.push(WaveElem(&(nodes[intersect_id_start]), NO_EDGE, 0.0, source_estimateTravelTime)); //source node
    
    while(!waveFront.empty()){
        WaveElem current_node = waveFront.top();
        //remove node from waveFront
        waveFront.pop();
        
        // visualize
        ezgl::renderer *g = application->get_renderer();
        if (current_node.edgeID != -1){
            draw_street_segments(edges[current_node.edgeID].idx_seg, g);
            application->flush_drawing();
            std::chrono::milliseconds duration(50);
            std::this_thread::sleep_for(duration);
        }
        
        //update the edge with shorter travel time
        if(current_node.travelTime < current_node.node->bestTime){
            current_node.node->bestTime = current_node.travelTime;
            current_node.node->reachingEdge = current_node.edgeID;
            
            //reach destination
            if(current_node.node->idx_pnt == intersect_id_end){ 
                return std::get<0>(path_search_result(intersect_id_end));     
            }

            //go through all the outEdges of the current_node
            for(int i=0; i< (current_node.node->outEdges.size()); ++i){
                //get to_node of one outEdge
                Node* to_node = &(nodes[ edges[((current_node.node)->outEdges)[i]].to ]);                                       

                //update the travel time & estimateTravelTime
                double totalTravelTime = 0.0;
                
                //outEdge[i] edge travel time 
                double this_edgeTravelTime = edges[((current_node.node)->outEdges)[i]].edgeTravelTime;
                
                //check if turn
                //compare street id of this_seg with reaching_edge of current_node
                int turn = 0;
                if (current_node.edgeID != NO_EDGE){
                    int this_seg_id = edges[current_node.edgeID].idx_seg;
                    int next_seg_id = edges[current_node.node->outEdges[i]].idx_seg;
                    //std::cout << "\n\n\n -------------------\n"<<this_seg_id<<"\n"<<current_node.edgeID<<"\n -------------------\n\n\n";
                    //calculate turns by comparing street id with its the next street id. 
                    if(street_segments_street[this_seg_id] != street_segments_street[next_seg_id]){
                            turn = 1;
                    }
                }
                totalTravelTime = (current_node.node->bestTime) + this_edgeTravelTime + (turn*turn_penalty);

                
                /************ A***************/
                //calculate estimateTravelTime for outEdge[i]
                int to_node_id = to_node->idx_pnt;
                
                //distance from to_node to end_node
                LatLon  to_node_position = getIntersectionPosition(to_node_id);
                std::pair<LatLon, LatLon> points_new = std::make_pair(to_node_position, end_position);
                //the distance between two coordinates in meters
                double distance_toNode_to_end_m = find_distance_between_two_points(points_new);
                double distance_toNode_to_end_km = distance_toNode_to_end_m/1000;
                
                //estimateTravelTime for source ( travel time + distance/ max speed limit of the city)
                double toNode_estimateTravelTime = (distance_toNode_to_end_km / max_speed_limit)*3600;
                double total_estimateTravelTime = 0.0;
                total_estimateTravelTime = totalTravelTime + toNode_estimateTravelTime;
                /************ A***************/
                
                //update waveFront 
                waveFront.push(WaveElem(to_node, current_node.node->outEdges[i], totalTravelTime, total_estimateTravelTime)); 
            }   
        }                       
    }
    return empty_path;
 }
    return empty_path;
}

// it traces back all the street segments that are driven through starting from 
// the intersect_id_end.
// The function returns the indicis of the street segments, the starting
// point correlated to the path, and the total time of the path.
std::tuple<std::vector<StreetSegmentIndex>,int,double> path_search_result(const IntersectionIndex intersect_id_end){
    std::vector<StreetSegmentIndex> path;
    Node* currentNode = &(nodes[intersect_id_end]);
    double totalTime = currentNode -> bestTime;
    int pre_edge = currentNode->reachingEdge;
    int pre_seg;
    //transverse the path
    while(pre_edge != NO_EDGE){
        //update pre_seg
        pre_seg = edges[pre_edge].idx_seg;
        
        path.insert(path.begin(),pre_seg);
        
        // update current_node
        currentNode =  &(nodes[edges[pre_edge].from]);
        // update pre_edge
        pre_edge = currentNode->reachingEdge; 
        
    }
    
    return std::make_tuple(path, currentNode->idx_pnt, totalTime); 
}

// This is an "uber pool"-like function that minimizes driving travel time by 
// walking to a pick-up intersection (within walking_time_limit secs) from
// start_intersection while waiting for the car to arrive.
// The routine first calls find_walkable_inters() to find all the shortest paths 
// from the parameter start_intersection to all the walkable intersections 
// within the distance = walking_speed * walking_time_limit. 
// Then it calls find_path_between_intersections_multi_starts() to find shortest
// driving path from walkable_intersections to the parameter end_intersection.
// Walking in the opposite direction of one-way streets is fine. Driving is not.
// The routine returns a pair of vectors of street segment ids. The first
// vector is the walking path starting at start_intersection and ending at the
// pick-up intersection. The second vector is the driving path from pick-up
// intersection to end_interserction.  Note that a start_intersection can be a
// pick-up intersection. If this happens, the first vector should be empty
// (size = 0).  If there is no driving path found, both returned vectors should
// be empty (size = 0). 
 std::pair<std::vector<StreetSegmentIndex>, std::vector<StreetSegmentIndex>> 
         find_path_with_walk_to_pick_up(
                          const IntersectionIndex start_intersection, 
                          const IntersectionIndex end_intersection,
                          const double turn_penalty,
                          const double walking_speed, 
                          const double walking_time_limit){
     
     // use Dijkstra to find all walkable intersections
     std::vector<int> walkable_inters;//= find_walkable_inters(start_intersection,turn_penalty, walking_speed, walking_time_limit);
     // find the final drive path and final walk_intersections
     std::pair<std::vector<StreetSegmentIndex>,int> drivePath_and_walkInter;// = find_path_between_intersections_multi_starts(walkable_inters,end_intersection,turn_penalty);
     std::vector<StreetSegmentIndex> drivePath = drivePath_and_walkInter.first;
     int walkInter = drivePath_and_walkInter.second;
     // find walk path
     std::vector<int> walkPath  = path_walk_search_result(walkInter);
     // return whole path
     return std::make_pair(walkPath, drivePath);
}
 
std::pair<std::vector<StreetSegmentIndex>, std::vector<StreetSegmentIndex>> 
         find_path_with_walk_to_pick_up_visualized(
                          const IntersectionIndex start_intersection, 
                          const IntersectionIndex end_intersection,
                          const double turn_penalty,
                          const double walking_speed, 
                          const double walking_time_limit,
                          ezgl::application *application){
     
     // use Dijkstra to find all walkable intersections
     std::vector<int> walkable_inters = find_walkable_inters(start_intersection,turn_penalty, walking_speed, walking_time_limit, application);
     // find the final drive path and final walk_intersections
     std::pair<std::vector<StreetSegmentIndex>,int> drivePath_and_walkInter = find_path_between_intersections_multi_starts(walkable_inters,end_intersection,turn_penalty, application);
     std::vector<StreetSegmentIndex> drivePath = drivePath_and_walkInter.first;
     int walkInter = drivePath_and_walkInter.second;
     // find walk path
     std::vector<int> walkPath  = path_walk_search_result(walkInter);
     // return whole path
     return std::make_pair(walkPath, drivePath);
}
 
// This function uses Dijkstra algorithm to find all the shortest path to find
// all the shortest paths  from the parameter start_intersection to all the 
// walkable intersections within the distance = walking_speed * walking_time_limit. 
// The function returns the vector of indicis of those walkable intersections.
 std::vector<int> find_walkable_inters(const IntersectionIndex start_intersection,
                                       const double turn_penalty,
                                       const double walking_speed,
                                       const double walking_time_limit,
                                       ezgl::application *application){
     
   std::vector<int> walkable_intersections;
   for(int i=0; i < nodes_w.size(); ++i){
        //reset reachingEdge to NO_EDGE
        nodes_w[i].reachingEdge= NO_EDGE;
        //reset bestTime of all nodes_w to 100000000.00
        nodes_w[i].bestTime = 100000000.00;
    }
    //define waveFront with root node with least travel time. 
    std::priority_queue<WaveElem, std::vector<WaveElem>, compareTravelTime> waveFront;
    waveFront.push(WaveElem(&(nodes_w[start_intersection]), NO_EDGE, 0.0)); //source node
    while(!waveFront.empty()){
        WaveElem current_node = waveFront.top();
        //remove node from waveFront
        waveFront.pop();
        
        // visualize
        ezgl::renderer *g = application->get_renderer();
        if (current_node.edgeID != -1){
            g->set_color(ezgl::GREEN);
            draw_street_segments(edges_w[current_node.edgeID].idx_seg, g);
            application->flush_drawing();
            std::chrono::milliseconds duration(50);
            std::this_thread::sleep_for(duration);
        }      
        
        //update the edge_w with shorter travel time
        if(current_node.travelTime <= walking_time_limit && current_node.travelTime < current_node.node->bestTime){             
            current_node.node->bestTime = current_node.travelTime;
            current_node.node->reachingEdge = current_node.edgeID;
            // push this node into return vector
            walkable_intersections.push_back(current_node.node->idx_pnt);
            
           //go through all the outEdges of the current_node
            for(int i=0; i< (current_node.node->outEdges.size()); ++i){
                //get to_node of one outEdge
                // to_node of the current out_edge
                Node* to_node = &(nodes_w[ edges_w[((current_node.node)->outEdges)[i]].to ]);                                       

                //update the travel time 
                double totalTravelTime = 0.0;
                
                //outEdge[i] edge travel time 
                double this_edgeTravelTime = streetSeg_length[edges_w[((current_node.node)->outEdges)[i]].idx_seg] / walking_speed ;
                
                //check if turn
                //compare street id of this_seg with reaching_edge of current_node
                int turn = 0;
                if (current_node.edgeID != NO_EDGE){
                    int this_seg_id = edges_w[current_node.edgeID].idx_seg;
                    int next_seg_id = edges_w[current_node.node->outEdges[i]].idx_seg;
                    //calculate turns by comparing street id with its the next street id. 
                    if(street_segments_street[this_seg_id] != street_segments_street[next_seg_id]){
                            turn = 1;
                    }
                }
                totalTravelTime = (current_node.node->bestTime) + this_edgeTravelTime + (turn*turn_penalty);
                
                //update waveFront 
                waveFront.push(WaveElem(to_node, current_node.node->outEdges[i], totalTravelTime)); 
            }   
        }      
    }
    
    return walkable_intersections;
 }

// it traces back all the street segments that are walked through starting from 
// the intersect_id_end.
// The function returns the indicis of the street segments
 std::vector<StreetSegmentIndex> path_walk_search_result(const IntersectionIndex intersect_id_end){
    std::vector<StreetSegmentIndex> path;
    Node* currentNode = &(nodes_w[intersect_id_end]);
    int pre_edge = currentNode->reachingEdge;
    int pre_seg;
    //transverse the path
    while(pre_edge != NO_EDGE){
        //update pre_seg
        pre_seg = edges_w[pre_edge].idx_seg;
        
        path.insert(path.begin(),pre_seg);
        
        // update current_node
        currentNode =  &(nodes_w[edges_w[pre_edge].from]);
        // update pre_edge
        pre_edge = currentNode->reachingEdge; 
        
    }
    
    return path;       
}

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
                                    ezgl::application *application){
   std::vector<StreetSegmentIndex> empty_path;
   
    //reset all elements 
    for(int i=0; i < nodes.size(); ++i){
        //reset reachingEdge to NO_EDGE
        nodes[i].reachingEdge= NO_EDGE;
        //reset bestTime of all nodes to 100000000.00
        nodes[i].bestTime = 100000000.00;
    }
    //define waveFront with root node with least travel time. 
    std::priority_queue<WaveElem, std::vector<WaveElem>, compareEstimateTravelTime> waveFront; //Nodes to explore next
   
    /************ A***************/
    
 if(max_speed_limit != 0){   
     // push all starting pnts into the wavefront
     LatLon  end_position   = getIntersectionPosition(intersect_id_end);
     for (int i = 0; i < intersect_ids_start.size(); ++i){
        //distance from source to destination 
        LatLon  start_position = getIntersectionPosition(intersect_ids_start[i]);
        std::pair<LatLon, LatLon> points = std::make_pair(start_position, end_position);
        //Returns the distance between two coordinates in meters
        double distance_start_to_end_m = find_distance_between_two_points(points);
        double distance_start_to_end_km = distance_start_to_end_m/1000;

        //estimateTravelTime for source ( travel time + distance/ max speed limit of the city)
        double source_estimateTravelTime = 0.0;
        source_estimateTravelTime = 0.0 + (distance_start_to_end_km / max_speed_limit)*3600;
        /************ A***************/

        waveFront.push(WaveElem(&(nodes[intersect_ids_start[i]]), NO_EDGE, 0.0, source_estimateTravelTime)); //source node
     }
     
    while(!waveFront.empty()){
        WaveElem current_node = waveFront.top();
        //remove node from waveFront
        waveFront.pop();

        // visualize
        ezgl::renderer *g = application->get_renderer();
        if (current_node.edgeID != -1){
            g->set_color(ezgl::ORANGE);
            draw_street_segments(edges[current_node.edgeID].idx_seg, g);
            application->flush_drawing();
            std::chrono::milliseconds duration(50);
            std::this_thread::sleep_for(duration);
        }
        
        //update the edge with shorter travel time
        if(current_node.travelTime < current_node.node->bestTime){
            current_node.node->bestTime = current_node.travelTime;
            current_node.node->reachingEdge = current_node.edgeID;
            
            //reach destination
            if(current_node.node->idx_pnt == intersect_id_end){ 
                std::tuple<std::vector<StreetSegmentIndex>,int,double> result_tuple = path_search_result(intersect_id_end);
                return std::make_pair(std::get<0>(result_tuple), std::get<1>(result_tuple));     
            }

            //go through all the outEdges of the current_node
            for(int i=0; i< (current_node.node->outEdges.size()); ++i){
                //get to_node of one outEdge
                Node* to_node = &(nodes[ edges[((current_node.node)->outEdges)[i]].to ]);                                       

                //update the travel time & estimateTravelTime
                double totalTravelTime = 0.0;
                
                //outEdge[i] edge travel time 
                double this_edgeTravelTime = edges[((current_node.node)->outEdges)[i]].edgeTravelTime;
                
                //check if turn
                //compare street id of this_seg with reaching_edge of current_node
                int turn = 0;
                if (current_node.edgeID != NO_EDGE){
                    int this_seg_id = edges[current_node.edgeID].idx_seg;
                    int next_seg_id = edges[current_node.node->outEdges[i]].idx_seg;
                    //std::cout << "\n\n\n -------------------\n"<<this_seg_id<<"\n"<<current_node.edgeID<<"\n -------------------\n\n\n";
                    //calculate turns by comparing street id with its the next street id. 
                    if(street_segments_street[this_seg_id] != street_segments_street[next_seg_id]){
                            turn = 1;
                    }
                }
                totalTravelTime = (current_node.node->bestTime) + this_edgeTravelTime + (turn*turn_penalty);

                
                /************ A***************/
                //calculate estimateTravelTime for outEdge[i]
                int to_node_id = to_node->idx_pnt;
                
                //distance from to_node to end_node
                LatLon  to_node_position = getIntersectionPosition(to_node_id);
                std::pair<LatLon, LatLon> points_new = std::make_pair(to_node_position, end_position);
                //the distance between two coordinates in meters
                double distance_toNode_to_end_m = find_distance_between_two_points(points_new);
                double distance_toNode_to_end_km = distance_toNode_to_end_m/1000;
                
                //estimateTravelTime for source ( travel time + distance/ max speed limit of the city)
                double toNode_estimateTravelTime = (distance_toNode_to_end_km / max_speed_limit)*3600;
                double total_estimateTravelTime = 0.0;
                total_estimateTravelTime = totalTravelTime + toNode_estimateTravelTime;
                /************ A***************/
                
                //update waveFront 
                waveFront.push(WaveElem(to_node, current_node.node->outEdges[i], totalTravelTime, total_estimateTravelTime)); 
            }   
        }                       
    }
    return std::make_pair(empty_path,-1);
 }
    return std::make_pair(empty_path,-1);
}

// This function is similar to find_path_between_intersections() except it takes
// multiple end points instead of one.
// It returns a multimap of < a double, <a vector, an int, a double>>
// The key double is the travel time of this path  
// The vector in the tuple the shortest path (route) between all the start 
// intersections and intersect_id_end
// The first int in the tuple is the end intersection idx that correlated 
// to the path returned.
// The seconf int in the tuple is the delivery idx that correlated 
// to the path returned
// The bool in the tool is the indicator whether it is pickUp or dropOff
// correlated to the path returned
std::multimap<double, std::tuple<std::vector<StreetSegmentIndex>,int,int,bool,bool>> find_path_between_intersections_multi_ends(
                                                const IntersectionIndex intersect_id_start, 
                                                const std::unordered_multimap<int, std::tuple<int,bool,bool>> intersect_ids_end,
                                                const double turn_penalty){
   std::multimap<double, std::tuple<std::vector<StreetSegmentIndex>,int,int,bool,bool>>  result_paths;
   std::unordered_multimap<int, std::tuple<int,bool,bool>> ends_to_check = intersect_ids_end;
   
    //reset all elements 
    for(int i=0; i < nodes.size(); ++i){
        //reset reachingEdge to NO_EDGE
        nodes[i].reachingEdge= NO_EDGE;
        //reset bestTime of all nodes to 100000000.00
        nodes[i].bestTime = 100000000.00;
    }
    //define waveFront with root node with least travel time. 
    std::priority_queue<WaveElem, std::vector<WaveElem>, compareTravelTime> waveFront;
    // push in source nodes
    waveFront.push(WaveElem(&(nodes[intersect_id_start]), NO_EDGE, 0.0)); 
    //source node
    while(!waveFront.empty()){
        WaveElem current_node = waveFront.top();
        //remove node from waveFront
        waveFront.pop();
        
        //update the edge_w with shorter travel time
        if(!ends_to_check.empty() && current_node.travelTime < current_node.node->bestTime){             
            current_node.node->bestTime = current_node.travelTime;
            current_node.node->reachingEdge = current_node.edgeID;
            
            // reach one destination. 
            if(ends_to_check.find(current_node.node->idx_pnt) != ends_to_check.end()){ 
                // remove this end from ends_to_check because a path is found
                ends_to_check.erase(current_node.node->idx_pnt);
                // traceback this path
                std::tuple<std::vector<StreetSegmentIndex>,int,double> traceback_tuple
                                        = path_search_result(current_node.node->idx_pnt);
                // push this path into result
                auto range = intersect_ids_end.equal_range(current_node.node->idx_pnt);
                for (auto itr = range.first; itr != range.second; ++itr){
                    result_paths.insert(std::make_pair(std::get<2>(traceback_tuple),
                        std::make_tuple(std::get<0>(traceback_tuple), 
                                        current_node.node->idx_pnt, 
                                        std::get<0>(itr->second),
                                        std::get<1>(itr->second),
                                        std::get<2>(itr->second))));
                }
            }
            
            //go through all the outEdges of the current_node
            for(int i=0; i< (current_node.node->outEdges.size()); ++i){
                // get to_node of one outEdge
                // to_node of the current out_edge
                Node* to_node = &(nodes[ edges[((current_node.node)->outEdges)[i]].to ]);                                       

                //update the travel time 
                double totalTravelTime = 0.0;
                
                //outEdge[i] edge travel time 
                double this_edgeTravelTime = edges[((current_node.node)->outEdges)[i]].edgeTravelTime;
                
                //check if turn
                //compare street id of this_seg with reaching_edge of current_node
                int turn = 0;
                if (current_node.edgeID != NO_EDGE){
                    int this_seg_id = edges[current_node.edgeID].idx_seg;
                    int next_seg_id = edges[current_node.node->outEdges[i]].idx_seg;
                    //calculate turns by comparing street id with its the next street id. 
                    if(street_segments_street[this_seg_id] != street_segments_street[next_seg_id]){
                            turn = 1;
                    }
                }
                totalTravelTime = (current_node.node->bestTime) + this_edgeTravelTime + (turn*turn_penalty);
                
                //update waveFront 
                waveFront.push(WaveElem(to_node, current_node.node->outEdges[i], totalTravelTime)); 
            }   
        }      
    }
    
    return result_paths;
 }

// multimap<travel_time, tuple<path, end_inter_idx, end_deliv_idx, pORd, start_inter_idx, delivORdepot>>
std::multimap<double, std::tuple<std::vector<StreetSegmentIndex>,int,int,bool,int,bool>> find_path_between_intersections_multi_starts_ends(
                                                const std::vector<int> intersect_ids_start, 
                                                const std::unordered_multimap<int, std::tuple<int,bool,bool>> intersect_ids_end,
                                                const double turn_penalty){
   std::multimap<double, std::tuple<std::vector<StreetSegmentIndex>,int,int,bool,int,bool>> result_paths;
   std::unordered_multimap<int, std::tuple<int,bool,bool>> ends_to_check = intersect_ids_end;
   
    //reset all elements 
    for(int i=0; i < nodes.size(); ++i){
        //reset reachingEdge to NO_EDGE
        nodes[i].reachingEdge= NO_EDGE;
        //reset bestTime of all nodes to 100000000.00
        nodes[i].bestTime = 100000000.00;
    }
    //define waveFront with root node with least travel time. 
    std::priority_queue<WaveElem, std::vector<WaveElem>, compareTravelTime> waveFront;
    // push in source nodes
    for (int i = 0; i < intersect_ids_start.size(); ++i){
        waveFront.push(WaveElem(&(nodes[intersect_ids_start[i]]), NO_EDGE, 0.0));  
    }
    //source node
    while(!waveFront.empty()){
        WaveElem current_node = waveFront.top();
        //remove node from waveFront
        waveFront.pop();
        
        //update the edge_w with shorter travel time
        if(!ends_to_check.empty() && current_node.travelTime < current_node.node->bestTime){             
            current_node.node->bestTime = current_node.travelTime;
            current_node.node->reachingEdge = current_node.edgeID;
            
            // reach one destination intersection. 
            if(ends_to_check.find(current_node.node->idx_pnt) != ends_to_check.end()){ 
                // remove all ends with this inter_id from ends_to_check because a path is found
                ends_to_check.erase(current_node.node->idx_pnt);
                // traceback this path
                std::tuple<std::vector<StreetSegmentIndex>,int,double> traceback_tuple
                                        = path_search_result(current_node.node->idx_pnt);
                // push this path into result
                auto range = intersect_ids_end.equal_range(current_node.node->idx_pnt);
                for (auto itr = range.first; itr != range.second; ++itr){
                    result_paths.insert(std::make_pair(std::get<2>(traceback_tuple),
                        std::make_tuple(std::get<0>(traceback_tuple), 
                                        current_node.node->idx_pnt, 
                                        std::get<0>(itr->second),
                                        std::get<1>(itr->second),
                                        std::get<1>(traceback_tuple),
                                        std::get<2>(itr->second))));
                }
            }
            
            //go through all the outEdges of the current_node
            for(int i=0; i< (current_node.node->outEdges.size()); ++i){
                // get to_node of one outEdge
                // to_node of the current out_edge
                Node* to_node = &(nodes[ edges[((current_node.node)->outEdges)[i]].to ]);                                       

                //update the travel time 
                double totalTravelTime = 0.0;
                
                //outEdge[i] edge travel time 
                double this_edgeTravelTime = edges[((current_node.node)->outEdges)[i]].edgeTravelTime;
                
                //check if turn
                //compare street id of this_seg with reaching_edge of current_node
                int turn = 0;
                if (current_node.edgeID != NO_EDGE){
                    int this_seg_id = edges[current_node.edgeID].idx_seg;
                    int next_seg_id = edges[current_node.node->outEdges[i]].idx_seg;
                    //calculate turns by comparing street id with its the next street id. 
                    if(street_segments_street[this_seg_id] != street_segments_street[next_seg_id]){
                            turn = 1;
                    }
                }
                totalTravelTime = (current_node.node->bestTime) + this_edgeTravelTime + (turn*turn_penalty);
                
                //update waveFront 
                waveFront.push(WaveElem(to_node, current_node.node->outEdges[i], totalTravelTime)); 
            }   
        }      
    }
    
    return result_paths;
 }