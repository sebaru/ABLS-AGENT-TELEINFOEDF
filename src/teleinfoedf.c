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
    if (!port)
     { Info(__func__, Agent->agent_classe, Agent->agent_tech_id, LOG_ERR, "Missing required config key 'port'");
       return(-1);
     }

    gint fd = open(port, O_RDONLY | O_NOCTTY | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0) {
      Info(__func__, Agent->agent_classe, Agent->agent_tech_id, LOG_ERR,
           "Unable to open teleinfo port '%s': %s", port, strerror(errno));
      return(-1);
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

    Agent_vars->fd = fd;
    Info(__func__, Agent->agent_classe, Agent->agent_tech_id, LOG_NOTICE, "Teleinfo port opened: %s", port);
    return(fd);
  }
/******************************************************************************************************************************/
/* Processer_trame: traite une trame Teleinfo et publie les points associes                                                   */
/* Entrée: le mode standard et la structure agent                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Processer_trame ( void )
  { if (Agent_vars->mode_standard == FALSE) /* Mode historique */
     {      if (!strncmp(Agent_vars->buffer, "ADCO", 4))  { Mqtt_Send_AI(Agent, Agent_vars->Adco,  atof(Agent_vars->buffer + 5), TRUE); }
       else if (!strncmp(Agent_vars->buffer, "ISOUS", 5)) { Mqtt_Send_AI(Agent, Agent_vars->Isous, atof(Agent_vars->buffer + 6), TRUE); }
       else if (!strncmp(Agent_vars->buffer, "BASE", 4))  { Mqtt_Send_AI(Agent, Agent_vars->Base,  atof(Agent_vars->buffer + 5), TRUE); }
       else if (!strncmp(Agent_vars->buffer, "HCHC", 4))  { Mqtt_Send_AI(Agent, Agent_vars->Hchc,  atof(Agent_vars->buffer + 5), TRUE); }
       else if (!strncmp(Agent_vars->buffer, "HCHP", 4))  { Mqtt_Send_AI(Agent, Agent_vars->Hchp,  atof(Agent_vars->buffer + 5), TRUE); }
       else if (!strncmp(Agent_vars->buffer, "IINST", 5)) { Mqtt_Send_AI(Agent, Agent_vars->Iinst, atof(Agent_vars->buffer + 6), TRUE); }
       else if (!strncmp(Agent_vars->buffer, "IMAX", 4))  { Mqtt_Send_AI(Agent, Agent_vars->Imax,  atof(Agent_vars->buffer + 5), TRUE); }
       else if (!strncmp(Agent_vars->buffer, "PAPP", 4))  { Mqtt_Send_AI(Agent, Agent_vars->Papp,  atof(Agent_vars->buffer + 5), TRUE); }
     }
    else /* Mode standard */
     { gint valeur_offset = (gint)strlen(Agent_vars->buffer) + 1;
            if (g_str_has_prefix(Agent_vars->buffer, "IRMS1"))   { Mqtt_Send_AI(Agent, Agent_vars->IRMS1,   atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "IRMS2"))   { Mqtt_Send_AI(Agent, Agent_vars->IRMS2,   atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "IRMS3"))   { Mqtt_Send_AI(Agent, Agent_vars->IRMS3,   atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "URMS1"))   { Mqtt_Send_AI(Agent, Agent_vars->URMS1,   atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "URMS2"))   { Mqtt_Send_AI(Agent, Agent_vars->URMS2,   atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "URMS3"))   { Mqtt_Send_AI(Agent, Agent_vars->URMS3,   atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "PREF"))    { Mqtt_Send_AI(Agent, Agent_vars->PREF,    atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "PCOUP"))   { Mqtt_Send_AI(Agent, Agent_vars->PCOUP,   atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "SINSTS"))  { Mqtt_Send_AI(Agent, Agent_vars->SINSTS,  atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "SINSTS1")) { Mqtt_Send_AI(Agent, Agent_vars->SINSTS1, atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "SINSTS2")) { Mqtt_Send_AI(Agent, Agent_vars->SINSTS2, atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "SINSTS3")) { Mqtt_Send_AI(Agent, Agent_vars->SINSTS3, atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "SMAXSN"))  { Mqtt_Send_AI(Agent, Agent_vars->SMAXSN,  atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "SMAXSN1")) { Mqtt_Send_AI(Agent, Agent_vars->SMAXSN1, atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "SMAXSN2")) { Mqtt_Send_AI(Agent, Agent_vars->SMAXSN2, atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "SMAXSN3")) { Mqtt_Send_AI(Agent, Agent_vars->SMAXSN3, atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "UMOY1"))   { Mqtt_Send_AI(Agent, Agent_vars->UMOY1,   atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "UMOY2"))   { Mqtt_Send_AI(Agent, Agent_vars->UMOY2,   atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "UMOY3"))   { Mqtt_Send_AI(Agent, Agent_vars->UMOY3,   atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "NTARF"))   { Mqtt_Send_AI(Agent, Agent_vars->NTARF,   atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "ADSC"))    { Mqtt_Send_AI(Agent, Agent_vars->ADSC,    atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "EAST"))    { Mqtt_Send_AI(Agent, Agent_vars->EAST,    atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "EASF01"))  { Mqtt_Send_AI(Agent, Agent_vars->EASF01,  atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "EASF02"))  { Mqtt_Send_AI(Agent, Agent_vars->EASF02,  atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "EASF03"))  { Mqtt_Send_AI(Agent, Agent_vars->EASF03,  atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "EASF04"))  { Mqtt_Send_AI(Agent, Agent_vars->EASF04,  atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "EASF05"))  { Mqtt_Send_AI(Agent, Agent_vars->EASF05,  atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "EASF06"))  { Mqtt_Send_AI(Agent, Agent_vars->EASF06,  atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "EASF07"))  { Mqtt_Send_AI(Agent, Agent_vars->EASF07,  atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "EASF08"))  { Mqtt_Send_AI(Agent, Agent_vars->EASF08,  atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "EASF09"))  { Mqtt_Send_AI(Agent, Agent_vars->EASF09,  atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "EASF10"))  { Mqtt_Send_AI(Agent, Agent_vars->EASF10,  atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "EASD01"))  { Mqtt_Send_AI(Agent, Agent_vars->EASD01,  atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "EASD02"))  { Mqtt_Send_AI(Agent, Agent_vars->EASD02,  atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "EASD03"))  { Mqtt_Send_AI(Agent, Agent_vars->EASD03,  atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "EASD04"))  { Mqtt_Send_AI(Agent, Agent_vars->EASD04,  atof(Agent_vars->buffer + valeur_offset), TRUE); }
       else if (g_str_has_prefix(Agent_vars->buffer, "PRM"))     { Mqtt_Send_AI(Agent, Agent_vars->PRM,     atof(Agent_vars->buffer + valeur_offset), TRUE); }
     }
  }
