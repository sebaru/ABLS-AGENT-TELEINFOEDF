/******************************************************************************************************************************/
/* ABLS-AGENT-TELEINFOEDF/src/teleinfoedf.c  Gestion des capteurs Teleinfo EDF                                                */
/* Projet Abls-Habitat                   Gestion d'habitat                                                14.08.2026 14:35:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * teleinfoedf.c
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

 #include <errno.h>
 #include <fcntl.h>
 #include <stdio.h>
 #include <string.h>
 #include <sys/select.h>
 #include <sys/stat.h>
 #include <sys/time.h>
 #include <termios.h>
 #include <time.h>
 #include <unistd.h>

 #include "teleinfoedf.h"

 struct ABLS_AGENT *Agent = NULL;                                                                     /* Structure de l'agent */
 struct ABLS_TELEINFOEDF_VARS *Agent_vars = NULL;                                       /* Structure des variables de l'agent */

/******************************************************************************************************************************/
/* Init_teleinfo: initialise l'acces serie au compteur Teleinfo                                                               */
/* Entrée: la structure agent                                                                                                 */
/* Sortie: l'identifiant de fichier de la ligne Teleinfo, ou -1 en cas d'erreur                                               */
/******************************************************************************************************************************/
 static gint Init_teleinfo( void )
  { struct termios oldtio;
    gchar *port = Agent_config_get_string( Agent, "port" );
    gint fd;

    if (!port)
     { Info(__func__, Agent->agent_classe, Agent->agent_tech_id, LOG_ERR, "Missing required config key 'port'");
       return -1;
     }

  fd = open(port, O_RDONLY | O_NOCTTY | O_NONBLOCK | O_CLOEXEC);
  if (fd < 0) {
    Info(__func__, Agent->agent_classe, Agent->agent_tech_id, LOG_ERR,
         "Unable to open teleinfo port '%s': %s", port, strerror(errno));
    return -1;
  }

  memset(&oldtio, 0, sizeof(oldtio));
  oldtio.c_cflag = B9600 | CS7 | CREAD | CLOCAL | PARENB;
  oldtio.c_oflag = 0;
  oldtio.c_iflag = 0;
  oldtio.c_lflag = 0;
  oldtio.c_cc[VTIME] = 0;
  oldtio.c_cc[VMIN] = 0;

  tcsetattr(fd, TCSANOW, &oldtio);
  tcflush(fd, TCIOFLUSH);

  vars->fd = fd;
  Info(__func__, Agent->agent_classe, Agent->agent_tech_id, LOG_NOTICE,
       "Teleinfo port opened: %s", port);
  return fd;
}

