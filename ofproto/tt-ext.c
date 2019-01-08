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

#include <config.h>

#include "ofp-actions.h"
#include "ofp-msgs.h"
#include "ofp-util.h"
#include "ofproto-provider.h"
#include "openvswitch/vlog.h"

#include "tt-ext.h"

VLOG_DEFINE_THIS_MODULE(tt);

enum ofperr 
onf_tt_flow_receive_start(struct ofconn *ofconn, unsigned int flow_cnt)
{
    enum ofperr error;
   
    /* TODO(zhanghao):
     * 1. switch command type
     * 2. send flow count to kernel
     */

    return error;
}

enum ofperr 
onf_tt_flow_receive_end(struct ofconn *ofconn)
{
    enum ofperr error;

    /* TODO(zhanghao):
     * 1. send end msg to kernel
     * 2. wait result from kernel
     * 3. judgu whether result is error
     * 4. if error reply error msg
     */

    return error;
}
