/*
All the main functions with respect to the MeMS are inplemented here
read the function discription for more details

NOTE: DO NOT CHANGE THE NAME OR SIGNATURE OF FUNCTIONS ALREADY PROVIDED
you are only allowed to implement the functions 
you can also make additional helper functions a you wish

REFER DOCUMENTATION FOR MORE DETAILS ON FUNSTIONS AND THEIR FUNCTIONALITY
*/
// add other headers as required
#include<stdio.h>
#include<stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
/*
Use this macro where ever you need PAGE_SIZE.
As PAGESIZE can differ system to system we should have flexibility to modify this 
macro to make the output of all system same and conduct a fair evaluation. 
*/
#define PAGE_SIZE 4096

typedef struct MainchainNode{
    void*  physical_address;
    void* stvirtual_add;
    size_t size;
    struct SubchainNode* sub;
    struct MainchainNode* next;
    struct MainchainNode* prev;
}MainchainNode;
struct MainchainNode* Global_Head;
void* Staring_Physical_Adrress=NULL;
void* counter = (void*)1000;
typedef struct SubchainNode{
    void* physical_address;
    void* virtual_address;
    int isProcess;
    size_t size;
    struct SubchainNode* prev;
    struct SubchainNode* next;
}SubchainNode;

/*
Initializes all the required parameters for the MeMS system. The main parameters to be initialized are:
1. the head of the free list i.e. the pointer that points to the head of the free list
2. the starting MeMS virtual address from which the heap in our MeMS virtual address space will start.
3. any other global variable that you want for the MeMS implementation can be initialized here.
Input Parameter: Nothing
Returns: Nothing
*/
void mems_init(){
    struct MainchainNode* Global_Head;
    void* Staring_Physical_Adrress=NULL;
    void* counter = (void*)1000;
}


/*
This function will be called at the end of the MeMS system and its main job is to unmap the 
allocated memory using the munmap system call.
Input Parameter: Nothing
Returns: Nothing
*/
void mems_finish() {
    struct MainchainNode* main_node = Global_Head;
    while (main_node != NULL) {
        struct SubchainNode* sub_node = main_node->sub;
        while (sub_node != NULL) {
            struct SubchainNode* temp = sub_node;
            sub_node = sub_node->next;
            
            if (munmap(temp, temp->size) == -1) {
                perror("MUNMAP");
                exit(EXIT_FAILURE);
            }
        }
        struct MainchainNode* temp = main_node;
        main_node = main_node->prev;
        if (temp != NULL) {
            if (munmap(temp, temp->size) == -1) {
                perror("MUNMAP");
                exit(EXIT_FAILURE);
            }
        }
    }
}