/*----------------------------------------------------------------------------------------------------------------------------*/
/* Processer_trame: traite une trame Teleinfo et publie les points associes                                                    */
/* Entrée: le mode standard et la structure agent                                                                              */
/* Sortie: néant                                                                                                               */
/*----------------------------------------------------------------------------------------------------------------------------*/
static void Processer_trame(gboolean mode_standard, struct ABLS_AGENT *agent)
{
  struct ABLS_TELEINFOEDF_VARS *vars = agent->vars;

  if (mode_standard == FALSE) {
    if (!strncmp(vars->buffer, "ADCO", 4)) {
      Mqtt_Send_AI(agent, vars->Adco, atof(vars->buffer + 5), TRUE);
    } else if (!strncmp(vars->buffer, "ISOUS", 5)) {
      Mqtt_Send_AI(agent, vars->Isous, atof(vars->buffer + 6), TRUE);
    } else if (!strncmp(vars->buffer, "BASE", 4)) {
      Mqtt_Send_AI(agent, vars->Base, atof(vars->buffer + 5), TRUE);
    } else if (!strncmp(vars->buffer, "HCHC", 4)) {
      Mqtt_Send_AI(agent, vars->Hchc, atof(vars->buffer + 5), TRUE);
    } else if (!strncmp(vars->buffer, "HCHP", 4)) {
      Mqtt_Send_AI(agent, vars->Hchp, atof(vars->buffer + 5), TRUE);
    } else if (!strncmp(vars->buffer, "IINST", 5)) {
      Mqtt_Send_AI(agent, vars->Iinst, atof(vars->buffer + 6), TRUE);
    } else if (!strncmp(vars->buffer, "IMAX", 4)) {
      Mqtt_Send_AI(agent, vars->Imax, atof(vars->buffer + 5), TRUE);
    } else if (!strncmp(vars->buffer, "PAPP", 4)) {
      Mqtt_Send_AI(agent, vars->Papp, atof(vars->buffer + 5), TRUE);
    }
  } else {
    gint valeur_offset = (gint)strlen(vars->buffer) + 1;

    if (g_str_has_prefix(vars->buffer, "IRMS1")) {
      Mqtt_Send_AI(agent, vars->IRMS1, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "IRMS2")) {
      Mqtt_Send_AI(agent, vars->IRMS2, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "IRMS3")) {
      Mqtt_Send_AI(agent, vars->IRMS3, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "URMS1")) {
      Mqtt_Send_AI(agent, vars->URMS1, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "URMS2")) {
      Mqtt_Send_AI(agent, vars->URMS2, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "URMS3")) {
      Mqtt_Send_AI(agent, vars->URMS3, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "PREF")) {
      Mqtt_Send_AI(agent, vars->PREF, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "PCOUP")) {
      Mqtt_Send_AI(agent, vars->PCOUP, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "SINSTS")) {
      Mqtt_Send_AI(agent, vars->SINSTS, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "SINSTS1")) {
      Mqtt_Send_AI(agent, vars->SINSTS1, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "SINSTS2")) {
      Mqtt_Send_AI(agent, vars->SINSTS2, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "SINSTS3")) {
      Mqtt_Send_AI(agent, vars->SINSTS3, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "SMAXSN")) {
      Mqtt_Send_AI(agent, vars->SMAXSN, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "SMAXSN1")) {
      Mqtt_Send_AI(agent, vars->SMAXSN1, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "SMAXSN2")) {
      Mqtt_Send_AI(agent, vars->SMAXSN2, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "SMAXSN3")) {
      Mqtt_Send_AI(agent, vars->SMAXSN3, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "UMOY1")) {
      Mqtt_Send_AI(agent, vars->UMOY1, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "UMOY2")) {
      Mqtt_Send_AI(agent, vars->UMOY2, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "UMOY3")) {
      Mqtt_Send_AI(agent, vars->UMOY3, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "NTARF")) {
      Mqtt_Send_AI(agent, vars->NTARF, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "ADSC")) {
      Mqtt_Send_AI(agent, vars->ADSC, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "EAST")) {
      Mqtt_Send_AI(agent, vars->EAST, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "EASF01")) {
      Mqtt_Send_AI(agent, vars->EASF01, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "EASF02")) {
      Mqtt_Send_AI(agent, vars->EASF02, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "EASF03")) {
      Mqtt_Send_AI(agent, vars->EASF03, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "EASF04")) {
      Mqtt_Send_AI(agent, vars->EASF04, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "EASF05")) {
      Mqtt_Send_AI(agent, vars->EASF05, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "EASF06")) {
      Mqtt_Send_AI(agent, vars->EASF06, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "EASF07")) {
      Mqtt_Send_AI(agent, vars->EASF07, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "EASF08")) {
      Mqtt_Send_AI(agent, vars->EASF08, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "EASF09")) {
      Mqtt_Send_AI(agent, vars->EASF09, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "EASF10")) {
      Mqtt_Send_AI(agent, vars->EASF10, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "EASD01")) {
      Mqtt_Send_AI(agent, vars->EASD01, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "EASD02")) {
      Mqtt_Send_AI(agent, vars->EASD02, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "EASD03")) {
      Mqtt_Send_AI(agent, vars->EASD03, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "EASD04")) {
      Mqtt_Send_AI(agent, vars->EASD04, atof(vars->buffer + valeur_offset), TRUE);
    } else if (g_str_has_prefix(vars->buffer, "PRM")) {
      Mqtt_Send_AI(agent, vars->PRM, atof(vars->buffer + valeur_offset), TRUE);
    }
  }

  vars->last_view_ds = TeleinfoEDF_top_ds();
}

