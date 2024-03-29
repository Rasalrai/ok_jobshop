#pragma once
#include <vector>

#define V_INT std::vector<int>
#define V_INT64 std::vector<int64_t>
#define V_V_INT std::vector< std::vector<int> >
#define V_V_INT64 std::vector< std::vector<int64_t> >

inline double get_temp(time_t& start, int& limit);
int64_t fit_jobs(int& machines_c, int& jobs_c, V_V_INT& proc_order, V_V_INT& proc_times, V_V_INT64& start_times, V_V_INT64& machines_usage, V_INT& job_order, int& max_tasks);
void exec_job(int& job_no, int& machines_c, V_INT& proc_order, V_INT& proc_times, V_INT64& start_times, V_V_INT64& machines_usage, int& max_tasks);
V_V_INT64 better_job_shop(int machines_c, int jobs_c, V_V_INT& proc_order, V_V_INT& proc_times, time_t start_stamp, int time_limit, int64_t& best_time, int max_tasks);
void get_neighbour(V_INT &perm);
inline double probability(int prev, int curr, double temp);
