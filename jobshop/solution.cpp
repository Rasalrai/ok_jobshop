#include <ctime>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cstdint>
#include <random>
#include <pair>

#include "solution.h"

#define MU_MN machines_usage[machine_no]
#define V_INT std::vector<int>
#define V_INT64 std::vector<int64_t>
#define V_V_INT std::vector< std::vector<int> >
#define V_V_INT64 std::vector< std::vector<int64_t> >
#define Q_INT std::queue<int>

bool time_passed(time_t start, int limit)
/* true if _limit_ minutes passed since beginning of execution of the program */
{
	if (time(NULL) >= start + 60 * limit)
		return true;
	return false;
}

V_V_INT64 job_shop(int machines_c, int jobs_c, V_V_INT& proc_order, V_V_INT& proc_times, time_t start_stamp, int time_limit)
{
/*
TODO remember to add time check

if(time_passed(start_stamp, time_limit))
{
    std::cout << best_time << "\n\n";
    return *p_best_times;
}
*/
	// storing iterations' data
	V_V_INT64 machines_usage(machines_c);
	V_V_INT priority(machines_c);
	for (int i = 0; i < machines_c; ++i)
	{
		machines_usage[i].reserve(jobs_c * 2);
		machines_usage[i].push_back(0);
		priority[i].reserve(jobs_c);
	}

	V_V_INT64 start_times(jobs_c);
	V_V_INT64 best_start_times(jobs_c);
	V_INT job_order(jobs_c);
	for (int i = 0; i < jobs_c; ++i)
	{
		start_times[i].resize(machines_c);
		best_start_times[i].resize(machines_c);
		job_order[i] = i;
	}
	std::vector<bool> jobs_scheduled(jobs_c, false);

	V_V_INT64* p_times = &start_times, *p_best_times = &best_start_times, *p_temp;
	int64_t best_time = INT64_MAX, curr_time;
    std::vector< std::pair<int, int64_t> > quality;

	// actual stuff starts here
	for (int position = 0; position < machines_c; ++position)
	{
	    if (jobs_scheduled[i]) continue;
	    jobs_scheduled[i] = true;
	    job_order[i]=i;     // TODO
	    for(int i=0;i<jobs_c;++i)
	    {
	        if(!jobs_scheduled[i])
	        {
	            jobs_scheduled[i] = true;
	            curr_time = (machines_c, jobs_c, proc_order, proc_times, i, position, job_order, jobs_scheduled, start_times, machines_usage);

                if (curr_time < best_time)
                {
                    best_time = curr_time;

                    p_temp = p_best_times;
                    p_best_times = p_times;
                    p_times = p_temp;
                    //std::cout << "\t--" << curr_time << "--\n\n";
                }
                // save curr_time
                quality.append(i, curr_time);
                jobs_scheduled[i] = false;

                // check time
                if(time_passed(start_stamp, time_limit))
                {
                    std::cout << best_time << "\n\n";
                    return *p_best_times;
                }
	        }

	    }
	    // order by quality -> save priorities of the position
        get_priorities(priority[position], quality);

        // check time
        if(time_passed(start_stamp, time_limit))
        {
            std::cout << best_time << "\n\n";
            return *p_best_times;
        }
        // move to next position
	}
	std::cout << best_time << "\n\n";
	return *p_best_times;
}

int64_t fit_jobs(int machines_c, int jobs_c, V_V_INT& proc_order, V_V_INT& proc_times, V_V_INT64& start_times, V_V_INT64 machines_usage, V_INT job_order)
{
	/* schedule time for jobs according to job_order */
	// this can also continue an already started combinations
	for (int i = 0; i < jobs_c; ++i)
		exec_job(job_order[i], machines_c, proc_order[job_order[i]], proc_times[job_order[i]], start_times[job_order[i]], machines_usage);

	/*for(int i=0;i<start_times.size();++i)
	{
	    for(int j=0;j<start_times[0].size();++j) std::cout << start_times[i][j] << '\t';
	    std::cout << "\n";
	}
	std::cout << "\n\n";*/

	// time of execution is max of last items of machines_usage's items
	int64_t max_time = machines_usage[0].back();
	for (int i = 1; i < machines_c; ++i)
		if (machines_usage[i].back() > max_time) max_time = machines_usage[i].back();

	return max_time;
}