/******************************************************************************************************************************/
/* TeleinfoEDF_create_mnemos: cree les mnemoniques Teleinfo du mode demande                                                   */
/* Entrée: la structure agent et le mode standard                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void TeleinfoEDF_create_mnemos( void )
  { if (Agent->vars->mode_standard == FALSE)                                                               /* Mode historique */
     { vars->Adco  = Mnemo_create_AI(Agent, "ADCO",  "Numero identification compteur", "numero", AGENT_ARCHIVE_1_JOUR);
       vars->Isous = Mnemo_create_AI(Agent, "ISOUS", "Intensite EDF souscrite", "A", AGENT_ARCHIVE_1_JOUR);
       vars->Base  = Mnemo_create_AI(Agent, "BASE",  "Index option BASE", "Wh", AGENT_ARCHIVE_1_MIN);
       vars->Hchc  = Mnemo_create_AI(Agent, "HCHC",  "Index heures creuses", "Wh", AGENT_ARCHIVE_1_MIN);
       vars->Hchp  = Mnemo_create_AI(Agent, "HCHP",  "Index heures pleines", "Wh", AGENT_ARCHIVE_1_MIN);
       vars->Iinst = Mnemo_create_AI(Agent, "IINST", "Intensite EDF instantanee", "A", AGENT_ARCHIVE_1_MIN);
       vars->Imax  = Mnemo_create_AI(Agent, "IMAX",  "Intensite EDF maximale", "A", AGENT_ARCHIVE_5_MIN);
       vars->Papp  = Mnemo_create_AI(Agent, "PAPP",  "Puissance apparente EDF consommee", "VA", AGENT_ARCHIVE_1_MIN);
     }
    else                                                                                                     /* Mode standard */
     { vars->IRMS1   = Mnemo_create_AI(Agent, "IRMS1",   "Courant efficace phase 1", "A", AGENT_ARCHIVE_1_MIN);
       vars->IRMS2   = Mnemo_create_AI(Agent, "IRMS2",   "Courant efficace phase 2", "A", AGENT_ARCHIVE_1_MIN);
       vars->IRMS3   = Mnemo_create_AI(Agent, "IRMS3",   "Courant efficace phase 3", "A", AGENT_ARCHIVE_1_MIN);
       vars->URMS1   = Mnemo_create_AI(Agent, "URMS1",   "Tension efficace phase 1", "V", AGENT_ARCHIVE_1_MIN);
       vars->URMS2   = Mnemo_create_AI(Agent, "URMS2",   "Tension efficace phase 2", "V", AGENT_ARCHIVE_1_MIN);
       vars->URMS3   = Mnemo_create_AI(Agent, "URMS3",   "Tension efficace phase 3", "V", AGENT_ARCHIVE_1_MIN);
       vars->PREF    = Mnemo_create_AI(Agent, "PREF",    "Puissance apparente souscrite", "kVA", AGENT_ARCHIVE_1_JOUR);
       vars->PCOUP   = Mnemo_create_AI(Agent, "PCOUP",   "Puissance apparente de coupure", "kVA", AGENT_ARCHIVE_1_JOUR);
       vars->SINSTS  = Mnemo_create_AI(Agent, "SINSTS",  "Puissance apparente instantanee soutiree", "VA", AGENT_ARCHIVE_1_MIN);
       vars->SINSTS1 = Mnemo_create_AI(Agent, "SINSTS1", "Puissance apparente instantanee soutiree phase 1", "VA", AGENT_ARCHIVE_1_MIN);
       vars->SINSTS2 = Mnemo_create_AI(Agent, "SINSTS2", "Puissance apparente instantanee soutiree phase 2", "VA", AGENT_ARCHIVE_1_MIN);
       vars->SINSTS3 = Mnemo_create_AI(Agent, "SINSTS3", "Puissance apparente instantanee soutiree phase 3", "VA", AGENT_ARCHIVE_1_MIN);
       vars->SMAXSN  = Mnemo_create_AI(Agent, "SMAXSN",  "Puissance apparente max soutiree n", "VA", AGENT_ARCHIVE_1_HEURE);
       vars->SMAXSN1 = Mnemo_create_AI(Agent, "SMAXSN1", "Puissance apparente max soutiree n phase 1", "VA", AGENT_ARCHIVE_1_HEURE);
       vars->SMAXSN2 = Mnemo_create_AI(Agent, "SMAXSN2", "Puissance apparente max soutiree n phase 2", "VA", AGENT_ARCHIVE_1_HEURE);
       vars->SMAXSN3 = Mnemo_create_AI(Agent, "SMAXSN3", "Puissance apparente max soutiree n phase 3", "VA", AGENT_ARCHIVE_1_HEURE);
       vars->UMOY1   = Mnemo_create_AI(Agent, "UMOY1",   "Tension moyenne phase 1", "V", AGENT_ARCHIVE_1_HEURE);
       vars->UMOY2   = Mnemo_create_AI(Agent, "UMOY2",   "Tension moyenne phase 2", "V", AGENT_ARCHIVE_1_HEURE);
       vars->UMOY3   = Mnemo_create_AI(Agent, "UMOY3",   "Tension moyenne phase 3", "V", AGENT_ARCHIVE_1_HEURE);
       vars->NTARF   = Mnemo_create_AI(Agent, "NTARF",   "Numero index tarifaire en cours", "", AGENT_ARCHIVE_1_HEURE);
       vars->ADSC    = Mnemo_create_AI(Agent, "ADSC",    "Adresse secondaire compteur", "", AGENT_ARCHIVE_1_JOUR);
       vars->EAST    = Mnemo_create_AI(Agent, "EAST",    "Energie active soutiree totale", "Wh", AGENT_ARCHIVE_1_MIN);
       vars->EASF01  = Mnemo_create_AI(Agent, "EASF01",  "Energie active fournisseur index 01", "Wh", AGENT_ARCHIVE_5_MIN);
       vars->EASF02  = Mnemo_create_AI(Agent, "EASF02",  "Energie active fournisseur index 02", "Wh", AGENT_ARCHIVE_5_MIN);
       vars->EASF03  = Mnemo_create_AI(Agent, "EASF03",  "Energie active fournisseur index 03", "Wh", AGENT_ARCHIVE_5_MIN);
       vars->EASF04  = Mnemo_create_AI(Agent, "EASF04",  "Energie active fournisseur index 04", "Wh", AGENT_ARCHIVE_5_MIN);
       vars->EASF05  = Mnemo_create_AI(Agent, "EASF05",  "Energie active fournisseur index 05", "Wh", AGENT_ARCHIVE_5_MIN);
       vars->EASF06  = Mnemo_create_AI(Agent, "EASF06",  "Energie active fournisseur index 06", "Wh", AGENT_ARCHIVE_5_MIN);
       vars->EASF07  = Mnemo_create_AI(Agent, "EASF07",  "Energie active fournisseur index 07", "Wh", AGENT_ARCHIVE_5_MIN);
       vars->EASF08  = Mnemo_create_AI(Agent, "EASF08",  "Energie active fournisseur index 08", "Wh", AGENT_ARCHIVE_5_MIN);
       vars->EASF09  = Mnemo_create_AI(Agent, "EASF09",  "Energie active fournisseur index 09", "Wh", AGENT_ARCHIVE_5_MIN);
       vars->EASF10  = Mnemo_create_AI(Agent, "EASF10",  "Energie active fournisseur index 10", "Wh", AGENT_ARCHIVE_5_MIN);
       vars->EASD01  = Mnemo_create_AI(Agent, "EASD01",  "Energie active distributeur index 01", "Wh", AGENT_ARCHIVE_5_MIN);
       vars->EASD02  = Mnemo_create_AI(Agent, "EASD02",  "Energie active distributeur index 02", "Wh", AGENT_ARCHIVE_5_MIN);
       vars->EASD03  = Mnemo_create_AI(Agent, "EASD03",  "Energie active distributeur index 03", "Wh", AGENT_ARCHIVE_5_MIN);
       vars->EASD04  = Mnemo_create_AI(Agent, "EASD04",  "Energie active distributeur index 04", "Wh", AGENT_ARCHIVE_5_MIN);
       vars->PRM     = Mnemo_create_AI(Agent, "PRM",     "Numero compteur", "", AGENT_ARCHIVE_1_JOUR);
     }
  }
