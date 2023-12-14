#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifndef TRUE
#define TRUE        1
#endif

#ifndef FALSE
#define FALSE       0
#endif

#define MAX_ALOCATION_ALLOWED       30 //최대 malloc 개수 
static unsigned char g_our_memory[1024 * 256]; // 공유메모리
volatile static int g_allocted_number;
volatile static int g_remindSize;
volatile static int g_heap_base_address;

typedef struct malloc_info
{
	int address;
	int size;
}malloc_info_t;
// 메모리 관리 구조체 
 static malloc_info_t   metadata_info[MAX_ALOCATION_ALLOWED];// = { 0 };

/*
함수 : my_mem_init
설명 : 커스텀 malloc를 위한 초기화 함수 
*/
void my_mem_init(void)
{
	int i;
	for (i = 0; i < MAX_ALOCATION_ALLOWED; i++)
	{
		metadata_info[i].address = 0;
		metadata_info[i].size = 0;
	}
	g_heap_base_address = (int)&g_our_memory[0];
	g_allocted_number =  0;
	g_remindSize = sizeof(g_our_memory);
}

/*
함수 : my_malloc
설명 : 커스텀 malloc 함수. 
	   return NULL : 메모리 없음 및 할당 불가
	   그외 : 사용가능한 메모리 주소	   
*/
void* my_malloc(int size)
{
	int j = 0;
	int index = 0;
	int initial_gap = 0;
	int gap = 0;
	int flag = FALSE;
	int initial_flag = FALSE;
	void *address = NULL;
	int heap_index = 0;
	malloc_info_t temp_info = { 0 };

	if (g_allocted_number >= MAX_ALOCATION_ALLOWED || size <= 0 || g_remindSize < size)
	{
		return NULL;
	}

	for (index = 0; index < g_allocted_number; index++)
	{
		if (metadata_info[index + 1].address != 0)
		{	
			/*할당된 메모리 사이의 갭 사이즈 비교 case  */
			gap = metadata_info[index + 1].address - (metadata_info[index].address + metadata_info[index].size);  
			if (gap >= size)
			{
				flag = TRUE;
				break;
			}			
		}
	}

	if (flag == TRUE)    /*case 에서 할당된 메모리 인덱스 추출*/
	{
		heap_index = ((metadata_info[index].address + metadata_info[index].size) - g_heap_base_address);
		index = g_allocted_number;
	}	
	else 
	{
		if (g_allocted_number != 0) //갭 영역이 없으면,
		{
			heap_index = ((metadata_info[index - 1].address + metadata_info[index - 1].size) - g_heap_base_address);
		}
		else    /*최초 할당 */
			heap_index = 0;
	}

	address = &g_our_memory[heap_index];
	metadata_info[index].address = g_heap_base_address + heap_index;
	metadata_info[index].size = size;
	g_remindSize -= size;

	g_allocted_number += 1;

	for (j = 0; j < g_allocted_number-1; j++)
	{
		if (metadata_info[j].address > metadata_info[j + 1].address)
		{
			temp_info.address = metadata_info[j +1 ].address;
			temp_info.size = metadata_info[j + 1].size;

			metadata_info[j + 1].address = metadata_info[j].address;
			metadata_info[j + 1].size = metadata_info[j].size;

			metadata_info[j].address = temp_info.address ;
			metadata_info[j].size = temp_info.size;
			j = -1;
		}
	}
	return address;
}
/*
함수 : my_free
설명 : 커스텀 free 함수.
*/
void my_free(int address)
{
	int i = 0,j;
	int copy_meta_data = FALSE;
	malloc_info_t temp_info = { 0 };
	//어드레스가 0,NULL은 처리 안함.
	if (address == (int)NULL || address == 0) return;

	for (i = 0; i < g_allocted_number; i++)
	{
		/*할당된 메모리 주소 있는지 검사 */
		if (address == metadata_info[i].address)
		{			
			g_allocted_number -= 1;
			g_remindSize += metadata_info[i].size;
			copy_meta_data = TRUE;
			//printf("g_allocted_number in free = %d %d\n", g_allocted_number, address);
			break;
		}
	}
	/*메모리 관리에서 메모리 할당 제거*/
	if (copy_meta_data == TRUE)
	{		
		if (i == MAX_ALOCATION_ALLOWED - 1)
		{
			metadata_info[i].address = 0;
			metadata_info[i].size = 0;
		}
		else {
			//한칸씩 앞으로
			for (j = i; j < g_allocted_number; j++)
			{
				memcpy(&metadata_info[j], &metadata_info[j + 1], sizeof(malloc_info_t));
			}
			metadata_info[j ].address = 0;
			metadata_info[j ].size = 0;
			
		}			

	}
}

int main() {
	int a = 1;
	
	int *ptr = NULL;
	int *ptr1 = NULL;
	int *ptr2 = NULL;
	int *ptr3 = NULL;
	int *ptr4 = NULL;
	int *ptr5 = NULL;
	int *ptr6 = NULL;
	
	my_mem_init();

	ptr = my_malloc(10);
	ptr1 = my_malloc(20);
	ptr2 = my_malloc(30);
	ptr3 = my_malloc(255*1024);

	printf("Addresses are: %d, %d, %d, \n", ptr, ptr1, ptr2);

	my_free((int)ptr2);
	ptr2 = 0;
	ptr3 = my_malloc(9);
	ptr4 = my_malloc(6);
	my_free((int)ptr4);
	my_free((int)ptr3);
	ptr5 = my_malloc(31);
	ptr6 = my_malloc(5);

	printf("Addresses are: %d, %d, %d, %d, %d, %d, %d\n", ptr, ptr1, ptr2, ptr3, ptr4, ptr5, ptr6);

	return 0;
}