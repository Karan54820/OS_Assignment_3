# OS_Assignment_3
## mems_init
In this funciton we are initialising the following:-

1. Free List Head: This points to where the memory management begins.
2. MeMS Virtual Address Start: It sets the starting point for memory allocation.
3. Other Globals: Any extra global variables your MeMS system needs.

## mems_malloc
This function reserves memory of a given size. It does this by first checking if there's a suitable chunk of free memory available. If such a chunk of memory exists in the free list, it's allocated to the process. Otherwise, it uses the mmap system call to allocate more memory on the heap and updates the free list accordingly.
we have taken three cases for the allocation of memory:-
1. If the main node is empty
2. If the main node is not empty and there is enough space present in it for the process
3. If the main node is not empty but there is not enough space present in it for the process

## mems_get
In this function we have used two while loops to compare the pointer with the virtual address.If it is equal we are returning the physical address of the same node else we are returning the physical address of the previous node plus the difference of their virtual address.

## mems_print_stats
In this function we have used two while loops to print the required items.

## mems_free
In this function we have implemented two cases:-
1. If the node to be freed has a hole on its next node
2. If the node to be freed has a process on its next node

##mems_finish
Here we are deallocating the memory using unmap.we have used a temp variable to ensure proper deallocation.


----------------------------------------------------------------------------------------------------------------------------------------------


# MeMS: Memory Management System [CSE231 OS Assignment 3]
[Documentation](https://docs.google.com/document/d/1Gs9kC3187lLrinvK1SueTc8dHCJ0QP43eRlrCRlXiCY/edit?usp=sharing)
---

### How to run the exmaple.c
After implementing functions in mems.h follow the below steps to run example.c file
```
$ make
$ ./example
```
---
### Format of mems_mems_print_stats
* For every Subchain in the free list print the data as follows 
```
MAIN[starting_mems_vitual_address:ending_mems_vitual_address] -> <HOLE or PROCESS>[starting_mems_vitual_address:ending_mems_vitual_address] <-> ..... <-> NULL
```
* After printing the whole freelist print the following stats
```
Page used: <Total Pages used by the user>
Space unused: <Total space mapped but not used by user>
Main Chain Length: <Length of the main chain>
Sub-chain Length array: <Array denoting the length of the subchains>
```
* Analyse the output of the example.c file below to get a better understanding of the output
---
### Example Output
PAGE_SIZE= 4096 Bytes

Starting MeMS Virtual Address= 1000

![Example Output](example_output.jpg)

-------------------------------------------------------------------------------------------------------------------------------------------------
