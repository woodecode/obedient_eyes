## Note

若使用FreeRTOS,将 Main(); 放在main.c /*USER CODE BEGIN 2*/区域并将无限循环删之

```c

osThreadId_t TaskHandleScreenUpdate;
void ThreadScreenUpdate(void* argument){
    UNUSED(argument);
    for(;;){
        lv_task_handler();
    }
}

void Main(){
	// ...
    // init
    
    // FreeRTOS init
    const osThreadAttr_t ThreadAttrScreenUpdate = {
            .name = "TaskScreenUpdate",
            .stack_size = 500,
            .priority = (osPriority_t) osPriorityNormal
    };
    
    TaskHandleScreenUpdate = osThreadNew(
        ThreadScreenUpdate, 
        nullptr,
        &ThreadAttrScreenUpdate
    );

}
```