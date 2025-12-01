// tests_common.h
#ifndef TESTS_COMMON_H
#define TESTS_COMMON_H

// Define the test result strings
#define PASS "PASS"
#define FAIL "FAIL"

#include <stdio.h>
#include <string.h>
#include <math.h>

// Initialize the test counter
extern int count;

// Test function declarations
int assertTrue(int condition);
int assertFalse(int condition);
int assertNull(void* ptr);
int assertNotNull(void* ptr);
int assertEquals(int a, int b);
int assertFloatEquals(float a, float b);
int assertDoubleEquals(double a, double b);
int assertNotEquals(int a, int b);
int assertGreaterThan(int a, int b);
int assertLessThan(int a, int b);
int assertIn(double value, double min, double max);
int assertPointersMatch(void* a, void* b);
int assertStringsMatch(char* a, char* b);
int assertContains(const char* str, const char* substring);

void reset_test_counter();

#endif // TESTS_COMMON_H