/******************************************************************************************************************************/
/* main: point d'entree du thread Teleinfo EDF                                                                                */
/* Entrée: les arguments du programme                                                                                         */
/* Sortie: code de retour du processus                                                                                        */
/******************************************************************************************************************************/
 gint main(gint argc, gchar *argv[])
  { Config_add_parameter ( "port", "port (/dev/)", "Port of the teleinfo device", CONFIG_STRING );
    Config_add_parameter ( "standard", "mode standard", "Mode standard for the teleinfo device", CONFIG_BOOL );
    Agent = Agent_init(argv[0], "teleinfoedf", ABLS_AGENT_TELEINFOEDF_VERSION, sizeof(struct ABLS_TELEINFOEDF_VARS), argc, argv);
    Agent_vars = Agent->vars;

    gchar *port = Agent_config_get_string( Agent, "port" );
    if (!port)
     { Info(__func__, Agent->agent_classe, Agent->agent_tech_id, LOG_ERR, "Missing required config key 'port'");
       Agent_end();
     }

    Agent_vars->mode = TINFO_RETRYING;
    Agent_vars->date_next_retry = 0;
    Agent_vars->nbr_octet_lu = 0;
    Agent_vars->last_view_ds = TeleinfoEDF_top_ds();
    memset(Agent_vars->buffer, 0, sizeof(Agent_vars->buffer));

    Agent_vars->mode_standard = Agent_config_get_bool(Agent, "standard");
    TeleinfoEDF_create_points(Agent, Agent_vars->mode_standard);

    Mqtt_subscribe(Agent->mqtt_local, "SET_DO/%s/#", Agent->agent_tech_id);
    Mqtt_subscribe(Agent->mqtt_local, "SET_AO/%s/#", Agent->agent_tech_id);
    Mqtt_subscribe(Agent->mqtt_local, "SYNC_INPUT/%s", Agent->agent_tech_id);

    Agent_is_ready (Agent);

  while (Agent->Agent_run == AGENT_IS_RUNNING) {
    guint now_ds;
    fd_set fdselect;
    struct timeval tv;
    gint retval;

    Agent_loop(agent);

    while (TRUE) {
      JsonNode *mqtt_local_message = Mqtt_get_message(agent->mqtt_local);
      if (!mqtt_local_message) {
        break;
      }

      if (Mqtt_topic_is(mqtt_local_message, 2, "SYNC_INPUT", agent->agent_tech_id)) {
        Info(__func__, agent->agent_classe, agent->agent_tech_id, LOG_INFO,
             "SYNC_INPUT requested");
      }

      Json_unref(mqtt_local_message);
    }

    now_ds = TeleinfoEDF_top_ds();
    if (vars->mode == TINFO_WAIT_BEFORE_RETRY) {
      if (vars->date_next_retry <= now_ds) {
        vars->mode = TINFO_RETRYING;
        vars->date_next_retry = 0;
        Info(__func__, agent->agent_classe, agent->agent_tech_id, LOG_NOTICE,
             "Retrying teleinfo connection");
      }
    } else if (vars->mode == TINFO_RETRYING) {
      if (Init_teleinfo(agent) < 0) {
        Info(__func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR,
             "Init teleinfo failed, retrying in %us", (TINFO_RETRY_DELAI_DS / 10));
        vars->mode = TINFO_WAIT_BEFORE_RETRY;
        vars->date_next_retry = now_ds + TINFO_RETRY_DELAI_DS;
      } else {
        vars->mode = TINFO_CONNECTED;
        Info(__func__, agent->agent_classe, agent->agent_tech_id, LOG_INFO,
             "Teleinfo connected (fd=%d)", vars->fd);
      }
    }

    if (vars->mode != TINFO_CONNECTED) {
      continue;
    }

    FD_ZERO(&fdselect);
    FD_SET(vars->fd, &fdselect);
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    retval = select(vars->fd + 1, &fdselect, NULL, NULL, &tv);

    if (retval >= 0 && FD_ISSET(vars->fd, &fdselect)) {
      gint cpt = read(vars->fd, ((unsigned char *)vars->buffer) + vars->nbr_octet_lu, 1);
      if (cpt > 0) {
        if (vars->buffer[vars->nbr_octet_lu] == '\n') {
          vars->buffer[vars->nbr_octet_lu] = 0;
          Processer_trame(mode_standard, agent);
          vars->nbr_octet_lu = 0;
          memset(vars->buffer, 0, sizeof(vars->buffer));
          Agent_send_comm_to_master(agent, TRUE);
        } else if (vars->nbr_octet_lu + cpt < TAILLE_BUFFER_TELEINFO) {
          if (mode_standard) {
            if (vars->buffer[vars->nbr_octet_lu] == '\t') {
              vars->buffer[vars->nbr_octet_lu] = 0;
            } else if (vars->buffer[vars->nbr_octet_lu] == '\r') {
              vars->buffer[vars->nbr_octet_lu] = 0;
            }
          }
          vars->nbr_octet_lu += cpt;
        } else {
          vars->nbr_octet_lu = 0;
          memset(vars->buffer, 0, sizeof(vars->buffer));
          Info(__func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR,
               "Buffer overflow, dropping frame");
        }
      }
    }

    if ((now_ds % 50) == 0) {
      gboolean closing = FALSE;
      struct stat buf;

      if (fstat(vars->fd, &buf) == -1) {
        Info(__func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR,
             "fstat failed (%s), reconnect in %us", strerror(errno), (TINFO_RETRY_DELAI_DS / 10));
        closing = TRUE;
      } else if (buf.st_nlink < 1) {
        Info(__func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR,
             "USB device disappeared, reconnect in %us", (TINFO_RETRY_DELAI_DS / 10));
        closing = TRUE;
      }

      if (closing == TRUE) {
        close(vars->fd);
        vars->fd = -1;
        vars->mode = TINFO_WAIT_BEFORE_RETRY;
        vars->date_next_retry = now_ds + TINFO_RETRY_DELAI_DS;
        Agent_send_comm_to_master(agent, FALSE);
      }
    }
  }

  if (vars->fd >= 0) {
    close(vars->fd);
    vars->fd = -1;
  }

  Agent_end(agent);
  return 0;
}
/*----------------------------------------------------------------------------------------------------------------------------*/