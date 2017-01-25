/*
 * Copyright (c) 2015 Deyan Hristanov <gameoverbg@gmail.com>.
 * Copyright (c) 2016 Al Indjov <ali.indzhov@gmail.com>.
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
#include "seenserv.h"

DECLARE_MODULE_V1("seenserv/part", false, _modinit, _moddeinit, PACKAGE_STRING, "Deyan Hristanov <gameoverbg@gmail.com>");

static void seen_cmd_part(sourceinfo_t *si, int parc, char *parv[]);

command_t seen_part = { "PART", N_("Parts a specified channel (identified successors and above)"), AC_NONE, 2, seen_cmd_part, {.path = "seenserv/part"} };

void _modinit(module_t *m)
{
	service_named_bind_command("seenserv", &seen_part);
}

void _moddeinit(module_unload_intent_t intent)
{

	service_named_unbind_command("seenserv", &seen_part);
}

void seen_cmd_part(sourceinfo_t *si, int parc, char *parv[])
{

	char *chan = parv[0];
	int guardsts = 0;
	mychan_t *mc;
	
	service_t *seensvs = si->service;
	// Ignore fantasy commands
	if (si->c != NULL)
		return;

	if (!chan)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "PART");
		command_fail(si, fault_needmoreparams, "Syntax: PART <#channel>");
		return;
	}
	
	if (!(mc = mychan_find(chan)))
	{
		command_fail(si, fault_nosuch_target, _("Channel \2%s\2 is not registered."), chan);
		return;
	}

 	if (chanacs_source_has_flag(mc, si, CA_SET) || has_priv(si, PRIV_CHAN_ADMIN))
	{ 
	} else {
		command_fail(si, fault_noprivs, _("You are not authorized to perform this command."));
		return;
	}

	if (!(MC_SEEN_GUARD & mc->flags))
	{
		command_fail(si, fault_nochange, _("SeenServ is already been gone from channel \2%s\2."), mc->name);
		return;
	}
	 mc->flags &= ~MC_SEEN_GUARD;
	 part(mc->name, si->service->nick);
	 command_success_nodata(si, _("SeenServ will part channel \2%s\2."), mc->name);
	 return;	 
}