/*
Allocates memory of the specified size by reusing a segment from the free list if 
a sufficiently large segment is available. 

Else, uses the mmap system call to allocate more memory on the heap and updates 
the free list accordingly.

Note that while mapping using mmap do not forget to reuse the unused space from mapping
by adding it to the free list.
Parameter: The size of the memory the user program wants
Returns: MeMS Virtual address (that is created by MeMS)
*/ 
void* mems_malloc(size_t size){
    int Multiplier=0;
    size_t tempsize=size;
    while(tempsize%PAGE_SIZE!=0){
        tempsize/=PAGE_SIZE;
        Multiplier++;
    }
    int outofspace=0;
    if(Global_Head==NULL){
        Global_Head=(MainchainNode*)mmap(NULL,PAGE_SIZE*Multiplier,PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANON,0,0);
        if (Global_Head == MAP_FAILED) {
        perror("MMAP");
        exit(1);
        }

        Global_Head->physical_address=(void *)Global_Head;
        Global_Head->size=PAGE_SIZE*Multiplier;
        Global_Head->stvirtual_add=counter;
        counter+=Global_Head->size;
        Global_Head->next=NULL;
        Global_Head->prev=NULL;

        void* temp_physical_add=Global_Head->physical_address;
        void* temp_virtual_add=Global_Head->stvirtual_add;

        struct SubchainNode* new_sub_node=(SubchainNode*)mmap(NULL,size,PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANON,0,0);
        if (new_sub_node == MAP_FAILED) {
        perror("MMAP");
        exit(1);
        }
        new_sub_node->size=(size);
        new_sub_node->virtual_address=temp_virtual_add;
        temp_virtual_add+=size;
        new_sub_node->physical_address=temp_physical_add;
        temp_physical_add+=size;
        new_sub_node->isProcess=1;

        struct SubchainNode* new_hole=(SubchainNode*)mmap(NULL,(PAGE_SIZE*Multiplier)- size,PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANON,0,0);
        if (new_hole == MAP_FAILED) {
        perror("MMAP");
        exit(1);
        }
        new_hole->size=((PAGE_SIZE*Multiplier)- size);
        new_hole->virtual_address=temp_virtual_add;
        temp_virtual_add+=(PAGE_SIZE*Multiplier)- size;
        new_hole->physical_address=temp_physical_add;
        temp_physical_add+=size;

        new_sub_node->next=new_hole;
        new_sub_node->prev=NULL;
        new_hole->prev=new_sub_node;
        new_hole->next=NULL;

        Global_Head->sub=new_sub_node;
        return new_sub_node->virtual_address;
    }else{
        struct MainchainNode* main_node=Global_Head;
        while(main_node!=NULL){
            struct SubchainNode* sub_node = main_node->sub;
            while(sub_node!=NULL){
                if(sub_node->isProcess==0){
                    if(sub_node->size>size){
                        struct SubchainNode* new_hole=(SubchainNode*)mmap(NULL,sub_node->size-size,PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS,0,0);
                        if (new_hole == MAP_FAILED) {
                        perror("MMAP");
                        exit(1);
                        }
                        new_hole->size=sub_node->size-size;
                        new_hole->virtual_address=sub_node->virtual_address+size;
                        new_hole->physical_address=sub_node->physical_address+size;
                        new_hole->isProcess=0;

                        sub_node->size=size;
                        sub_node->isProcess=1;

                        sub_node->next=new_hole;
                        new_hole->prev=sub_node;
                        new_hole->next=NULL;
                        return sub_node->virtual_address;
                    }else if(sub_node->size==size){
                        sub_node->isProcess=1;
                        return sub_node->virtual_address+size;
                    }else{
                        outofspace=1;
                        break;
                    }
                }
                sub_node=sub_node->next;
            }
            main_node=main_node->next;
        }
    }
    if(outofspace==1){
        struct MainchainNode* main_node=Global_Head;
        while(main_node->next!=NULL){
            main_node=main_node->next;
        }
        struct MainchainNode* new_main_node=(MainchainNode*)mmap(NULL,PAGE_SIZE*Multiplier,PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS,0,0);
        if (new_main_node == MAP_FAILED) {
        perror("MMAP");
        exit(1);
        }
        new_main_node->physical_address=(void *)new_main_node;
        new_main_node->size=PAGE_SIZE*Multiplier;
        new_main_node->stvirtual_add=counter;
        counter+=new_main_node->size;
        new_main_node->next=NULL;
        new_main_node->prev=main_node;
        main_node->next=new_main_node;


        void* temp_physical_add=new_main_node->physical_address;
        void* temp_virtual_add=main_node->stvirtual_add+main_node->size;

        struct SubchainNode* new_sub_node=(SubchainNode*)mmap(NULL,size,PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS,0,0);
        if (new_sub_node == MAP_FAILED) {
        perror("MMAP");
        exit(1);
        }
        new_sub_node->size=(size);
        new_sub_node->virtual_address=temp_virtual_add;
        temp_virtual_add+=size;
        new_sub_node->physical_address=temp_physical_add;
        temp_physical_add+=size;
        new_sub_node->isProcess=1;

        struct SubchainNode* new_hole=(SubchainNode*)mmap(NULL,(PAGE_SIZE*Multiplier)- size,PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANON,0,0);
        if (new_hole == MAP_FAILED) {
        perror("MMAP");
        exit(1);
        }
        new_hole->size=((PAGE_SIZE*Multiplier)- size);
        new_hole->virtual_address=temp_virtual_add;
        temp_virtual_add+=(PAGE_SIZE*Multiplier)- size;
        new_hole->physical_address=temp_physical_add;
        temp_physical_add+=size;

        new_sub_node->next=new_hole;
        new_sub_node->prev=NULL;
        new_hole->prev=new_sub_node;
        new_hole->next=NULL;

        new_main_node->sub=new_sub_node;
        return new_sub_node->virtual_address;
    }

}


