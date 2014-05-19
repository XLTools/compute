//---------------------------------------------------------------------------//
// Copyright (c) 2013-2014 Kyle Lutz <kyle.r.lutz@gmail.com>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// See http://kylelutz.github.com/compute for more information.
//---------------------------------------------------------------------------//

#define BOOST_TEST_MODULE TestWaitList
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <vector>

#include <boost/compute/command_queue.hpp>
#include <boost/compute/system.hpp>
#include <boost/compute/wait_list.hpp>
#include <boost/compute/async/future.hpp>
#include <boost/compute/algorithm/copy.hpp>
#include <boost/compute/container/vector.hpp>

#include "check_macros.hpp"
#include "context_setup.hpp"

namespace compute = boost::compute;

BOOST_AUTO_TEST_CASE(create_wait_list)
{
    compute::wait_list events;
    BOOST_CHECK_EQUAL(events.size(), 0);
    BOOST_CHECK_EQUAL(events.empty(), true);
    BOOST_CHECK(events.get_event_ptr() == 0);
}

#ifndef BOOST_COMPUTE_DETAIL_NO_VARIADIC_TEMPLATES
BOOST_AUTO_TEST_CASE(variadic_insert)
{
    // wait list
    compute::wait_list events;

    // create host data array
    int data[] = { 1, 2, 3, 4, 5, 6, 7, 8 };

    // create vector on the device
    compute::vector<int> vector(8, context);

    // copy each pair of values independently and asynchronously
    compute::event copy1 = queue.enqueue_write_buffer_async(
        vector.get_buffer(), 0 * sizeof(int), 2 * sizeof(int), data + 0
    );
    compute::event copy2 = queue.enqueue_write_buffer_async(
        vector.get_buffer(), 2 * sizeof(int), 2 * sizeof(int), data + 2
    );
    compute::event copy3 = queue.enqueue_write_buffer_async(
        vector.get_buffer(), 4 * sizeof(int), 2 * sizeof(int), data + 4
    );
    compute::event copy4 = queue.enqueue_write_buffer_async(
        vector.get_buffer(), 6 * sizeof(int), 2 * sizeof(int), data + 6
    );

    // add all events to the wait list
    events.insert(copy1, copy2, copy3, copy4);

    // block until all events complete
    events.wait();

    // check
    CHECK_RANGE_EQUAL(int, 8, vector, (1, 2, 3, 4, 5, 6, 7, 8));
}
#endif // BOOST_COMPUTE_DETAIL_NO_VARIADIC_TEMPLATES

BOOST_AUTO_TEST_CASE(insert_future)
{
    // create vector on the host
    std::vector<int> host_vector(4);
    std::fill(host_vector.begin(), host_vector.end(), 7);

    // create vector on the device
    compute::vector<int> device_vector(4, context);

    // create wait list
    compute::wait_list events;

    // copy values to device
    compute::future<void> future = compute::copy_async(
        host_vector.begin(), host_vector.end(), device_vector.begin(), queue
    );

    // add future event to the wait list
    events.insert(future);
    BOOST_CHECK_EQUAL(events.size(), 1);
    BOOST_CHECK(events.get_event_ptr() != 0);

    // wait for copy to complete
    events.wait();

    // check values
    CHECK_RANGE_EQUAL(int, 4, device_vector, (7, 7, 7, 7));

    // clear the event list
    events.clear();
    BOOST_CHECK_EQUAL(events.size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