/******************************************************************************************************************************/
/* TeleinfoEDF_create_mnemos: cree les mnemoniques Teleinfo du mode demande                                                   */
/* Entrée: la structure agent et le mode standard                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void TeleinfoEDF_create_IO( void )
  { if (Agent_vars->mode_standard == FALSE)                                                                /* Mode historique */
     { Agent_vars->Adco  = Mnemo_create_AI(Agent, "ADCO",  "Numero identification compteur", "numero", AGENT_ARCHIVE_1_JOUR);
       Agent_vars->Isous = Mnemo_create_AI(Agent, "ISOUS", "Intensite EDF souscrite", "A", AGENT_ARCHIVE_1_JOUR);
       Agent_vars->Base  = Mnemo_create_AI(Agent, "BASE",  "Index option BASE", "Wh", AGENT_ARCHIVE_1_MIN);
       Agent_vars->Hchc  = Mnemo_create_AI(Agent, "HCHC",  "Index heures creuses", "Wh", AGENT_ARCHIVE_1_MIN);
       Agent_vars->Hchp  = Mnemo_create_AI(Agent, "HCHP",  "Index heures pleines", "Wh", AGENT_ARCHIVE_1_MIN);
       Agent_vars->Iinst = Mnemo_create_AI(Agent, "IINST", "Intensite EDF instantanee", "A", AGENT_ARCHIVE_1_MIN);
       Agent_vars->Imax  = Mnemo_create_AI(Agent, "IMAX",  "Intensite EDF maximale", "A", AGENT_ARCHIVE_5_MIN);
       Agent_vars->Papp  = Mnemo_create_AI(Agent, "PAPP",  "Puissance apparente EDF consommee", "VA", AGENT_ARCHIVE_1_MIN);
     }
    else                                                                                                     /* Mode standard */
     { Agent_vars->IRMS1   = Mnemo_create_AI(Agent, "IRMS1",   "Courant efficace phase 1", "A", AGENT_ARCHIVE_1_MIN);
       Agent_vars->IRMS2   = Mnemo_create_AI(Agent, "IRMS2",   "Courant efficace phase 2", "A", AGENT_ARCHIVE_1_MIN);
       Agent_vars->IRMS3   = Mnemo_create_AI(Agent, "IRMS3",   "Courant efficace phase 3", "A", AGENT_ARCHIVE_1_MIN);
       Agent_vars->URMS1   = Mnemo_create_AI(Agent, "URMS1",   "Tension efficace phase 1", "V", AGENT_ARCHIVE_1_MIN);
       Agent_vars->URMS2   = Mnemo_create_AI(Agent, "URMS2",   "Tension efficace phase 2", "V", AGENT_ARCHIVE_1_MIN);
       Agent_vars->URMS3   = Mnemo_create_AI(Agent, "URMS3",   "Tension efficace phase 3", "V", AGENT_ARCHIVE_1_MIN);
       Agent_vars->PREF    = Mnemo_create_AI(Agent, "PREF",    "Puissance apparente souscrite", "kVA", AGENT_ARCHIVE_1_JOUR);
       Agent_vars->PCOUP   = Mnemo_create_AI(Agent, "PCOUP",   "Puissance apparente de coupure", "kVA", AGENT_ARCHIVE_1_JOUR);
       Agent_vars->SINSTS  = Mnemo_create_AI(Agent, "SINSTS",  "Puissance apparente instantanee soutiree", "VA", AGENT_ARCHIVE_1_MIN);
       Agent_vars->SINSTS1 = Mnemo_create_AI(Agent, "SINSTS1", "Puissance apparente instantanee soutiree phase 1", "VA", AGENT_ARCHIVE_1_MIN);
       Agent_vars->SINSTS2 = Mnemo_create_AI(Agent, "SINSTS2", "Puissance apparente instantanee soutiree phase 2", "VA", AGENT_ARCHIVE_1_MIN);
       Agent_vars->SINSTS3 = Mnemo_create_AI(Agent, "SINSTS3", "Puissance apparente instantanee soutiree phase 3", "VA", AGENT_ARCHIVE_1_MIN);
       Agent_vars->SMAXSN  = Mnemo_create_AI(Agent, "SMAXSN",  "Puissance apparente max soutiree n", "VA", AGENT_ARCHIVE_1_HEURE);
       Agent_vars->SMAXSN1 = Mnemo_create_AI(Agent, "SMAXSN1", "Puissance apparente max soutiree n phase 1", "VA", AGENT_ARCHIVE_1_HEURE);
       Agent_vars->SMAXSN2 = Mnemo_create_AI(Agent, "SMAXSN2", "Puissance apparente max soutiree n phase 2", "VA", AGENT_ARCHIVE_1_HEURE);
       Agent_vars->SMAXSN3 = Mnemo_create_AI(Agent, "SMAXSN3", "Puissance apparente max soutiree n phase 3", "VA", AGENT_ARCHIVE_1_HEURE);
       Agent_vars->UMOY1   = Mnemo_create_AI(Agent, "UMOY1",   "Tension moyenne phase 1", "V", AGENT_ARCHIVE_1_HEURE);
       Agent_vars->UMOY2   = Mnemo_create_AI(Agent, "UMOY2",   "Tension moyenne phase 2", "V", AGENT_ARCHIVE_1_HEURE);
       Agent_vars->UMOY3   = Mnemo_create_AI(Agent, "UMOY3",   "Tension moyenne phase 3", "V", AGENT_ARCHIVE_1_HEURE);
       Agent_vars->NTARF   = Mnemo_create_AI(Agent, "NTARF",   "Numero index tarifaire en cours", "", AGENT_ARCHIVE_1_HEURE);
       Agent_vars->ADSC    = Mnemo_create_AI(Agent, "ADSC",    "Adresse secondaire compteur", "", AGENT_ARCHIVE_1_JOUR);
       Agent_vars->EAST    = Mnemo_create_AI(Agent, "EAST",    "Energie active soutiree totale", "Wh", AGENT_ARCHIVE_1_MIN);
       Agent_vars->EASF01  = Mnemo_create_AI(Agent, "EASF01",  "Energie active fournisseur index 01", "Wh", AGENT_ARCHIVE_5_MIN);
       Agent_vars->EASF02  = Mnemo_create_AI(Agent, "EASF02",  "Energie active fournisseur index 02", "Wh", AGENT_ARCHIVE_5_MIN);
       Agent_vars->EASF03  = Mnemo_create_AI(Agent, "EASF03",  "Energie active fournisseur index 03", "Wh", AGENT_ARCHIVE_5_MIN);
       Agent_vars->EASF04  = Mnemo_create_AI(Agent, "EASF04",  "Energie active fournisseur index 04", "Wh", AGENT_ARCHIVE_5_MIN);
       Agent_vars->EASF05  = Mnemo_create_AI(Agent, "EASF05",  "Energie active fournisseur index 05", "Wh", AGENT_ARCHIVE_5_MIN);
       Agent_vars->EASF06  = Mnemo_create_AI(Agent, "EASF06",  "Energie active fournisseur index 06", "Wh", AGENT_ARCHIVE_5_MIN);
       Agent_vars->EASF07  = Mnemo_create_AI(Agent, "EASF07",  "Energie active fournisseur index 07", "Wh", AGENT_ARCHIVE_5_MIN);
       Agent_vars->EASF08  = Mnemo_create_AI(Agent, "EASF08",  "Energie active fournisseur index 08", "Wh", AGENT_ARCHIVE_5_MIN);
       Agent_vars->EASF09  = Mnemo_create_AI(Agent, "EASF09",  "Energie active fournisseur index 09", "Wh", AGENT_ARCHIVE_5_MIN);
       Agent_vars->EASF10  = Mnemo_create_AI(Agent, "EASF10",  "Energie active fournisseur index 10", "Wh", AGENT_ARCHIVE_5_MIN);
       Agent_vars->EASD01  = Mnemo_create_AI(Agent, "EASD01",  "Energie active distributeur index 01", "Wh", AGENT_ARCHIVE_5_MIN);
       Agent_vars->EASD02  = Mnemo_create_AI(Agent, "EASD02",  "Energie active distributeur index 02", "Wh", AGENT_ARCHIVE_5_MIN);
       Agent_vars->EASD03  = Mnemo_create_AI(Agent, "EASD03",  "Energie active distributeur index 03", "Wh", AGENT_ARCHIVE_5_MIN);
       Agent_vars->EASD04  = Mnemo_create_AI(Agent, "EASD04",  "Energie active distributeur index 04", "Wh", AGENT_ARCHIVE_5_MIN);
       Agent_vars->PRM     = Mnemo_create_AI(Agent, "PRM",     "Numero compteur", "", AGENT_ARCHIVE_1_JOUR);
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
       Agent_end(Agent);
     }

    Agent_vars->mode                = TINFO_RETRYING;
    Agent_vars->next_retry_top      = 0;
    Agent_vars->next_file_check_top = 0;
    Agent_vars->mode_standard       = Agent_config_get_bool(Agent, "standard");
    Agent_vars->nbr_octet_lu        = 0;
    memset(Agent_vars->buffer, 0, sizeof(Agent_vars->buffer));

    TeleinfoEDF_create_IO();

    Mqtt_subscribe(Agent->mqtt_local, "SET_DO/%s/#", Agent->agent_tech_id);
    Mqtt_subscribe(Agent->mqtt_local, "SET_AO/%s/#", Agent->agent_tech_id);
    Mqtt_subscribe(Agent->mqtt_local, "SYNC_INPUT/%s", Agent->agent_tech_id);

    Agent_is_ready (Agent);

    while (Agent->Agent_run == AGENT_IS_RUNNING)
     { Agent_loop ( Agent );
/****************************************************** Ecoute du master ******************************************************/
       JsonNode *mqtt_local_message;
       while ( (mqtt_local_message = Agent_get_mqtt_local_message( Agent )) != NULL )
        { Json_unref ( mqtt_local_message );
        }
/****************************************************** Ecoute de l'api *******************************************************/
       JsonNode *mqtt_api_message;
       while ( (mqtt_api_message = Agent_get_mqtt_api_message ( Agent ) ) != NULL )
        { if ( Mqtt_topic_is ( mqtt_api_message, 4, "+", "AGENT", Agent->agent_tech_id, "TEST" ) )
          { Info(__func__, Agent->agent_classe, Agent->agent_tech_id, LOG_NOTICE, "Test from Master."); }
         Json_unref (mqtt_api_message);
        }
/************************************************* Traitement opérationnel ****************************************************/
       if (Agent_vars->mode == TINFO_WAIT_BEFORE_RETRY)
        { if (Agent_vars->next_retry_top <= Agent->Top)
           { Agent_vars->mode = TINFO_RETRYING;
             Agent_vars->next_retry_top = 0;
             Info( __func__, Agent->agent_classe, Agent->agent_tech_id, LOG_NOTICE, "Retrying teleinfo connection" );
           }
        }
       else if (Agent_vars->mode == TINFO_RETRYING)
        { if (Init_teleinfo() < 0)
           { Info( __func__, Agent->agent_classe, Agent->agent_tech_id, LOG_ERR,
                   "Init teleinfo failed, retrying in %us", (TINFO_RETRY_DELAI / 10) );
             Agent_vars->mode = TINFO_WAIT_BEFORE_RETRY;
             Agent_vars->next_retry_top = Agent->Top + TINFO_RETRY_DELAI;
           }
          else
           { Agent_vars->mode = TINFO_CONNECTED;
             Info( __func__, Agent->agent_classe, Agent->agent_tech_id, LOG_INFO, "Teleinfo connected (fd=%d)", Agent_vars->fd );
           }
        }

       if (Agent_vars->mode != TINFO_CONNECTED) { sleep(1); continue; }

       struct timeval tv;
       fd_set fdselect;
       FD_ZERO(&fdselect);
       FD_SET(Agent_vars->fd, &fdselect);
       tv.tv_sec = 1;
       tv.tv_usec = 0;
       gint retval = select(Agent_vars->fd + 1, &fdselect, NULL, NULL, &tv);                        /* Attente d'un caractere */

       if (retval >= 0 && FD_ISSET(Agent_vars->fd, &fdselect))
        { gint cpt = read(Agent_vars->fd, ((unsigned char *)Agent_vars->buffer) + Agent_vars->nbr_octet_lu, 1);
          if (cpt > 0)
           { if (Agent_vars->buffer[Agent_vars->nbr_octet_lu] == '\n')                                           /* Process de la trame ? */
              { Agent_vars->buffer[Agent_vars->nbr_octet_lu] = 0;                                               /* Caractère fin de trame */
                Processer_trame();
                Agent_vars->nbr_octet_lu = 0;
                memset(Agent_vars->buffer, 0, sizeof(Agent_vars->buffer));
                Agent_send_comm_to_master(Agent, TRUE);
              }
             else if (Agent_vars->nbr_octet_lu + cpt < TAILLE_BUFFER_TELEINFO)                  /* Encore en dessous de la limite ? */
              { if (Agent_vars->mode_standard)                                                              /* Change \t and \r into null */
                 {      if (Agent_vars->buffer[Agent_vars->nbr_octet_lu] == '\t') { Agent_vars->buffer[Agent_vars->nbr_octet_lu] = 0; }
                   else if (Agent_vars->buffer[Agent_vars->nbr_octet_lu] == '\r') { Agent_vars->buffer[Agent_vars->nbr_octet_lu] = 0; }
                 }
                Agent_vars->nbr_octet_lu += cpt;                                               /* Preparation du prochain caractere */
              }
             else
              { Agent_vars->nbr_octet_lu = 0;
                memset(Agent_vars->buffer, 0, sizeof(Agent_vars->buffer));
                Info( __func__, Agent->agent_classe, Agent->agent_tech_id, LOG_ERR, "Buffer overflow, dropping frame" );
              }
           }
        }

       if ( Agent_vars->next_file_check_top <= Agent->Top )      /* Si pb FD ou débranchage USB -> test toutes les 10 secondes */
        { gboolean closing_needed = FALSE;
          struct stat buf;

          if (fstat(Agent_vars->fd, &buf) == -1)
           { Info( __func__, Agent->agent_classe, Agent->agent_tech_id, LOG_ERR,
                   "fstat failed (%s), reconnect in %us", strerror(errno), (TINFO_RETRY_DELAI / 10) );
             closing_needed = TRUE;
           }
          else if (buf.st_nlink < 1)
           { Info( __func__, Agent->agent_classe, Agent->agent_tech_id, LOG_ERR,
                   "USB device disappeared, reconnect in %us", (TINFO_RETRY_DELAI / 10) );
             closing_needed = TRUE;
           }

          if (closing_needed == TRUE)
           { close(Agent_vars->fd);
             Agent_vars->fd = -1;
             Agent_vars->mode = TINFO_WAIT_BEFORE_RETRY;
             Agent_vars->next_retry_top = Agent->Top + TINFO_RETRY_DELAI;
             Agent_send_comm_to_master(Agent, FALSE);
           }
          Agent_vars->next_file_check_top = Agent->Top + 100;                                  /* Test toutes les 10 secondes */
        }
     }

  if (Agent_vars->fd >= 0) { close(Agent_vars->fd); Agent_vars->fd = -1; }

  Agent_end(Agent);
  return 0;
}
/*----------------------------------------------------------------------------------------------------------------------------*/