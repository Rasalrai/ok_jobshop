#include <ctime>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <climits>
#include "solution.h"

#define MU_MN machines_usage[machine_no]
#define V_INT std::vector<int>
#define V_INT64 std::vector<int64_t>
#define V_V_INT std::vector< std::vector<int> >
#define V_V_INT64 std::vector< std::vector<int64_t> >
const double K = 0.5;


inline double get_temp(time_t& start, int& limit)
{
	// used for temperature
	return (1000.0 * (start + limit - time(nullptr)) / limit);
}

int64_t fit_jobs(int& machines_c, int& jobs_c, V_V_INT& proc_order, V_V_INT& proc_times, V_V_INT64& start_times, V_V_INT64& machines_usage, V_INT& job_order, int& max_tasks)
{
	/* schedule time for jobs according to job_order
	this can also continue an already started combinations*/
	for (int i = 0; i < jobs_c; ++i)
		exec_job(job_order[i], machines_c, proc_order[job_order[i]], proc_times[job_order[i]], start_times[job_order[i]], machines_usage, max_tasks);

	// time of execution is max of last items of machines_usage's items
	int64_t max_time = machines_usage[0].back();
	for (int i = 1; i < machines_c; ++i)
		if (machines_usage[i].back() > max_time) max_time = machines_usage[i].back();

	return max_time;
}

void exec_job(int& job_no, int& machines_c, V_INT& proc_order, V_INT& proc_times, V_INT64& start_times, V_V_INT64& machines_usage, int& max_tasks)
{
	/* schedule execution of each of job's tasks in first available time window */
	int machine_no, task_dur;
	int64_t last_ended = 0;
	bool scheduled;
	for (int i = 0; i < max_tasks; ++i)
	{
		machine_no = proc_order[i];
		task_dur = proc_times[i];
		if (!task_dur)			// task duration is 0
			start_times[i] = last_ended;
		else
		{
			scheduled = false;
			// search for an available window for this
			// you can put it either right after previous task done by this machine, or some time after
			for (unsigned int j = 1; j < MU_MN.size(); j += 2)
			{
				if (task_dur <= (MU_MN[j] - MU_MN[j - 1]) && task_dur <= (MU_MN[j] - last_ended))
					//fits into window and is after previous task
				{
					if (last_ended <= MU_MN[j - 1])
					{
						// execute this task as soon as the machine is free
						start_times[i] = MU_MN[j - 1];
						MU_MN[j - 1] += task_dur;
						scheduled = true;
						break;
					}
					else
					{
						// execute later -> need to insert into vector
						start_times[i] = last_ended;
						if (task_dur == (MU_MN[j] - last_ended))
						{
							MU_MN[j] = last_ended;
                            scheduled = true;
                            break;
						}
						else
						{
							MU_MN.insert(MU_MN.begin() + j, 2, last_ended);
							MU_MN[j + 1] += task_dur;
							scheduled = true;
							break;
						}
					}
				}
			}
			if (!scheduled)
			{
				// start the task when possible - machine available or previous finished, whatever comes later
				if (MU_MN.back() >= last_ended)
				{
					start_times[i] = MU_MN.back();
					//increase last element by task_dur
					*(MU_MN.end() - 1) += task_dur;
				}
				else
				{
					start_times[i] = last_ended;
					MU_MN.push_back(last_ended);
					MU_MN.push_back(last_ended + task_dur);
				}
			}
		}
		last_ended = start_times[i] + task_dur;
	}
}

V_V_INT64 better_job_shop(int machines_c, int jobs_c, V_V_INT& proc_order, V_V_INT& proc_times, time_t start_stamp, int time_limit, int64_t& best_time, int max_tasks)
{
    // init all the stuff
    // incl. best one, previous, current
    V_V_INT64 machines_usage(machines_c);
	for (int i = 0; i < machines_c; ++i)
	{
		machines_usage[i].reserve(jobs_c * 2);
		machines_usage[i].push_back(0);
	}

	V_V_INT64 start_times(jobs_c);
	V_V_INT64 best_start_times(jobs_c);
	V_INT job_order(jobs_c), prev_order(jobs_c);
	for (int i = 0; i < jobs_c; ++i)
	{
		start_times[i].resize(max_tasks);
		best_start_times[i].resize(max_tasks);
		job_order[i] = i;
	}

	V_V_INT64* p_times = &start_times, *p_best_times = &best_start_times, *p_temp;
	int64_t curr_time, prev_time = INT_MAX;
	double temperature = 100;

    do	// actual stuff
    {
        // check the permutation
        curr_time = fit_jobs(machines_c, jobs_c, proc_order, proc_times, *p_times, machines_usage, job_order, max_tasks);
        for (int j = 0; j < machines_c; ++j)
        {
            machines_usage[j].erase(machines_usage[j].begin() + 1, machines_usage[j].end());
            machines_usage[j][0] = 0;
        }

		// compare with previous
		if (curr_time < prev_time)
		{
			// compare with best
			if (curr_time < best_time)
			{
				best_time = curr_time;

				p_temp = p_best_times;
				p_best_times = p_times;
				p_times = p_temp;
			}
			if (get_temp(start_stamp, time_limit)<=0)
                return *p_best_times;
			// always use the new option
			prev_order = job_order;
		}
		// check chances of staying with previous permutation
		else
		{
			temperature = get_temp(start_stamp, time_limit);
			if (temperature <= 0) return *p_best_times;
			if (probability(prev_time, curr_time, temperature) <= (double(std::rand()-1) / RAND_MAX))
			{
				// stay with previous permutation
                prev_order = job_order;
			}
			else job_order = prev_order;
		}
		
		// get new permutation
		get_neighbour(job_order);
		prev_time = curr_time;
    }
	while (true);
}

void get_neighbour(V_INT &perm)
{
	// choose two different indices
	int i1 = rand() % (perm.size()), i2;
	do
	{
		i2 = rand() % (perm.size());
	} while (i1 == i2);

	// swap contents of chosen indices
	perm[i1] ^= perm[i2];
	perm[i2] ^= perm[i1];
	perm[i1] ^= perm[i2];
}

inline double probability(int prev, int curr, double temp) { return exp(double(prev - curr) / (K*temp)); }
