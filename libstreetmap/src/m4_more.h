/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   m4_more.h
 * Author: wangbox2
 *
 * Created on March 27, 2020, 11:40 PM
 */

#pragma once
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <time.h>
#include <stdlib.h>
#include "structs_and_classes.h"
#include "m1.h"
#include "m2.h"
#include "m3.h"
#include "m4.h"
#include "m3_more.h"

std::vector<CourierSubpath> traveling_courier_a(
		            const std::vector<DeliveryInfo>& deliveries,
                            const std::vector<int>& depots, 
		            const float turn_penalty, 
		            const float truck_capacity);

std::vector<CourierSubpath> traveling_courier_b(
		            const std::vector<DeliveryInfo>& deliveries,
                            const std::vector<int>& depots, 
		            const float turn_penalty, 
		            const float truck_capacity);

std::vector<CourierSubpath> traveling_courier_c(
		            const std::vector<DeliveryInfo>& deliveries,
                            const std::vector<int>& depots, 
		            const float turn_penalty, 
		            const float truck_capacity);

std::vector<Pick_Drop>perturb_2opt(const std::vector<Pick_Drop>& initial_solution, 
                                int idx1, int idx2);

bool check_legality_for_simplified_paths(const std::vector<Pick_Drop>& solution, 
                                         const std::vector<DeliveryInfo>& deliveries,
                                         const float truck_capacity);

double calculate_time_for_paths(std::vector<CourierSubpath> final_path, const float turn_penalty);
