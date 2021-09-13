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
In this milestone, we implemented some very basic functions used to interpret the map data(e.g. finding the distance between points, returning the street segments connected to an intersection, etc.). We consider the street map as a graph, where street segments are edges, and intersections are nodes. 

![overall_structure](https://github.com/nzcsx/ece297_map_project/blob/master/README_images/map_as_graph.png)

The graph is directed. The directions of the edges are allowed traveling directions of the street segments. If a segment is two-way, we simply use two edges to represent it. 

![overall_structure](https://github.com/nzcsx/ece297_map_project/blob/master/README_images/directed_graph.png)

```load_map``` and ```close_map``` are two most essential functions of this part. 

```bool load_map(std::string map_name);```

This function acts as an overall constructor function. The function first interprets the {map} name, then loads a {map}.streets.bin file, and saves all the adjacency lists and other needed data in various data stuctures(std::vector, std::map, etc.) This function must be called before any other function in this API can be used. Returns true if the load succeeded, false if it failed.

```void close_map();```

This function acts as an overall destructor function. The function first clear and delete all data stuctures. The function then unloads a map and frees the memory used by the API. No other api calls can be made until the load function is called again for some map. You can only have one map open at a time.

### Milestone 2

In this milestone, we implemented graphic interface related functions. The user interface window is drawn using Glade. The functionalities of the widgets on the interface are implemented using GTK toolkit. 

[insert glade screenshot]

We first visualized the map by ploting all the street segments, intersections, lakes and greenspace. We set street segments as white lines, intersections as white dots, lakes and greenspace as blue and green polygons respectively. To optimize the display, the function ```draw_features``` draws more detail when the map is more zoomed-in. 

We then programmed some interactive widgets (e.g. click on the segments/intersections to reveal names). The program moniters trigger events. When a trigger event happens,  corresponding callback functions are called. We were provided an EZGL library to help us on the development.

### Milestone 3

In this milestone, we implemented wayfindinf algorithms. A star and Dijkstra. 

    std::vector<StreetSegmentIndex> find_path_between_intersections(
		const IntersectionIndex intersect_id_start, 
        const IntersectionIndex intersect_id_end,
        const double turn_penalty);

In this function, we used 