#ifndef __SDBSC_H__
#define __SDBSC_H__

#include "db.h" // Include the student database structure

// Function prototypes for student database operations
int open_db(char *dbFile, bool should_truncate);
int add_student(int fd, int id, char *fname, char *lname, int gpa);
int get_student(int fd, int id, student_t *s);
int del_student(int fd, int id);
int compress_db(int fd);
void print_student(student_t *s);
int validate_range(int id, int gpa);
int count_db_records(int fd);
int print_db(int fd);
void usage(char *);

// Error codes returned by database operations
#define NO_ERROR        0
#define ERR_DB_FILE     -1
#define ERR_DB_OP       -2
#define SRCH_NOT_FOUND  -3
#define NOT_IMPLEMENTED_YET 0

// Exit codes for the shell
#define EXIT_OK         0
#define EXIT_FAIL_DB    1
#define EXIT_FAIL_ARGS  2
#define EXIT_NOT_IMPL   3

// Output messages (to match expected testing output)
#define M_ERR_STD_RNG     "Can't add student, either ID or GPA out of allowable range!\n"
#define M_ERR_DB_CREATE   "Error creating DB file, exiting!\n"
#define M_ERR_DB_OPEN     "Error opening DB file, exiting!\n"
#define M_ERR_DB_READ     "Error reading DB file, exiting!\n"
#define M_ERR_DB_WRITE    "Error writing DB file, exiting!\n"
#define M_ERR_DB_ADD_DUP  "Can't add student with ID=%d, already exists in db.\n"
#define M_ERR_STD_PRINT   "Can't print student. Student is NULL or ID is zero\n"

#define M_STD_ADDED       "Student %d added to database.\n"
#define M_STD_DEL_MSG     "Student %d was deleted from database.\n"
#define M_STD_NOT_FND_MSG "Student %d was not found in database.\n"
#define M_DB_COMPRESSED_OK "Database successfully compressed!\n"
#define M_DB_ZERO_OK      "All database records removed!\n"
#define M_DB_EMPTY        "Database contains no student records.\n"
#define M_DB_RECORD_CNT   "Database contains %d student record(s).\n"
#define M_NOT_IMPL        "The requested operation is not implemented yet!\n"

// Useful format strings for printing students
// Example: Print header
//   printf(STUDENT_PRINT_HDR_STRING, "ID","FIRST NAME","LAST_NAME", "GPA");
#define STUDENT_PRINT_HDR_STRING   "%-6s %-24s %-32s %-3s\n"
#define STUDENT_PRINT_FMT_STRING   "%-6d %-24.24s %-32.32s %-3.2f\n"

#endif // __SDBSC_H__
