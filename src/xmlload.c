/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * xmlload.c
 *
 * Copyright (C) 2002 David Malcolm
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

/*
  xmlload.c
  
  This traverses the libxml structures below a xmlDocPtr and generates a TTREE*
  corresponding to that which would have been created by the old flux xml loaders.
  
  The code is a hybrid of the libxml tree traversal code in
  libxml's xmlDebugDumpDocument, and the TTREE creation on code found in
  flux-0.2.8/src/xml/xmltree-rxp.c.
*/

#include <glib.h>
#include <libxml/tree.h>

#include "global.h" 
/* purely to get at build settings */

/* This is dead code */
