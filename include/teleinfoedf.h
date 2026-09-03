/******************************************************************************************************************************/
/* ABLS-AGENT-TELEINFOEDF/include/teleinfoedf.h  Header Teleinfo EDF                                                        */
/* Projet Abls-Habitat                   Gestion d'habitat                                                14.08.2026 14:30:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * teleinfoedf.h
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2026 - Sebastien LEFEVRE
 *
 * ABLS-AGENT-TELEINFOEDF is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * ABLS-AGENT-TELEINFOEDF is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ABLS-AGENT-TELEINFOEDF; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#ifndef _ABLS_TELEINFOEDF_H_
#define _ABLS_TELEINFOEDF_H_

#include <abls-agent-libs/abls-agent-libs.h>

#define TAILLE_BUFFER_TELEINFO 128
#define TINFO_RETRY_DELAI_DS   600

enum {
  TINFO_WAIT_BEFORE_RETRY,
  TINFO_RETRYING,
  TINFO_CONNECTED
};

struct ABLS_TELEINFOEDF_VARS {
  gint mode;
  guint date_next_retry;
  gint fd;
  gchar buffer[TAILLE_BUFFER_TELEINFO];
  gint nbr_octet_lu;
  guint last_view_ds;

  JsonNode *Adco;
  JsonNode *Isous;
  JsonNode *Base;
  JsonNode *Hchc;
  JsonNode *Hchp;
  JsonNode *Iinst;
  JsonNode *Imax;
  JsonNode *Papp;
  JsonNode *IRMS1;
  JsonNode *IRMS2;
  JsonNode *IRMS3;
  JsonNode *URMS1;
  JsonNode *URMS2;
  JsonNode *URMS3;
  JsonNode *PREF;
  JsonNode *PCOUP;
  JsonNode *SINSTS;
  JsonNode *SINSTS1;
  JsonNode *SINSTS2;
  JsonNode *SINSTS3;
  JsonNode *SMAXSN;
  JsonNode *SMAXSN1;
  JsonNode *SMAXSN2;
  JsonNode *SMAXSN3;
  JsonNode *UMOY1;
  JsonNode *UMOY2;
  JsonNode *UMOY3;
  JsonNode *NTARF;
  JsonNode *ADSC;
  JsonNode *EAST;
  JsonNode *EASF01;
  JsonNode *EASF02;
  JsonNode *EASF03;
  JsonNode *EASF04;
  JsonNode *EASF05;
  JsonNode *EASF06;
  JsonNode *EASF07;
  JsonNode *EASF08;
  JsonNode *EASF09;
  JsonNode *EASF10;
  JsonNode *EASD01;
  JsonNode *EASD02;
  JsonNode *EASD03;
  JsonNode *EASD04;
  JsonNode *PRM;
};

#endif
