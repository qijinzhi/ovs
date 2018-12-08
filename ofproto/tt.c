/*
 * Copyright (c) 2013, 2014 Alexandru Copot <alex.mihai.c@gmail.com>, with support from IXIA.
 * Copyright (c) 2013, 2014 Daniel Baluta <dbaluta@ixiacom.com>
 * Copyright (c) 2014, 2015 Nicira, Inc.
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

#include "tt.h"

enum ofperr 
tt_table_mod(struct ofconn *ofconn, struct ofputil_tt_table_mod *ttm)
{
	struct ofproto *ofproto = ofconn_get_ofproto(ofconn);
	switch (ttm->command) {
	case TXTTMC_ADD: //openflow protocol tt command add
		ofproto->ofproto_class->tt_table_add(ofproto, ttm);
	}
	return 0;
}
