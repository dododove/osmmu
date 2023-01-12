#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// defined constants
#define SUCCESS 0
#define NOT_VALID -1
#define NOT_ACCESSIBLE -2
#define TRUE 1
#define FALSE 0
#define VALID_MASK 0b01
#define ACCESS_MASK 0b10
#define PFN_POSITION 12
//taehyung
#define PTE_SIZE 4 //byte

// global variables for MMU
unsigned char *PTBR = NULL;
unsigned int VPN_MASK = 0;
unsigned int SHIFT = 0;
unsigned int PFN_SHIFT = 0;
unsigned int OFFSET_MASK = 0;

// function declaration
void init_mmu_variables(int address_space_bits, int page_size);
void alloc_page_table(int address_space_bits, int page_size);
void init_page_table(int address_space_bits, int page_size);
int mmu_address_translation(unsigned int virtual_address, unsigned int *physical_address);

/* This function initializes the globl variables for MMU. 
   For each meaning, refer to Page 13 of the lecture slide of Chapter 18.
*/
void init_mmu_variables(int address_space_bits, int page_size)
{
    VPN_MASK = (0xffffffff >> (int)log2(page_size)) << (int)log2(page_size);
    SHIFT = (int)log2(page_size);
    PFN_SHIFT = SHIFT;
    OFFSET_MASK = 0xffffffff >> (sizeof(OFFSET_MASK) * 8 - (int)log2(page_size));
}

/* This function initializes the global variable, PTBR. 
   First, caculate the number of PTEs in the page table.
   Then, allocate memory for the page table by malloc() with the number of PTEs * the size of PTE.
   Third, memset the allocated memory.
*/
void alloc_page_table(int address_space_bits, int page_size)
{
    /* Please implement your own code below */
    int number_of_ptes = pow(2, address_space_bits - (int)log2(page_size));
    char* ptr = malloc(number_of_ptes * PTE_SIZE);
    PTBR = ptr;
    memset(ptr,0,sizeof(number_of_ptes * PTE_SIZE));
}

/* This function inserts initial PTEs into the page_table.
   It will fill the page table only half.
   Also, if i % 4 == 0, the PTE will be inaccessible.
*/
void init_page_table(int address_space_bits, int page_size)
{
    unsigned int i;
    int number_of_ptes = pow(2, address_space_bits - (int)log2(page_size));

    /* fill the page table only half */
    for (i = 0; i < number_of_ptes / 2; i++)
    {
        unsigned int *pte_addr = (unsigned int *) (PTBR + i * sizeof(unsigned int));
        *pte_addr = (i * 2) << PFN_POSITION;
        if (i % 4 == 0)
            *pte_addr = *pte_addr | VALID_MASK; // make this pte as valid and inaccessible
        else
            *pte_addr = *pte_addr | VALID_MASK | ACCESS_MASK; // make this pte as valid and accessible
    }
}

/* This function performs address translation from virtual to physical.
   To implement this function, refer to Page 13 of the lecture slide of Chapter 18.
   The function returns SUCCESS if successful, and copies the translated address into the physical_adress variable.
   The function returns NOT_VALID if the PTE is not valid.
   The function returns NOT_ACCESSIBLE if the PTE is not valid.
*/
int mmu_address_translation(unsigned int virtual_address, unsigned int *physical_address)
{

    /* Please implement your own code below */

    unsigned int vpn;
    unsigned int pfn;
    int valid;
    int access;
    //Extract VPN from virtual address
    vpn = (virtual_address & VPN_MASK) >>SHIFT;
    //Form the address of the PTE
    unsigned int* PTEAddr = (unsigned int*) (PTBR + (vpn * PTE_SIZE));
    //Fetch the PTE
    unsigned int PTE = *PTEAddr;
    //Extract pfn, valid, access bit
    pfn = PTE >> PFN_POSITION;
    valid = PTE & VALID_MASK;
    access = (PTE & ACCESS_MASK) >> 1;

    printf(" (vpn:%u, pfn: %u, valid: %d, access: %d) ", vpn, pfn, valid, access);
    
    //Check if process can access the page
    if(valid == 0) return NOT_VALID;
    else if (access == 0) return NOT_ACCESSIBLE;
    else {
        unsigned int offset = virtual_address & OFFSET_MASK;
        *physical_address = (pfn << PFN_SHIFT) | offset;
        return SUCCESS;
    }
}

// Please do not modify the main() function.
int main(int argc, char *argv[])
{
    printf("Welcome to Software-managed MMU\n");

    if (argc != 3)
    {
        printf("Usage: ./mmu [address_space_size_in_bits] [page_size_in_bytes]\n");
        exit(1);
    }

    int address_space_bits = atoi(argv[1]);
    int page_size = atoi(argv[2]);

    if (address_space_bits < 1 || address_space_bits > 32)
    {
        printf("address_space_bits shoud be between 1 and 32\n");
        exit(1);
    }

    if (page_size < 1 || page_size > 4096)
    {
        printf("page_size shoud be between 1 and 4096\n");
        exit(1);
    }

    alloc_page_table(address_space_bits, page_size);

    if (PTBR == NULL)
    {
        printf("Please allocate the page table with malloc()\n");
        exit(1);
    }

    init_page_table(address_space_bits, page_size);
    init_mmu_variables(address_space_bits, page_size);

    while (1)
    {
        unsigned int value;
        printf("Input a virtual address of hexadecimal value without \"0x\" (-1 to exit): ");
        scanf("%x", &value);

        if (value == -1)
            break;

        printf("Virtual address: %#x", value);

        unsigned int physical_address = 0;
        int result = mmu_address_translation(value, &physical_address);
        if (result == NOT_VALID)
        {
            printf(" -> Segmentation Fault.\n");
        }
        else if (result == NOT_ACCESSIBLE)
        {
            printf(" -> Protection Fault.\n");
        }
        else if (result == SUCCESS)
        {
            printf(" -> Physical address: %#x\n", physical_address);
        }
        else
        {
            printf(" -> Unknown error.\n");
        }
    }

    if (PTBR != NULL)
        free(PTBR);

    return 0;
}