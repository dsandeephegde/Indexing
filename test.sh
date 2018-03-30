#!/usr/bin/env bash
cc main.c

declare -a methods=("first" "best" "worst")
declare -a inputs=("-simple" "-medium")

for input in "${inputs[@]}"
do
    echo "$input input"
    for method in "${methods[@]}"
    do
        ./a.out --$method-fit student.db < Test/input$input.txt > Test/my-output$input-$method.txt 2> /dev/null
        sdiff -s Test/output$input-$method.txt Test/my-output$input-$method.txt && echo "$method-fit Pass" || echo "$method-fit Fail"
        rm student.db
    done
    echo "\n"
done

echo "double inputs"
for method in "${methods[@]}"
do
    ./a.out --$method-fit student.db < Test/input-01.txt > Test/my-output-01-$method.txt 2> /dev/null
    ./a.out --$method-fit student.db < Test/input-02.txt > Test/my-output-02-$method.txt 2> /dev/null
    sdiff -s Test/output-01-$method.txt Test/my-output-01-$method.txt && echo "$method-fit 01 Pass" || echo "$method-fit 01 Fail"
    sdiff -s Test/output-02-$method.txt Test/my-output-02-$method.txt && echo "$method-fit 02 Pass" || echo "$method-fit 02 Fail"
    rm student.db
done