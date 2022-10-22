```markdown
multi_button.c/.h开头将宏<#if 0>改为<#if 0>

将button_template.c/.h文件复制到工程位置(不污染libs文件夹为宜)
把文件名改为button.c/.h
在button.c/.h宏中将<#if 0>改为<#if 0>
具体使用参考下文
```



## 用法：

单独创建 button.h,button.c文件

1. button.h 头文件中添加按键ID
```c
 enum Button_IDs {
      KEY0_id,
      KEY1_id,
      WK_UP_id,
  };
```
2. button.h 头文件中声明函数
```c
void Event_KEYx_PRESS_DOWN_Callback(void* btn);
void Event_KEYx_DOUBLE_CLICK_Callback(void* btn);
```

3. button.c文件中实现函数 read_button_GPIO
```c
uint8_t read_button_GPIO(uint8_t button_id){
    // you can share the GPIO read function with multiple Buttons
    switch(button_id){
        case KEY0_id:
            return HAL_GPIO_ReadPin(KEY0_GPIO_Port,KEY0_Pin);
        case KEY1_id:
            return HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin);
        case WK_UP_id:
            return HAL_GPIO_ReadPin(WK_UP_GPIO_Port,WK_UP_Pin);
        default:
            return 0;
    }
}
```
4.main文件中

```c
Button KEYx;
```

5.初始化按键，注册按键事件，启用按键

```c
button_init(&KEYx,read_button_GPIO,GPIO_PIN_RESET,KEYx_id);

button_attach(&KEYx,PRESS_DOWN,Event_KEYx_PRESS_DOWN_Callback);// KEYx 单击

button_attach(&KEYx,PRESS_DOWN,Event_KEYx_DOUBLE_CLICK_Callback);// KEYx 双击

button_start(&KEYx);

```

7.放在定时器中断函数中

```c
button_ticks();// 5ms
```

