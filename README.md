# ECE297 AmazingMap Project
Second year project with Leo Zhang and Helen Cui.

## Abstract
This map can display regional map given raw [OpenStreetMap](http://wiki.openstreetmap.org/wiki/Main_Page) file. User can search points of interests and street names on it. The user can also use the map to find the shortest route to a destination. 

![default_interface](https://github.com/nzcsx/ece297_map_project/blob/master/README_images/default_interface.png)

The functionality and user interface of the map are implemented in the map functions in files under /libstreetmap/src/. Raw OSM files are serialized to program readable bin files. The map functions reads the serialized files through two api's provided. 

![overall_structure](https://github.com/nzcsx/ece297_map_project/blob/master/README_images/overall_strcuture.png)

There are two serialzed files and two api's for them respectively. StreetsDatabaseAPI.h is more processed and contains only streets informaiton. OSMDatabaseAPI.h is less processed but contains more information (such as subways, points of interests, etc.)
