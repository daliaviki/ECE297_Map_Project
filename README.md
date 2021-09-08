# ECE297 AmazingMap Project
Second year project with Leo Zhang and Helen Cui.

## Abstract
This map can display regional map given raw [OpenStreetMap](http://wiki.openstreetmap.org/wiki/Main_Page) file. User can search points of interests and street names on it. The user can also use the map to find the shortest route to a destination. 

![default_interface](https://github.com/nzcsx/ece297_map_project/blob/master/README_images/default_interface.png)

## How It's Implemented
### General Strucrture
The functionality and user interface of the map are implemented in the map functions in files under /libstreetmap/src/. Raw OSM files are serialized to program readable bin files. The map functions reads the serialized files through two api's provided. 

![overall_structure](https://github.com/nzcsx/ece297_map_project/blob/master/README_images/overall_strcuture.png)

There are two serialzed files and two api's for them respectively. StreetsDatabaseAPI.h (also called "layer 2 API") is more simplified and contains only streets informaiton. OSMDatabaseAPI.h (also called "layer 1 API") is less processed but contains more information (such as subways, points of interests, etc.) All functionalities are implemented in four milestones sequentially.

### Milestone 1
In this milestone, some very basic helper functions are implemented. Below is a list of all functions implemented in this milestone.

```bool load_map(std::string map_name);```

This function acts as a sub-main function. The function first interprets the {map} name, then loads a {map}.streets.bin file, and saves all the data(streets, intersections, etc.) in data stuctures(std::vector, std::map, etc.) This function must be called before any other function in this API can be used. Returns true if the load succeeded, false if it failed.

```void close_map();```

This function first clear and delete all data stuctures. The function then unloads a map and frees the memory used by the API. No other api calls can be made until the load function is called again for some map. You can only have one map open at a time.

```double find_distance_between_two_points(std::pair<LatLon, LatLon> points);``

```double find_street_segment_length(int street_segment_id);```

```double find_street_segment_travel_time(int street_segment_id);```

```int find_closest_intersection(LatLon my_position);```

```int find_closest_POI(LatLon my_position);```

```std::vector<int> find_street_segments_of_intersection(int intersection_id);```

```std::vector<std::string> find_street_names_of_intersection(int intersection_id);```

```bool are_directly_connected(std::pair<int, int> intersection_ids);```
