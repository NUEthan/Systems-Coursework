#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <munit.h>

#include "vect.h"

/* vect_get on an mt vector */
MunitResult test_get_on_empty_vector(const MunitParameter params[], void *data) {
  vect_t *v1 = vect_new();
  munit_assert_null(vect_get(v1, 0));
  vect_delete(v1);
  return MUNIT_OK;
}

/* vect_set with null vector */
MunitResult test_set_null_vector(const MunitParameter params[], void *data) {
  vect_t *v1 = NULL;
  vect_set(v1, 0, "test");
  return MUNIT_OK; // We expect this to not segfault and handle the null vector gracefully
}

/* removing an elem from mt vector */
MunitResult test_remove_from_empty_vector(const MunitParameter params[], void *data) {
  vect_t *v1 = vect_new();
  vect_remove_last(v1); // Should not segfault
  vect_delete(v1);
  return MUNIT_OK;
}

/* adding elem after deleting the vector */
MunitResult test_add_after_delete(const MunitParameter params[], void *data) {
  vect_t *v1 = vect_new();
  vect_delete(v1);
  vect_add(v1, "test"); 
  return MUNIT_OK;
}

static void *vector_setup(const MunitParameter params[], void *data) {
  vect_t *v1 = vect_new();

  return v1;
}

static void vector_teardown(void* v) {
  vect_delete(v);
}

/* Test null */
MunitResult test_null_vector(const MunitParameter params[], void *data) {
  vect_t *v = NULL;
  munit_assert_null(v);

  vect_add(v, "test");
  munit_assert_null(vect_get(v, 0));

  return MUNIT_OK;
}

/* Test growth */
MunitResult test_vector_growth(const MunitParameter params[], void *data) {
  vect_t *v = vect_new();

  vect_add(v, "1");
  vect_add(v, "2");
  vect_add(v, "3");

  munit_assert_uint(vect_current_capacity(v), >, 2); // cap should ++
  munit_assert_uint(vect_size(v), ==, 3);

  vect_delete(v);
  return MUNIT_OK;
}

/* Test bounds */
MunitResult test_boundary_check(const MunitParameter params[], void *data) {
  vect_t *v = vect_new();

  vect_add(v, "test");

  munit_assert_null(vect_get(v, 10));

  vect_delete(v);
  return MUNIT_OK;
}


MunitResult test_empty(const MunitParameter params[], void *data) {
  vect_t *v1 = (vect_t *) data;
  munit_assert_not_null(v1);
  munit_assert_uint(vect_current_capacity(v1), ==, 2);
  munit_assert_uint(vect_size(v1), ==, 0);

  return MUNIT_OK;
}

MunitResult test_single(const MunitParameter params[], void *data) {
  vect_t *v1 = data;

  vect_add(v1, "hello");
  munit_assert_uint(vect_current_capacity(v1), ==, 2);
  munit_assert_uint(vect_size(v1), ==, 1);
  munit_assert_string_equal(vect_get(v1, 0), "hello");

  return MUNIT_OK;
}

MunitResult test_second(const MunitParameter params[], void *data) {
  vect_t *v1 = data;

  //munit_log(MUNIT_LOG_INFO, "Adding a second item");
  vect_add(v1, "hello");
  vect_add(v1, "world");
  munit_assert_uint(vect_current_capacity(v1), ==, 2);
  munit_assert_uint(vect_size(v1), ==, 2);
  munit_assert_string_equal(vect_get(v1, 0), "hello");
  munit_assert_string_equal(vect_get(v1, 1), "world");
  //munit_log(MUNIT_LOG_INFO, "OK.");

  return MUNIT_OK;
}


MunitResult test_third(const MunitParameter params[], void *data) {
  vect_t *v1 = data;

  vect_add(v1, "hello");
  vect_add(v1, "world");
  vect_add(v1, "CS3650");
  munit_assert_uint(vect_current_capacity(v1), ==, 4);
  munit_assert_uint(vect_size(v1), ==, 3);
  munit_assert_string_equal(vect_get(v1, 0), "hello");
  munit_assert_string_equal(vect_get(v1, 1), "world");
  munit_assert_string_equal(vect_get(v1, 2), "CS3650");

  return MUNIT_OK;
}

MunitResult test_modify(const MunitParameter params[], void *data) {
  vect_t *v1 = data;

  vect_add(v1, "hello");
  vect_add(v1, "world");
  vect_add(v1, "CS3650");

  vect_set(v1, 1, "Foo");
  munit_assert_uint(vect_current_capacity(v1), ==, 4);
  munit_assert_uint(vect_size(v1), ==, 3);
  munit_assert_string_equal(vect_get(v1, 0), "hello");
  munit_assert_string_equal(vect_get(v1, 1), "Foo");
  munit_assert_string_equal(vect_get(v1, 2), "CS3650");

  return MUNIT_OK;
}

