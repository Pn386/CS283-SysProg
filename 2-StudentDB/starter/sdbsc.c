#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include "db.h"
#include "sdbsc.h"

/*
 *  get_student
 *      Retrieves a student record from the database based on ID.
 */
int get_student(int fd, int id, student_t *s)
{
    if (id < MIN_STD_ID || id > MAX_STD_ID)
        return ERR_DB_OP;

    int offset = (id - 1) * sizeof(student_t);
    if (lseek(fd, offset, SEEK_SET) == -1)
        return ERR_DB_FILE;

    if (read(fd, s, sizeof(student_t)) != sizeof(student_t))
        return ERR_DB_FILE;

    if (memcmp(s, &EMPTY_STUDENT_RECORD, sizeof(student_t)) == 0)
        return SRCH_NOT_FOUND;

    return NO_ERROR;
}

/*
 *  add_student
 *      Adds a student record to the database.
 */
int add_student(int fd, int id, char *fname, char *lname, int gpa)
{
    if (id < MIN_STD_ID || id > MAX_STD_ID || gpa < MIN_STD_GPA || gpa > MAX_STD_GPA)
    {
        printf(M_ERR_STD_RNG);
        return ERR_DB_OP;
    }

    student_t s;
    int offset = (id - 1) * sizeof(student_t);
    
    if (lseek(fd, offset, SEEK_SET) == -1)
        return ERR_DB_FILE;

    if (read(fd, &s, sizeof(student_t)) == sizeof(student_t) && memcmp(&s, &EMPTY_STUDENT_RECORD, sizeof(student_t)) != 0)
    {
        printf(M_ERR_DB_ADD_DUP, id);
        return ERR_DB_OP;
    }

    memset(&s, 0, sizeof(student_t));
    s.id = id;
    strncpy(s.fname, fname, sizeof(s.fname) - 1);
    strncpy(s.lname, lname, sizeof(s.lname) - 1);
    s.gpa = gpa;

    if (lseek(fd, offset, SEEK_SET) == -1 || write(fd, &s, sizeof(student_t)) != sizeof(student_t))
        return ERR_DB_FILE;

    printf(M_STD_ADDED, id);
    return NO_ERROR;
}

/*
 *  del_student
 *      Deletes a student record by writing an empty record.
 */
int del_student(int fd, int id)
{
    student_t s;
    int result = get_student(fd, id, &s);
    if (result == SRCH_NOT_FOUND)
    {
        printf(M_STD_NOT_FND_MSG, id);
        return ERR_DB_OP;
    }
    if (result == ERR_DB_FILE)
        return ERR_DB_FILE;

    int offset = (id - 1) * sizeof(student_t);
    if (lseek(fd, offset, SEEK_SET) == -1 || write(fd, &EMPTY_STUDENT_RECORD, sizeof(student_t)) != sizeof(student_t))
        return ERR_DB_FILE;

    printf(M_STD_DEL_MSG, id);
    return NO_ERROR;
}

/*
 *  count_db_records
 *      Counts the number of valid student records in the database.
 */
int count_db_records(int fd)
{
    student_t s;
    int count = 0;
    lseek(fd, 0, SEEK_SET);

    while (read(fd, &s, sizeof(student_t)) == sizeof(student_t))
    {
        if (memcmp(&s, &EMPTY_STUDENT_RECORD, sizeof(student_t)) != 0)
            count++;
    }

    if (count == 0)
        printf(M_DB_EMPTY);
    else
        printf(M_DB_RECORD_CNT, count);

    return count;
}

/*
 *  print_db
 *      Prints all student records in the database.
 */
int print_db(int fd)
{
    student_t s;
    int count = 0;
    lseek(fd, 0, SEEK_SET);

    while (read(fd, &s, sizeof(student_t)) == sizeof(student_t))
    {
        if (memcmp(&s, &EMPTY_STUDENT_RECORD, sizeof(student_t)) != 0)
        {
            if (count == 0)
                printf(STUDENT_PRINT_HDR_STRING, "ID", "FIRST NAME", "LAST NAME", "GPA");

            float gpa = s.gpa / 100.0;
            printf(STUDENT_PRINT_FMT_STRING, s.id, s.fname, s.lname, gpa);
            count++;
        }
    }

    if (count == 0)
        printf(M_DB_EMPTY);

    return NO_ERROR;
}

/*
 *  print_student
 *      Prints a single student record.
 */
void print_student(student_t *s)
{
    if (!s || s->id == 0)
    {
        printf(M_ERR_STD_PRINT);
        return;
    }

    printf(STUDENT_PRINT_HDR_STRING, "ID", "FIRST NAME", "LAST NAME", "GPA");
    float gpa = s->gpa / 100.0;
    printf(STUDENT_PRINT_FMT_STRING, s->id, s->fname, s->lname, gpa);
}

/*
 *  compress_db (Extra Credit)
 *      Creates a new compressed database without empty records.
 */
int compress_db(int fd)
{
    int new_fd = open(TMP_DB_FILE, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (new_fd < 0)
    {
        printf(M_ERR_DB_OPEN);
        return ERR_DB_FILE;
    }

    student_t s;
    lseek(fd, 0, SEEK_SET);

    while (read(fd, &s, sizeof(student_t)) == sizeof(student_t))
    {
        if (memcmp(&s, &EMPTY_STUDENT_RECORD, sizeof(student_t)) != 0)
        {
            if (write(new_fd, &s, sizeof(student_t)) != sizeof(student_t))
            {
                printf(M_ERR_DB_WRITE);
                return ERR_DB_FILE;
            }
        }
    }

    close(fd);
    rename(TMP_DB_FILE, DB_FILE);
    fd = open(DB_FILE, O_RDWR);

    if (fd < 0)
    {
        printf(M_ERR_DB_CREATE);
        return ERR_DB_FILE;
    }

    printf(M_DB_COMPRESSED_OK);
    return fd;
}
