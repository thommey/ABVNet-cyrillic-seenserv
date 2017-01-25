/*Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "atheme.h"
#include "seenserv.h"

DECLARE_MODULE_V1("seenserv/seen", false, _modinit, _moddeinit, PACKAGE_STRING, "Deyan Hristanov <gameoverbg@gmail.com>");

static void seen_cmd_seen(sourceinfo_t *si, int parc, char *parv[]);
static void seen_cmd_seennick(sourceinfo_t *si, int parc, char *parv[]);

command_t seen_seen = { "SEEN", N_("Gives information on a nick/hostmask"), AC_NONE, 2, seen_cmd_seen, {.path = "seenserv/seen"} };

command_t seen_seennick = { "SEENNICK", N_("Gives information on specific nick only"), AC_NONE, 2, seen_cmd_seennick, {.path = "seenserv/seennick"} };

void _modinit(module_t *m)
{
	service_named_bind_command("seenserv", &seen_seen);
	service_named_bind_command("seenserv", &seen_seennick);
}

void _moddeinit(module_unload_intent_t intent)
{

	service_named_unbind_command("seenserv", &seen_seen);
	service_named_unbind_command("seenserv", &seen_seennick);
}

static void seen_cmd_seen(sourceinfo_t *si, int parc, char *parv[])
{
	seen_db_t dbs;
	char *seen;
	char *snick = parv[0];
	char *option = parv[1];
	if (si->c && parc >= 2)
	{
	option = strtok(parv[1], " ");
	snick = option;
	option = strtok(NULL, " ");	
	} else {
	option = strtok(parv[1], " ");
	}
	if (parc < 1)
	{
	 command_fail(si, fault_needmoreparams, _("Syntax: Syntax: \2SEEN <nick|hostmask>"));
	 command_fail(si, fault_needmoreparams, _("Type \2/msg SeenServ HELP SEEN\2 for more information"));
	 return;
	}

	int c;
	  dbs.utf8str = 0;
	if (match_utf_cyr(snick))
	{
	  snick = utf8_to_cp1251(snick);
	  dbs.utf8str = 1;
	}

	dbs.method = 1;
    dbs.recors = 1;
	dbs.starting = 1;
	dbs.exactm = false;
while (option != NULL)
{	
if (option)
{
	if(option[0] == 'r')
	{
	   option++;
	  if (strlen(option) == 1) 
	  {
		 if(isdigit(option[0]))
		 if(option[0] <= '1' && '9' > option[0])
		 {
	         command_fail(si, fault_needmoreparams, _("Syntax: \2SEEN <nick|hostmask> rX\2 where \2X>1 X<=10"));
	         return;			 
		 }
          dbs.recors = atoi(option);		 
	  } else if(strlen(option) == 2) {
		 if(option[0] != '1' || option[1] != '0')
		 {
	         command_fail(si, fault_needmoreparams, _("Syntax: \2SEEN <nick|hostmask> rX\2 where \2X>1 X<=10"));
	         return;			 
		 }
          dbs.recors = atoi(option);		 
	  }   
	 }

	if(option[0] == 's')
	{
	  option++;
	  for(c=0;strlen(option) > c; c++)
	    if(!isdigit(option[c]))
		  {
			 command_fail(si, fault_needmoreparams, _("Syntax: \2SEEN <nick|hostmask> sX\2 where X number"));
	         return;  
	     }
		dbs.starting = atoi(option);
	}

	if(option[0] == 'c')
	{
	 dbs.exactm = true;	
	}		
  }
 option = strtok(NULL, " ");  
}
      int count=0, match=0,i=0;
      while(snick[i] != '\0')
      {
		  if ((unsigned char)snick[i] >= 0x41 && (unsigned char)snick[i] <= 0x7d || (unsigned char)snick[i] >=0x30 && (unsigned char)snick[i] <= 0x39 || (unsigned char)snick[i] >= 0xc0 && (unsigned char)snick[i] <= 0xff || (unsigned char)snick[i] == 0x2d)	  
		  count++;
		  
		  if ((unsigned char)snick[i] == 0x2a || (unsigned char)snick[i] == 0x3f)
		  match=1;
		  
		  i++;
	  }
	  
	  if (count < 4 && match == 1)
	  {
		notice("SeenServ", si->su->nick, "\2%s\2 is too general, please include at least 4 non-wildcard characters.", snick);
	  } else {
	   strcpy(dbs.purp, "R");
	   dbs.host = snick;
	   seen_db_t *cptr = &dbs;
	   seen_write_to_db(si->su->nick, cptr);		  
	  }
}

static void seen_cmd_seennick(sourceinfo_t *si, int parc, char *parv[])
{
	seen_db_t dbs;
	char *seen;
	char *snick = parv[0];
	char *option = parv[1];		
	if (si->c && parc >= 2)
	{
	option = strtok(parv[1], " ");
	snick = option;
	option = strtok(NULL, " ");	
	} else {
	option = strtok(parv[1], " ");
	}
	if (parc < 1)
	{
	 command_fail(si, fault_needmoreparams, _("Syntax: Syntax: \2SEENNICK <nick>"));
	 command_fail(si, fault_needmoreparams, _("Type \2/msg SeenServ HELP SEENNICK\2 for more information"));
	 return;
	}

	int c;
      	dbs.utf8str = 0;
	if (match_utf_cyr(snick))
	{
	  snick = utf8_to_cp1251(snick);
	  dbs.utf8str = 1;
	}
		
	dbs.method = 0;
    	dbs.recors = 1;
	dbs.starting = 1;
	dbs.exactm = false;
	
while (option != NULL)
{ 	
if (option)
{
	if(option[0] == 'r')
	{
	   option++;
	  if (strlen(option) == 1) 
	  {
		 if(isdigit(option[0]))
		 if(option[0] <= '1' && '9' > option[0])
		 {
	         command_fail(si, fault_needmoreparams, _("Syntax: \2SEENNICK <nick> rX\2 where \2X>1 X<=10"));
	         return;			 
		 }
          dbs.recors = atoi(option);		 
	  } else if(strlen(option) == 2) {
		 if(option[0] != '1' || option[1] != '0')
		 {
	         command_fail(si, fault_needmoreparams, _("Syntax: \2SEENNICK <nick> rX\2 where \2X>1 X<=10"));
	         return;			 
		 }
          dbs.recors = atoi(option);		 
	  }   
	 }

	if(option[0] == 's')
	{
	  option++;
	  for(c=0;strlen(option) > c; c++)
	    if(!isdigit(option[c]))
		  {
			 command_fail(si, fault_needmoreparams, _("Syntax: \2SEENNICK <nick> sX\2 where X number"));
	         return;  
	     }
		dbs.starting = atoi(option);
	}

	if(option[0] == 'c')
	{
	 dbs.exactm = true;	
	}
   option = strtok(NULL, " ");  
  }	
}

      int count=0, match=0,i=0;
      while(snick[i] != '\0')
      {
		  if ((unsigned char)snick[i] >= 0x41 && (unsigned char)snick[i] <= 0x7d || (unsigned char)snick[i] >=0x30 && (unsigned char)snick[i] <= 0x39 || (unsigned char)snick[i] >= 0xc0 && (unsigned char)snick[i] <= 0xff || (unsigned char)snick[i] == 0x2d)	  
		  count++;
		  
		  if ((unsigned char)snick[i] == 0x2a || (unsigned char)snick[i] == 0x3f)
		  match=1;
		  
		  i++;
	  }
	  
	  if (count < 4 && match == 1)
	  {
		notice("SeenServ", si->su->nick, "\2%s\2 is too general, please include at least 4 non-wildcard characters.", snick);  
	  } else {
	   strcpy(dbs.purp, "R");
	   dbs.host = snick;
	   seen_db_t *cptr = &dbs;
	   seen_write_to_db(si->su->nick, cptr);		  
	  }
}