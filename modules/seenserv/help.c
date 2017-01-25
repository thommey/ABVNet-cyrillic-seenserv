/*
 * Copyright (c) 2015 Deyan Hristanov <gameoverbg@gmail.com>.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
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

DECLARE_MODULE_V1
(
     "seenserv/help", false, _modinit, _moddeinit,
    PACKAGE_STRING, 
	"Deyan Hristanov <gameoverbg@gmail.com>"
);

static void seen_cmd_help(sourceinfo_t *si, int parc, char *parv[]);

command_t seen_help = { "HELP", N_("Displays contextual help information."),
                         AC_NONE, 1, seen_cmd_help, { .path = "help"} };

void _modinit(module_t *m)
{
	service_named_bind_command("seenserv", &seen_help);
}

void _moddeinit(module_unload_intent_t intent)
{
	service_named_unbind_command("seenserv", &seen_help);
}


static void seen_cmd_help(sourceinfo_t *si, int parc, char *parv[])
{
    char *command = parv[0];
     
	if (si->c)
	{
		command = strchr(command, ' ');
		if (command != NULL)
			command++;
	}
		
    if (!command)
    {
        command_success_nodata(si, _("***** \2%s Help\2 *****"), si->service->nick);
        command_success_nodata(si, _("\2%s\2 is a utility allowing IRC users to obtain"),
                si->service->nick);
        command_success_nodata(si, "'last seen' information on certain nicknames or hosts."); 
        command_success_nodata(si, "To use the following commands, type: /msg SeenServ <command>."); 
        command_success_nodata(si, " ");
        command_success_nodata(si, _("For more information on a command, type:"));
        command_success_nodata(si, "\2/%s%s help <command>\2",
                (ircd->uses_rcommand == false) ? "msg " : "",
                si->service->disp);
        command_success_nodata(si, " ");

        command_help(si, si->service->commands);

        command_success_nodata(si, _("***** \2End of Help\2 *****"));
        return;
    }

	if (!strcasecmp("COMMANDS", command))
	{
		command_success_nodata(si, _("***** \2%s Help\2 *****"), si->service->nick);
		command_help(si, si->service->commands);
		command_success_nodata(si, _("***** \2End of Help\2 *****"));
		return;
	}
	
    help_display(si, si->service, command, si->service->commands);
}
