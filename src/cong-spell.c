/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-spell.c
 *
 * Copyright (C) 2004 David Malcolm
 *
 * Conglomerate is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * Conglomerate is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Authors: David Malcolm <david@davemalcolm.demon.co.uk>,
 * Jeff Martin <jeff@custommonkey.org>
 */

#include "global.h"
#include "cong-util.h"
#include "cong-spell.h"

#if ENABLE_ENCHANT
#include <enchant.h>

EnchantDict *dict;

void create_dict()
{
	EnchantBroker * broker;
	
	broker = enchant_broker_init();
	dict = enchant_broker_request_dict(broker, "en_GB");
}

gboolean cong_is_word_misspelt (const gchar* string, const CongWord* word)
{
	if(!dict)
	{
		create_dict();
	}

	return enchant_dict_check(dict, string + word->start_byte_offset,
			word->length_in_bytes);
}
#endif