void exec_job(int job_no, int machines_c, V_INT& proc_order, V_INT& proc_times, V_INT64& start_times, V_V_INT64& machines_usage)
/* schedule execution of each of job's tasks in first available time window */
{
	int machine_no, task_dur;
	int64_t last_ended = 0;
	bool scheduled;
	for (int i = 0; i < machines_c; ++i)
	{
		machine_no = proc_order[i];
		task_dur = proc_times[i];
		if (!task_dur)                                       // task duration is 0
			start_times[i] = last_ended;

		else if (MU_MN.size() == 1)     // if no jobs assigned yet: allocate as soon as the previous task has ended
		{
			if (last_ended)
			{
				start_times[i] = last_ended;
				MU_MN.push_back(last_ended);
				MU_MN.push_back(last_ended + task_dur);
			}
			else
			{
				MU_MN[0] = task_dur;
				start_times[i] = 0;
			}
		}
		else
		{
			scheduled = 0;
			// search for an available window for this
			// you can put it either right after previous task done by this machine, or some time after
			// TODO binary search for the first available window
			for (int j = MU_MN[0] + 1; j < MU_MN.size(); j += 2)
			{
				if (task_dur <= (MU_MN[j] - MU_MN[j-1]) && task_dur <= (MU_MN[j] - last_ended))
					//fits into window and is after previous task
				{
					if (last_ended <= MU_MN[j-1])
					{
						// execute this task as soon as the machine is free
						start_times[i] = MU_MN[j-1];
						MU_MN[j-1] += task_dur;
						scheduled = 1;
						break;
					}
					else
					{
						// execute later -> need to insert into vector
						start_times[i] = last_ended;
						if (task_dur == (MU_MN[j] - last_ended))
						{
							MU_MN[j] = last_ended;
						}
						else
						{
							MU_MN.insert(MU_MN.begin()+j, 2, last_ended);
							MU_MN[j+1] += task_dur;
							scheduled = 1;
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
					// machines_usage[machine_no][machines_usage[machine_no].size()] = machines_usage[machine_no].back() + task_dur;
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

//void job_ordering(V_INT* order_begin, V_INT* order_end, int machines_c, V_V_INT& proc_order, V_V_INT& proc_times, V_V_INT64& start_times, V_V_INT64 machines_usage)
V_INT job_ordering(std::vector<bool> jobs_scheduled)
{
// returns a vector of jobs that will be executed on the top of currently done
    V_INT job_order.reserve(jobs_c);

    for(int j=0;j<jobs_scheduled.size();++j)
        if (!jobs_scheduled[j])
        {
            job_order.append(j);
            jobs_scheduled[j] = true;
            break;
        }
    std::random_shuffle(job_order.begin(), job_order.end());
    return job_order
}

int64_t check_iteration(int machines_c, int jobs_c, V_V_INT& proc_order, V_V_INT& proc_times, int job_no, int position, V_INT job_order, std::vector<bool> jobs_scheduled, V_V_INT64& start_times, V_V_INT64 machines_usage)
{
    job_order[position] = next_job;
    jobs_scheduled[job_no] = true;
    // find rest of the order
    V_INT new_order = job_ordering(jobs_scheduled);
    // add old and new order

    // execute
    return fit_jobs(machines_c, jobs_c-position-1, proc_order, proc_times, start_times, machines_usage, job_order)

    // insert as executed to job_order and jobs_scheduled ???

}

void get_priorities(V_INT& priorities, std::vector< std::pair<int, int64_t> > quality)
{
    // TODO
    // quality: (job_no, exe_time)
    // priorities is empty
}

V_V_INT64 random_job_shop(int machines_c, int jobs_c, V_V_INT& proc_order, V_V_INT& proc_times, time_t start_stamp, int time_limit)
{
	// storing iterations' data
	V_V_INT64 machines_usage(machines_c);
	for (int i = 0; i < machines_c; ++i)
	{
		machines_usage[i].reserve(jobs_c * 2);
		machines_usage[i].push_back(0);
	}

	V_V_INT64 start_times(jobs_c);
	V_V_INT64 best_start_times(jobs_c);
	V_INT job_order(jobs_c);
	for (int i = 0; i < jobs_c; ++i)
	{
		start_times[i].resize(machines_c);
		best_start_times[i].resize(machines_c);
		job_order[i] = i;
	}

	V_V_INT64* p_times = &start_times, *p_best_times = &best_start_times, *p_temp;
	int64_t best_time = INT64_MAX, curr_time;

	for (int i = 0; i < 100; ++i)
	{
		curr_time = fit_jobs(machines_c, jobs_c, proc_order, proc_times, *p_times, machines_usage, job_order);

		// compare with previous best
		if (curr_time < best_time)
		{
			best_time = curr_time;

			p_temp = p_best_times;
			p_best_times = p_times;
			p_times = p_temp;
		}
		std::random_shuffle(job_order.begin(), job_order.end());
	}
	std::cout << best_time << "\n\n";
	return *p_best_times;
}