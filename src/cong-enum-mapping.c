/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * cong-enum-mapping.c
 *
 * Copyright (C) 2003 David Malcolm
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
 * Authors: David Malcolm <david@davemalcolm.demon.co.uk>
 */

#include "global.h"
#include "cong-enum-mapping.h"

/**
 * cong_enum_mapping_lookup:
 * @enum_mapping:
 * @num_values:
 * @text_value:
 * @default_value:
 *
 * TODO: Write me
 * Returns:
 */
guint
cong_enum_mapping_lookup (const CongEnumMapping *enum_mapping,
			  guint num_values,
			  const gchar *text_value,
			  guint default_value)
{
	guint i;

	g_return_val_if_fail (enum_mapping, default_value);

	if (NULL==text_value) {
		return default_value;
	}

	for (i=0;i<num_values;i++) {
		if (0==strcmp(enum_mapping[i].text_value, text_value)) {
			return enum_mapping[i].numeric_value;
		}
	}

	/* Unrecognised value: */
	return default_value;	
}

/**
 * cong_enum_mapping_lookup_string:
 * @enum_mapping:
 * @num_values:
 * @value:
 *
 * TODO: Write me
 * Returns:
 */
const gchar*
cong_enum_mapping_lookup_string (const CongEnumMapping *enum_mapping,
				 guint num_values,
				 guint value)
{
	guint i;

	g_return_val_if_fail (enum_mapping, NULL);

	for (i=0;i<num_values;i++) {
		if (enum_mapping[i].numeric_value==value) {
			return enum_mapping[i].text_value;
		}
	}

	/* Unrecognised value: */
	return NULL;	
}