/*
this function print the stats of the MeMS system like
1. How many pages are utilised by using the mems_malloc
2. how much memory is unused i.e. the memory that is in freelist and is not used.
3. It also prints details about each node in the main chain and each segment (PROCESS or HOLE) in the sub-chain.
Parameter: Nothing
Returns: Nothing but should print the necessary information on STDOUT
*/
void mems_print_stats(){
    printf("----- MeMs SYSTEM STATS -----\n");
    struct MainchainNode* main_node = Global_Head;
    int page_count=0;
    int main_length=0;
    long long int unused_space=0;
    int arr[100];
    int i=0;
    while(main_node!= NULL){
        struct SubchainNode* sub_node = main_node->sub;
        int sub_count=0;
        printf("Main[%ld:%ld]-> ",(intptr_t)main_node->stvirtual_add,(intptr_t)(main_node->stvirtual_add)+(intptr_t)(main_node->size)-1);
        while (sub_node != NULL) {
            if(sub_node->isProcess==0){
                printf("H[%ld:%ld] <->",(intptr_t)(sub_node->virtual_address),(intptr_t)(sub_node->size)+(intptr_t)(sub_node->virtual_address)-1);
                unused_space+=sub_node->size;
            }else{
                printf("P[%ld:%ld] <-> ",(intptr_t)(sub_node->virtual_address),(intptr_t)(sub_node->size)+(intptr_t)(sub_node->virtual_address)-1);
            }
            sub_node = sub_node->next;
            sub_count++;
        }
        printf("NULL\n");
        arr[i]=sub_count;
        page_count+=(main_node->size)/4096;
        main_node = main_node->next;
        main_length++;
        i++;
    }
    printf("Pages Used:%d\nSpace Unused:%lld\nMain Chain Length:%d\n",page_count,unused_space,main_length);
    printf("Sub-Chain Length Array: [");
    for(int j=0;j<i;j++){
        printf("%d, ",arr[j]);
    }
    printf("]\n");
}


/*
Returns the MeMS physical address mapped to ptr ( ptr is MeMS virtual address).
Parameter: MeMS Virtual address (that is created by MeMS)
Returns: MeMS physical address mapped to the passed ptr (MeMS virtual address).
*/
void *mems_get(void* v_ptr){
    struct MainchainNode* main_node = Global_Head;
    while (main_node != NULL) {
        struct SubchainNode* sub_node = main_node->sub;
        while (sub_node != NULL) {
            if (v_ptr == sub_node->virtual_address) {
                return sub_node->physical_address;
            } else if (sub_node->next != NULL) {
                if(sub_node->next->virtual_address > v_ptr){
                    intptr_t temp = (intptr_t)v_ptr - (intptr_t)sub_node->virtual_address;
                    return (void *)((intptr_t)sub_node->physical_address + temp);
                }
            }
            sub_node = sub_node->next;
        }
        main_node = main_node->next;
    }
    return NULL;
}


/*
this function free up the memory pointed by our virtual_address and add it to the free list
Parameter: MeMS Virtual address (that is created by MeMS) 
Returns: nothing
*/
void mems_free(void* v_ptr) {
    struct MainchainNode* main_node = Global_Head;
    while (main_node != NULL) {
        struct SubchainNode* sub_node = main_node->sub;
        while (sub_node != NULL) {
            if (v_ptr == sub_node->virtual_address) {
                // For Freeing Memory of first subnode.
                if(sub_node->prev==NULL){
                    sub_node->isProcess=0;
                    break;
                }
                sub_node->isProcess = 0;

                // For Freeing Memory of rest subnodes.
                if (sub_node->prev->isProcess == 0) {
                    if(sub_node->prev != NULL ){
                        sub_node->prev->size += sub_node->size;
                        sub_node->prev->next = sub_node->next;
                        if (sub_node->next != NULL) {
                            sub_node->next->prev = sub_node->prev;
                        }
                        sub_node = sub_node->prev;
                    }
                }
                if (sub_node->next->isProcess == 0) {
                    if(sub_node->next != NULL){
                        sub_node->size += sub_node->next->size;
                        sub_node->next = sub_node->next->next;
                        if (sub_node->next != NULL) {
                            sub_node->next->prev = sub_node;
                        }
                    }
                }
            }
            sub_node = sub_node->next;
        }
        main_node = main_node->next;
    }
}