//by wushuangzhi
#ifndef CONFIG_H
#define CONFIG_H

#include "globaldefs.h"

struct tf_config *readconfig(char *fname);
int add_sensor(struct tf_config *cfg, struct sensor *sensor);
int add_limit(struct tf_config *cfg, struct limit *limit);
void free_config(struct tf_config *cfg);

#endif
