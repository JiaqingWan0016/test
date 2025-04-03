#ifndef IF_SYNC_H
#define IF_SYNC_H

#include "linkd.h"

/* 同步接口状态
 * @param if_name: 接口名称
 * @return: 成功返回0，失败返回-1
 */
int sync_interface_state(const char *if_name);

#endif /* IF_SYNC_H */ 