MunitResult test_remove_last(const MunitParameter params[], void *data) {
  vect_t *v1 = data;

  vect_add(v1, "hello");
  vect_add(v1, "world");
  vect_add(v1, "CS3650");
  vect_set(v1, 1, "Foo");

  vect_remove_last(v1);
  munit_assert_uint(vect_current_capacity(v1), ==, 4);
  munit_assert_uint(vect_size(v1), ==, 2);
  munit_assert_string_equal(vect_get(v1, 0), "hello");
  munit_assert_string_equal(vect_get(v1, 1), "Foo");

  return MUNIT_OK;
}

MunitResult test_vect_get(const MunitParameter params[], void *data) {
  vect_t *v1 = data;

  vect_add(v1, "bar");

  const char *tmp1 = vect_get(v1, 0);
  const char *tmp2 = vect_get(v1, 0);
  munit_assert_string_equal(tmp1, tmp2);
  munit_assert_ptr_equal(tmp1, tmp2);

  return MUNIT_OK;
}

MunitResult test_vect_get_copy(const MunitParameter params[], void *data) {
  vect_t *v1 = data;

  vect_add(v1, "hello");
  vect_add(v1, "world");
  vect_add(v1, "CS3650");
  vect_set(v1, 1, "Foo");

  char *copy1 = vect_get_copy(v1, 1);
  char *copy2 = vect_get_copy(v1, 1);
  munit_assert_string_equal(copy1, "Foo");
  munit_assert_string_equal(copy1, copy2);
  munit_assert_ptr_not_equal(copy1, copy2);
  munit_assert_ptr_not_equal(copy1, vect_get(v1, 1));
  munit_assert_ptr_not_equal(copy2, vect_get(v1, 1));

  free(copy1);
  free(copy2);

  return MUNIT_OK;
}

MunitResult test_delete_and_create(const MunitParameter params[], void *data) {
  vect_t *v1 = data;

  vect_delete(v1);

  v1 = vect_new();
  munit_assert_not_null(v1);
  vect_delete(v1);

  return MUNIT_OK;
}

// Adds 'count' items to v, checks them, modifies them, and cleans up
void stress_helper(vect_t *v, unsigned int count) {
  char buf[128];

  munit_logf(MUNIT_LOG_INFO, "Adding %d items", count);
  for (int i = 0; i < count; i++) {
    sprintf(buf, "Item no. %d", i);
    vect_add(v, buf);
  }
  munit_assert_uint(vect_size(v), ==, count);

  munit_logf(MUNIT_LOG_INFO, "Ensuring %d items added correctly", count);
  for (int i = 0; i < count; i++) {
    sprintf(buf, "Item no. %d", i);
    munit_assert_string_equal(vect_get(v, i), buf);
  }

  munit_logf(MUNIT_LOG_INFO, "Modifying all %d items", count);
  for (int i = 0; i < count; i++) {
    sprintf(buf, "%s, %d", vect_get(v, i), i * i);
    vect_set(v, i, buf);
  }
  for (int i = 0; i < count; i++) {
    sprintf(buf, "Item no. %d, %d", i, i * i);
    munit_assert_string_equal(vect_get(v, i), buf);
  }
}

MunitResult test_stress(const MunitParameter params[], void *data) {

  vect_t *v1 = data;
  unsigned int count = strtol(munit_parameters_get(params, "count"), NULL, 10);
  stress_helper(v1, count);

  return MUNIT_OK;
}

#define MUNIT_SIMPLE(name, test_func, params) { \
  name,                   /* name */            \
  test_func,              /* test */            \
  NULL,                   /* setup */           \
  NULL,                   /* tear_down */       \
  MUNIT_TEST_OPTION_NONE, /* options */         \
  params                  /* parameters */      \
}

#define VECTOR_FIXTURE(name, test_func, params) { \
  name,                   /* name */              \
  test_func,              /* test */              \
  vector_setup,           /* setup */             \
  vector_teardown,        /* tear_down */         \
  MUNIT_TEST_OPTION_NONE, /* options */           \
  (void *) params         /* parameters */        \
}

#define MUNIT_TESTS_END MUNIT_SIMPLE(NULL, NULL, NULL) 

static char* small_counts[] = {
  (char*) "1000", (char*) "10000", (char*) "100000", NULL
};

