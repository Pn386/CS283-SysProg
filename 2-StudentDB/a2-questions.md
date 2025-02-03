## Assignment 2 Questions

#### Directions
Please answer the following questions and submit in your repo for the second assignment.  Please keep the answers as short and concise as possible.

1. In this assignment I asked you provide an implementation for the `get_student(...)` function because I think it improves the overall design of the database application.   After you implemented your solution do you agree that externalizing `get_student(...)` into it's own function is a good design strategy?  Briefly describe why or why not.

    > **Answer**:  Yes, externalizing get_student(...) into its own function is a good design strategy. It promotes modularity and reusability, making the code easier to maintain and test. By separating the logic for retrieving a student record into a dedicated function, the code becomes more organized, and the function can be reused in other parts of the application without duplicating code. Additionally, it adheres to the Single Responsibility Principle, where each function has a clear and specific purpose.



2. Another interesting aspect of the `get_student(...)` function is how its function prototype requires the caller to provide the storage for the `student_t` structure:

    ```c
    int get_student(int fd, int id, student_t *s);
    ```

    Notice that the last parameter is a pointer to storage **provided by the caller** to be used by this function to populate information about the desired student that is queried from the database file. This is a common convention (called pass-by-reference) in the `C` programming language. 

    In other programming languages an approach like the one shown below would be more idiomatic for creating a function like `get_student()` (specifically the storage is provided by the `get_student(...)` function itself):

    ```c
    //Lookup student from the database
    // IF FOUND: return pointer to student data
    // IF NOT FOUND: return NULL
    student_t *get_student(int fd, int id){
        student_t student;
        bool student_found = false;
        
        //code that looks for the student and if
        //found populates the student structure
        //The found_student variable will be set
        //to true if the student is in the database
        //or false otherwise.

        if (student_found)
            return &student;
        else
            return NULL;
    }
    ```
    Can you think of any reason why the above implementation would be a **very bad idea** using the C programming language?  Specifically, address why the above code introduces a subtle bug that could be hard to identify at runtime? 

    > **ANSWER:** The implementation shown is problematic because it returns a pointer to a local variable (student), which is allocated on the stack. Once the function returns, the stack frame for get_student(...) is destroyed, and the pointer to student becomes invalid. Accessing this pointer after the function returns would lead to undefined behavior, as the memory it points to may be overwritten by other function calls. This is a subtle bug that could cause crashes or unpredictable behavior at runtime, making it difficult to debug.



3. Another way the `get_student(...)` function could be implemented is as follows:

    ```c
    //Lookup student from the database
    // IF FOUND: return pointer to student data
    // IF NOT FOUND or memory allocation error: return NULL
    student_t *get_student(int fd, int id){
        student_t *pstudent;
        bool student_found = false;

        pstudent = malloc(sizeof(student_t));
        if (pstudent == NULL)
            return NULL;
        
        //code that looks for the student and if
        //found populates the student structure
        //The found_student variable will be set
        //to true if the student is in the database
        //or false otherwise.

        if (student_found){
            return pstudent;
        }
        else {
            free(pstudent);
            return NULL;
        }
    }
    ```
    In this implementation the storage for the student record is allocated on the heap using `malloc()` and passed back to the caller when the function returns. What do you think about this alternative implementation of `get_student(...)`?  Address in your answer why it work work, but also think about any potential problems it could cause.  
    
    > **ANSWER:** This implementation works because the storage for the student record is allocated on the heap, which persists after the function returns. However, it introduces potential issues:Memory Management Responsibility: The caller must remember to free the allocated memory to avoid memory leaks. If the caller forgets to call free(), the program will leak memory.Error Handling: If malloc() fails (e.g., due to insufficient memory), the function returns NULL, which the caller must handle appropriately.
Performance Overhead: Frequent allocation and deallocation of small memory blocks can lead to fragmentation and performance degradation.
While this approach is valid, it shifts the burden of memory management to the caller, which can be error-prone.




