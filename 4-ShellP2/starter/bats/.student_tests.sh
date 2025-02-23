#!/usr/bin/env bats

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF
ls
EOF
    [ "$status" -eq 0 ]
}

@test "Change directory" {
    current=$(pwd)
    cd /tmp
    mkdir -p dsh-test

    run "${current}/dsh" <<EOF
cd dsh-test
pwd
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="/tmp/dsh-testdsh2>dsh2>dsh2>cmdloopreturned0"

    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Change directory - no args" {
    current=$(pwd)
    cd /tmp
    mkdir -p dsh-test

    run "${current}/dsh" <<EOF
cd
pwd
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="/tmpdsh2>dsh2>dsh2>cmdloopreturned0"

    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Which which ... which?" {
    run "./dsh" <<EOF
which which
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="/usr/bin/whichdsh2>dsh2>cmdloopreturned0"

    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    [ "$stripped_output" = "$expected_output" ]
}

@test "It handles quoted spaces" {
    run "./dsh" <<EOF
echo " hello     world     "
EOF

    stripped_output=$(echo "$output" | tr -d '\t\n\r\f\v')
    expected_output=" hello     world     dsh2>dsh2>cmdloopreturned0"

    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    [ "$stripped_output" = "$expected_output" ]
}

@test "It handles multiple commands" {
    run "./dsh" <<EOF
ls
pwd
EOF

    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"

    [ "$status" -eq 0 ]
}

@test "It handles invalid commands" {
    run "./dsh" <<EOF
invalid_command
EOF

    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"

    [ "$status" -ne 0 ]
}

@test "It handles the rc command" {
    run "./dsh" <<EOF
invalid_command
rc
EOF

    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"

    [ "$output" = "Error executing command: No such file or directory\n127\n" ]
}