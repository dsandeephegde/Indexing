**In-Memory Indexing with Availability Lists**

Goal: 
- To build and maintain an in-memory primary key index to improve search efficiency.
- To use an in-memory availability list to support the reallocation of space for records that are deleted.

*Student Database File*

`712412913|Ford|Rob|Phi` represents a student with an SID of 712412913, a last of Ford, a first of Rob, and a major of Phi.

*Availability list order*
- First fit
- Best fit
- Worst fit

*User Interface*
- add key rec

    Adds a new record rec with an SID of key to the student file. The format of rec is a |-delimited set of fields (exactly as described in Student File section above), for example
    
    `add 712412913 712412913|Ford|Rob|Phi`
    
    adds a new record with an SID of 712412913, a last of Ford, a first of Rob, and a major of Phi.
    
    Binary search the index for an entry with a key value equal to the new rec's SID. Writes the following on-screen
    
    `Record with SID=key exists`

- find key
    
    Finds the record with SID of key in the stu­dent file, if it exists. The record should be printed in |-delimited for­mat, (exactly as described in Student File section above), for example

    `find 712412913`

    should print on-screen

    `712412913|Ford|Rob|Phi`
    
    If no index entry with the given key exists, Writes the following on-screen

    `No record with SID=key exists`

- del key
    Delete the record with SID of key from the student file, if it exists.

    If no index entry with the given key exists, Prints the following on-screen
    
    `No record with SID=key exists`

- end
    End the program, close the student file, and write the index and availability lists to the corresponding index and availability files.

*Program Execution*

`
cc main.c
`

`
./a.out availListOrder studentFileName
`

Example: 

`./a.out --best-fit student.db`

Test Cases

Test cases can be run by following command

`sh test.sh`