4. Lets take a look at how storage is managed for our simple database. Recall that all student records are stored on disk using the layout of the `student_t` structure (which has a size of 64 bytes).  Lets start with a fresh database by deleting the `student.db` file using the command `rm ./student.db`.  Now that we have an empty database lets add a few students and see what is happening under the covers.  Consider the following sequence of commands:

    ```bash
    > ./sdbsc -a 1 john doe 345
    > ls -l ./student.db
        -rw-r----- 1 bsm23 bsm23 128 Jan 17 10:01 ./student.db
    > du -h ./student.db
        4.0K    ./student.db
    > ./sdbsc -a 3 jane doe 390
    > ls -l ./student.db
        -rw-r----- 1 bsm23 bsm23 256 Jan 17 10:02 ./student.db
    > du -h ./student.db
        4.0K    ./student.db
    > ./sdbsc -a 63 jim doe 285 
    > du -h ./student.db
        4.0K    ./student.db
    > ./sdbsc -a 64 janet doe 310
    > du -h ./student.db
        8.0K    ./student.db
    > ls -l ./student.db
        -rw-r----- 1 bsm23 bsm23 4160 Jan 17 10:03 ./student.db
    ```

    For this question I am asking you to perform some online research to investigate why there is a difference between the size of the file reported by the `ls` command and the actual storage used on the disk reported by the `du` command.  Understanding why this happens by design is important since all good systems programmers need to understand things like how linux creates sparse files, and how linux physically stores data on disk using fixed block sizes.  Some good google searches to get you started: _"lseek syscall holes and sparse files"_, and _"linux file system blocks"_.  After you do some research please answer the following:

    - Please explain why the file size reported by the `ls` command was 128 bytes after adding student with ID=1, 256 after adding student with ID=3, and 4160 after adding the student with ID=64? 

        > **ANSWER:** The file size reported by ls corresponds to the logical size of the file, which is determined by the file's metadata. When a student is added, the file is extended to accommodate the new record. For example:Adding student ID=1 writes 64 bytes, so the file size becomes 128 bytes (likely due to alignment or metadata).Adding student ID=3 writes another 64 bytes, increasing the file size to 256 bytes.Adding student ID=64 writes a record at an offset of 64 * 64 = 4096 bytes, extending the file size to 4160 bytes.



    -   Why did the total storage used on the disk remain unchanged when we added the student with ID=1, ID=3, and ID=63, but increased from 4K to 8K when we added the student with ID=64? 

        > **ANSWER:** The du command reports the actual disk space used, which is allocated in fixed-size blocks (typically 4KB). When the file size is small (e.g., 128 or 256 bytes), it still occupies one 4KB block on disk. Adding students with IDs 1, 3, and 63 does not exceed the first 4KB block, so the disk usage remains unchanged. However, adding student ID=64 requires a new block, increasing the disk usage from 4KB to 8KB.

    - Now lets add one more student with a large student ID number  and see what happens:

        ```bash
        > ./sdbsc -a 99999 big dude 205 
        > ls -l ./student.db
        -rw-r----- 1 bsm23 bsm23 6400000 Jan 17 10:28 ./student.db
        > du -h ./student.db
        12K     ./student.db
        ```
        We see from above adding a student with a very large student ID (ID=99999) increased the file size to 6400000 as shown by `ls` but the raw storage only increased to 12K as reported by `du`.  Can provide some insight into why this happened?

        > **ANSWER:**  When adding a student with ID=99999, the file is extended to an offset of 99999 * 64 = 6,399,936 bytes, resulting in a logical file size of 6,400,000 bytes (rounded up). However, the file is sparse, meaning that only the blocks containing actual data are allocated on disk. Since only a few blocks are written (e.g., for the metadata and the new record), the actual disk usage increases minimally, from 8KB to 12KB. Sparse files allow efficient storage of large files with holes (unallocated regions), saving disk space.
