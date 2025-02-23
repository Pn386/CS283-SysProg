#!/usr/bin/env bats

# Test basic command execution
@test "Execute simple command" {
    run ./dsh <<EOF
echo hello
EOF
    [ "$status" -eq 0 ]
}

# Test cd command with no arguments
@test "cd command - no arguments" {
    local orig_dir=$(pwd)
    run ./dsh <<EOF
cd
pwd
EOF
    [ "$status" -eq 0 ]
}

# Test cd command with argument
@test "cd command - with argument" {
    mkdir -p test_dir
    run ./dsh <<EOF
cd test_dir
pwd
EOF
    [ "$status" -eq 0 ]
    rmdir test_dir
}

# Test quoted string handling
@test "Handle quoted strings" {
    run ./dsh <<EOF
echo "  hello,    world  "
EOF
    [ "$status" -eq 0 ]
}

# Test multiple space handling
@test "Handle multiple spaces between arguments" {
    run ./dsh <<EOF
echo    test     message
EOF
    [ "$status" -eq 0 ]
}

# Test empty input
@test "Handle empty input" {
    run ./dsh <<EOF

EOF
    [ "$status" -eq 0 ]
}

# Test exit command
@test "Exit command" {
    run ./dsh <<EOF
exit
EOF
    [ "$status" -eq 0 ]
}

# Test command not found
@test "Handle nonexistent command" {
    run ./dsh <<EOF
nonexistentcommand
EOF
    [ "$status" -eq 0 ]
}

# Test multiple commands
@test "Handle multiple sequential commands" {
    run ./dsh <<EOF
pwd
ls
echo test
EOF
    [ "$status" -eq 0 ]
}

# Test empty quoted string
@test "Handle empty quoted string" {
    run ./dsh <<EOF
echo ""
EOF
    [ "$status" -eq 0 ]
}

# Test command with multiple arguments
@test "Handle command with multiple arguments" {
    run ./dsh <<EOF
echo arg1 arg2 arg3
EOF
    [ "$status" -eq 0 ]
}

# Test leading/trailing spaces
@test "Handle leading and trailing spaces" {
    run ./dsh <<EOF
   echo test   
EOF
    [ "$status" -eq 0 ]
}