static char* big_counts[] = {
  (char*) "1000000", (char*) "10000000", (char*) "100000000", NULL
};

static MunitParameterEnum small_count_params[] = {
  { (char*) "count", small_counts  },
  { NULL, NULL },
};

static MunitParameterEnum big_count_params[] = {
  { (char*) "count", big_counts },
  { NULL, NULL },
};

MunitResult test_out_of_bounds(const MunitParameter params[], void *data) {
  vect_t *v1 = data;

  vect_add(v1, "test1");
  vect_add(v1, "test2");

  return MUNIT_OK;
}

/* For null  */
MunitResult test_null_element(const MunitParameter params[], void *data) {
  vect_t *v1 = data;

  vect_add(v1, NULL);

  munit_assert_null(vect_get(v1, 0));

  vect_add(v1, "test");

  munit_assert_string_equal(vect_get(v1, 1), "test");

  vect_set(v1, 1, NULL);

  munit_assert_null(vect_get(v1, 1));

  return MUNIT_OK;
}

MunitTest additional_tests[] = {
  VECTOR_FIXTURE("/out_of_bounds", test_out_of_bounds, NULL),
  VECTOR_FIXTURE("/null_element", test_null_element, NULL),
  VECTOR_FIXTURE("/get_on_empty_vector", test_get_on_empty_vector, NULL),
  MUNIT_SIMPLE("/set_null_vector", test_set_null_vector, NULL),
  VECTOR_FIXTURE("/remove_from_empty_vector", test_remove_from_empty_vector, NULL),
  VECTOR_FIXTURE("/add_after_delete", test_add_after_delete, NULL),
  VECTOR_FIXTURE("/out_of_bounds", test_out_of_bounds, NULL),
  VECTOR_FIXTURE("/null_element", test_null_element, NULL),
  MUNIT_TESTS_END
};


MunitTest tests_basic[] = {
  VECTOR_FIXTURE("/empty", test_empty, NULL),
  VECTOR_FIXTURE("/1 item", test_single, NULL),
  VECTOR_FIXTURE("/2 items", test_second, NULL),
  VECTOR_FIXTURE("/3 items", test_third, NULL),
  VECTOR_FIXTURE("/modification", test_modify, NULL),
  VECTOR_FIXTURE("/remove_last", test_remove_last, NULL),
  VECTOR_FIXTURE("/get", test_vect_get, NULL),
  VECTOR_FIXTURE("/get_copy", test_vect_get_copy, NULL),
  { "/delete and create", test_delete_and_create, vector_setup, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  VECTOR_FIXTURE("/out_of_bounds", test_out_of_bounds, NULL),
  VECTOR_FIXTURE("/null_element", test_null_element, NULL),
  VECTOR_FIXTURE("/null_vector", test_null_vector, NULL),
  VECTOR_FIXTURE("/vector_growth", test_vector_growth, NULL),
  VECTOR_FIXTURE("/boundary_check", test_boundary_check, NULL),
  MUNIT_TESTS_END
};

MunitTest tests_small_stress[] = {
  VECTOR_FIXTURE("/small stress test", test_stress, small_count_params),
  MUNIT_TESTS_END
};

MunitTest tests_big_stress[] = {
  VECTOR_FIXTURE("/bigger stress test", test_stress, big_count_params),
  MUNIT_TESTS_END
};

#define DEFINE_SUITE(name, suites) \
  static const MunitSuite name = { \
    "/vect", \
    tests_basic, \
    suites, \
    1, \
    MUNIT_SUITE_OPTION_NONE \
  };

static MunitSuite only_small_tests[] = {
   { "", tests_small_stress, NULL, 1, MUNIT_SUITE_OPTION_NONE }, 
   { NULL, NULL, NULL, 0, MUNIT_SUITE_OPTION_NONE }
};

static MunitSuite small_big_tests[] = {
   { "", tests_small_stress, NULL, 1, MUNIT_SUITE_OPTION_NONE }, 
   { "", tests_big_stress, NULL, 1, MUNIT_SUITE_OPTION_NONE }, 
   { NULL, NULL, NULL, 0, MUNIT_SUITE_OPTION_NONE }
};

DEFINE_SUITE(suite_small, only_small_tests);

DEFINE_SUITE(suite_big, small_big_tests);

int main(int argc, char **argv) {
  if (argc > 1 && strcmp(argv[1], "--bigger") == 0) {
    // passing argc - 1 and argv + 1 is a hacky solution to make --bigger work
    return munit_suite_main(&suite_big, NULL, argc - 1, argv + 1);
  }
  else {
    return munit_suite_main(&suite_small, NULL, argc, argv);    
  }
}


