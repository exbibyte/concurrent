#define CATCH_CONFIG_MAIN

#include <vector>

#include "alloc_sthread_first_fit_part_none.hpp"
#include "alloc_test_template_sthread.hpp"

#include "catch.hpp"

TEST_CASE( "allocator single thread first fit partition none allocating", "[first fit]" ) {
    test_allocating_freeing< alloc_sthread_first_fit_part_none >();
}
    
TEST_CASE( "allocator single thread first fit partition none newing", "[first fit]" ) {
    test_newing_deleting< alloc_sthread_first_fit_part_none >();
}

TEST_CASE( "allocator single thread first fit partition none newing placement", "[first fit]" ) {
    test_newing_deleting_placement< alloc_sthread_first_fit_part_none >();
}

TEST_CASE( "allocator single thread first fit partition none invalid delete", "[first fit]" ) {
    test_invalid_deleting< alloc_sthread_first_fit_part_none >();
}

TEST_CASE( "allocator single thread first fit partition none out of memory newing", "[first fit]" ) {
    test_newing_out_of_memory< alloc_sthread_first_fit_part_none >();
}
