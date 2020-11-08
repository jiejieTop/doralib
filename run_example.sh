#!/bin/bash

example_list=`ls build/bin`

for example in $example_list;  
    do   
        ./build/bin/$example; 
        wait;
    done   


