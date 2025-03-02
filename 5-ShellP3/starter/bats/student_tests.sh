#!/usr/bin/env bats
# File: student_tests.sh
# 
# Custom unit tests for the dsh shell implementation

# Basic command execution test
@test "Basic command execution - ls" {
    run ./dsh <<EOF
ls
EOF
    [ "$status" -eq 0 ]
}

# Test built-in exit command
@test "Built-in exit command" {
    run ./dsh <<EOF
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"exiting"* ]]
}

# Test empty command
@test "Empty command handling" {
    run ./dsh <<EOF

EOF
    [ "$status" -eq 0 ]
}

# Basic pipe test - one pipe
@test "Basic pipe - ls | wc" {
    run ./dsh <<EOF
ls | wc
EOF
    [ "$status" -eq 0 ]
}

# Multiple pipe test - three commands
@test "Multiple pipes - ls | grep sh | wc -l" {
    run ./dsh <<EOF
ls | grep sh | wc -l
EOF
    [ "$status" -eq 0 ]
}

# Test non-existent command
@test "Non-existent command handling" {
    run ./dsh <<EOF
nonexistentcommand
EOF
    [ "$status" -eq 0 ]  # Shell should continue running
}

# Test piping to a non-existent command
@test "Pipe to non-existent command" {
    run ./dsh <<EOF
ls | nonexistentcommand
EOF
    [ "$status" -eq 0 ]  # Shell should continue running
}

# Test command with arguments
@test "Command with arguments - echo hello" {
    run ./dsh <<EOF
echo hello world
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"hello world"* ]]
}

# Test maximum pipe limit
@test "Maximum pipe limit" {
    run ./dsh <<EOF
ls | grep sh | sort | uniq | wc | grep 1 | wc -l | cat | grep 1
EOF
    [ "$status" -eq 0 ]
}

# Test pipe with grep filtering
@test "Pipe with grep filtering" {
    run ./dsh <<EOF
ls | grep dshlib
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"dshlib"* ]]
}

#
# Redirection Tests
#

# Test output redirection
@test "Output redirection - echo to file" {
    # Ensure test file doesn't exist initially
    rm -f test_output.txt
    
    run ./dsh <<EOF
echo "test output" > test_output.txt
cat test_output.txt
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"test output"* ]]
    
    # Clean up
    rm -f test_output.txt
}

# Test input redirection
@test "Input redirection - cat from file" {
    # Create a test file
    echo "test input" > test_input.txt
    
    run ./dsh <<EOF
cat < test_input.txt
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"test input"* ]]
    
    # Clean up
    rm -f test_input.txt
}

# Test append redirection
@test "Append redirection - multiple echo to file" {
    # Ensure test file doesn't exist initially
    rm -f test_append.txt
    
    run ./dsh <<EOF
echo "line 1" > test_append.txt
echo "line 2" >> test_append.txt
cat test_append.txt
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"line 1"* ]]
    [[ "$output" == *"line 2"* ]]
    
    # Clean up
    rm -f test_append.txt
}

# Test combination of pipe and redirection
@test "Pipe and redirection combined" {
    # Ensure test file doesn't exist initially
    rm -f test_pipe_redir.txt
    
    run ./dsh <<EOF
ls | grep sh > test_pipe_redir.txt
cat test_pipe_redir.txt
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"dsh"* ]]
    
    # Clean up
    rm -f test_pipe_redir.txt
}

# Test multi-command pipeline with redirection
@test "Multi-command pipeline with redirection" {
    # Ensure test file doesn't exist initially
    rm -f test_multi.txt
    
    run ./dsh <<EOF
ls | grep .c | sort > test_multi.txt
cat test_multi.txt
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *".c"* ]]
    
    # Clean up
    rm -f test_multi.txt
}