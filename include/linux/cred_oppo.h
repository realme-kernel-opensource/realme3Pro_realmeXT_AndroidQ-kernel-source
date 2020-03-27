/*
 *Copyright (c)  2018  Guangdong OPPO Mobile Comm Corp., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _OPPO_CRED_H
#define _OPPO_CRED_H

#include "cred.h"

#ifdef VENDOR_EDIT
#ifdef CONFIG_OPPO_FG_OPT
/* Huacai.Zhou@PSW.BSP.Kernel.MM, 2018-07-07, add fg process opt*/
extern bool is_fg(int uid);
static inline int current_is_fg(void)
{
	int cur_uid;
	cur_uid = current_uid().val;
	if (is_fg(cur_uid))
		return 1;
	return 0;
}

static inline int task_is_fg(struct task_struct *tsk)
{
	int cur_uid;
	cur_uid = task_uid(tsk).val;
	if (is_fg(cur_uid))
		return 1;
	return 0;
}
#else
static inline int current_is_fg(void)
{
	return 0;
}


static inline int task_is_fg(struct task_struct *tsk)
{
	return 0;
}
#endif /*CONFIG_OPPO_FG_OPT*/
#endif /*VENDOR_EDIT*/

#endif /* _OPPO_CRED_H */
