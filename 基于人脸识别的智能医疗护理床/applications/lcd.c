/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-05-25     lenovo       the first version
 */
#include <lcd.h>
#include <drv_lcd.h>

void lcd_config()
{
    lcd_clear(WHITE);

    lcd_set_color(WHITE, BLACK);

    lcd_show_string(10, 20, 24, "Seat High:");
    lcd_show_string(10, 50, 24, "Seat Angle:");
    lcd_show_string(130, 20, 24, "*");        //lcd_fill(130,10,180,40,WHITE);//清除*上半部分
                                               // lcd_fill(140,40,180,80,WHITE);//清除*下半部分
    lcd_show_string(140, 50, 24, "*");
    lcd_show_string(10, 80, 24, "Temp:");
    lcd_show_string(10, 110, 24, "Hum:");
    lcd_show_string(10, 140, 24, "Name:");//lcd_fill(70,130,110,180,WHITE);//清除name后部分
    lcd_show_string(90, 110, 24, "%%");
    lcd_show_string(90, 80, 24, "c");
    lcd_fill(110,170,115,250,BLACK);//竖划线
    lcd_show_string(10, 180, 24, "add face");
    lcd_show_string(122, 180, 24, "settings");//设置参数
    //lcd_show_string(10, 180, 24, "begin...");
    //lcd_show_string(10, 180, 24, "add ok!");
    //lcd_fill(10,210,100,220,RED);//addface下划线
    //lcd_fill(120,210,210,220,RED);//set下划线
    //lcd_fill(10,170,105,250,WHITE);//清除addface半部分
    //lcd_fill(10,170,220,250,WHITE);//清除设置界面，name下半部分
}

void write()
{

    lcd_show_string(55, 160, 32, "writing");
    rt_thread_mdelay(20);
    lcd_show_string(55, 160, 32, "writing");
    rt_thread_mdelay(20);
    lcd_show_string(55, 160, 32, "writing");
    rt_thread_mdelay(20);
    lcd_show_string(55, 160, 32, "writing");
    rt_thread_mdelay(20);
    lcd_show_string(55, 160, 32, "writing");
    rt_thread_mdelay(20);
    lcd_show_string(55, 160, 32, "writing");
    rt_thread_mdelay(20);
    lcd_show_string(55, 160, 32, "writing");
    rt_thread_mdelay(20);
    lcd_show_string(55, 160, 32, "writing");
    rt_thread_mdelay(20);
    lcd_show_string(55, 160, 32, "writing");
    rt_thread_mdelay(20);
    lcd_show_string(55, 160, 32, "writing");
    rt_thread_mdelay(20);
    lcd_show_string(55, 160, 32, "       ");
}
