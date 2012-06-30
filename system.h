//by wushuangzhi
#ifndef SYSTEM_H
#define SYSTEM_H

int get_temp_ibm();
int get_temp_sysfs();
int depulse_and_get_temp_ibm();
int depulse_and_get_temp_sysfs();
void setfan_ibm();
void setfan_sysfs();
void setfan_sysfs_safe();
int init_fan_ibm();
int preinit_fan_sysfs();
int init_fan_sysfs_once();
int init_fan_sysfs();
void uninit_fan_ibm();
void uninit_fan_sysfs();

#endif
