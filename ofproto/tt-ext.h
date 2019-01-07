/*
 * Copyright (c) 2019 Tsinghua University, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TT_H
#define TT_H 1

#include <sys/types.h>

#include "connmgr.h"
#include "ofp-util.h"
#include "ofproto-provider.h"

#ifdef  __cplusplus
extern "C" {
#endif

enum ofperr onf_tt_flow_receive_start(struct ofconn *, unsigned int flow_cnt);
enum ofperr onf_tt_flow_receive_end(struct ofconn *);

#ifdef  __cplusplus
}
#endif

#endif
