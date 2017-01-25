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
#include "seenserv.h"

DECLARE_MODULE_V1
(
        "seenserv/main", true, _modinit, _moddeinit,
		PACKAGE_STRING, 
		"Deyan Hristanov <gameoverbg@gmail.com>"
);

//service_t *seensrv;

static void seena_join(hook_channel_joinpart_t *hdata);
static void seena_part(hook_channel_joinpart_t *hdata);
static void seena_on_shutdown(void *unused);
static void seen_leave_empty(void *unused);

static mowgli_eventloop_timer_t *seen_leave_empty_timer = NULL;

static void seen_handle_nickchange(user_t *u)
{
	if (seensrv.me == NULL || chansvs.me == NULL)
		return;
	char host[BUFSIZE];
	seen_db_t dbs;
	snprintf(host, sizeof host, "%s!%s@%s", u->nick, u->user, u->vhost);
	strcpy(dbs.purp, "S");
	dbs.method = 1;
	dbs.host = host;
	dbs.time_in = CURRTIME;
	dbs.time_out = 0;
	dbs.reason = "";
	seen_db_t *cptr = &dbs;
	seen_write_to_db(u->nick, cptr);
}

static void seen_nickchange(hook_user_nick_t *data)
{
	char host[BUFSIZE], reason[BUFSIZE];
	user_t *u = data->u;
	seen_db_t dbs;
	snprintf(host, sizeof host, "%s!%s@%s", data->oldnick, u->user, u->vhost);
	strcpy(dbs.purp, "P");
	dbs.method = 1;
	dbs.host = host;
	dbs.time_in = CURRTIME;
	dbs.time_out = CURRTIME;
	snprintf(reason, sizeof reason, "changing nick from %s to %s", data->oldnick, u->nick);
	dbs.reason = reason;
	seen_db_t *cptr = &dbs;
	seen_write_to_db(u->nick, cptr);
}
static void seen_user_delete_info_hook(hook_user_delete_t *hdata)
{    
	char host[BUFSIZE], reason[BUFSIZE];
	user_t *u = hdata->u;
	seen_db_t dbs;
	snprintf(host, sizeof host, "%s!%s@%s", u->nick, u->user, u->vhost);
	strcpy(dbs.purp, "P");
	dbs.method = 1;
	dbs.host = host;
	dbs.time_in = CURRTIME;
	dbs.time_out = CURRTIME;
	snprintf(reason, sizeof reason, "Quiting: %s", hdata->comment);
	dbs.reason = reason;
	seen_db_t *cptr = &dbs;
	seen_write_to_db(u->nick, cptr);
}
//int match(const char *mask, const char *name) funkciq za match sus zvezdichki 
static void seenserv(sourceinfo_t *si, int parc, char *parv[])
{
	mychan_t *mc = NULL;
	char orig[BUFSIZE];
	char newargs[BUFSIZE];
	char *cmd;
	char *args;
	char seenalt[BUFSIZE];

	/* this should never happen */
	if (parv[parc - 2][0] == '&')
	{
		slog(LG_ERROR, "services(): got parv with local channel: %s", parv[0]);
		return;
	}

	/* is this a fantasy command? */
	if (parv[parc - 2][0] == '#')
	{
		metadata_t *md;

		if (chansvs.fantasy == false)
		{
			/* *all* fantasy disabled */
			return;
		}

		mc = mychan_find(parv[parc - 2]);
		if (!mc)
		{
			/* unregistered, NFI how we got this message, but let's leave it alone! */
			return;
		}

		md = metadata_find(mc, "disable_fantasy");
		if (md)
		{
			/* fantasy disabled on this channel. don't message them, just bail. */
			return;
		}
	}

	/* make a copy of the original for debugging */
	mowgli_strlcpy(orig, parv[parc - 1], BUFSIZE);
    mowgli_strlcpy(seenalt, parv[parc - 1], BUFSIZE);
	/* lets go through this to get the command */
	cmd = strtok(seenalt, " ");
	if (!cmd)
		return;
	if (*orig == '\001')
	{
		handle_ctcp_common(si, cmd, strtok(NULL, ""));
		return;
	}
	/* take the command through the hash table */
	if (mc == NULL)
		command_exec_split(si->service, si, cmd, strtok(NULL, ""), si->service->commands);
	else
	{
		metadata_t *md = metadata_find(mc, "private:prefix");
		const char *prefix = (md ? md->value : seensrv.trigger);

		if (strlen(cmd) >= 2 && strchr(prefix, cmd[0]) && isalpha((unsigned char)*++cmd))
		{			
			const char *realcmd = service_resolve_alias(si->service, NULL, cmd);
			/* XXX not really nice to look up the command twice
			 * -- jilles */
			if (command_find(si->service->commands, realcmd) == NULL)
				return;
			if (floodcheck(si->su, si->service->me))
				return;
			/* construct <channel> <args> */
			mowgli_strlcpy(newargs, parv[parc - 2], sizeof newargs);
			args = strtok(NULL, " ");
			// while (args != NULL)
            // {
				// mowgli_strlcat(newargs, " ", sizeof newargs);
				// mowgli_strlcat(newargs, args, sizeof newargs);				
			// }
			while (args != NULL)
            {
				mowgli_strlcat(newargs, " ", sizeof newargs);
				mowgli_strlcat(newargs, args, sizeof newargs);
				args = strtok(NULL, " ");			
			}
			/* if (args != NULL)
			{
				mowgli_strlcat(newargs, " ", sizeof newargs);
				mowgli_strlcat(newargs, args, sizeof newargs);
			}

			args = strtok(NULL, " ");

			if (args != NULL)
			{
				mowgli_strlcat(newargs, " ", sizeof newargs);
				mowgli_strlcat(newargs, args, sizeof newargs);
			} */
			/* let the command know it's called as fantasy cmd */
			si->c = mc->chan;
			/* fantasy commands are always verbose
			 * (a little ugly but this way we can !set verbose)
			 */
			mc->flags |= MC_FORCEVERBOSE;
			command_exec_split(si->service, si, realcmd, newargs, si->service->commands);
			mc->flags &= ~MC_FORCEVERBOSE;
		}
		else if (!ircncasecmp(cmd, seensrv.nick, strlen(seensrv.nick)) && !isalnum((unsigned char)cmd[strlen(seensrv.nick)]) && (cmd = strtok(NULL, "")) != NULL)
		{
			//const char *realcmd;
			//char *pptr;
            //command_success_nodata(si, _("second"));
			//mowgli_strlcpy(newargs, parv[parc - 2], sizeof newargs);
			//while (*cmd == ' ')
				//cmd++;
			//if ((pptr = strchr(cmd, ' ')) != NULL)
			//{
				//mowgli_strlcat(newargs, pptr, sizeof newargs);
				//*pptr = '\0';
			//}

			//realcmd = service_resolve_alias(si->service, NULL, cmd);

			//if (command_find(si->service->commands, realcmd) == NULL)
				//return;
			//if (floodcheck(si->su, si->service->me))
				//return;

			//si->c = mc->chan;

			//* fantasy commands are always verbose
			 //* (a little ugly but this way we can !set verbose)
			 //*/
			//mc->flags |= MC_FORCEVERBOSE;
			//command_exec_split(si->service, si, realcmd, newargs, si->service->commands);
			//mc->flags &= ~MC_FORCEVERBOSE;
		}
	}
}

static void seen_config_ready(void *unused)
{
	seensrv.nick = seensrv.me->nick;
	seensrv.user = seensrv.me->user;
	seensrv.host = seensrv.me->host;
	seensrv.real = seensrv.me->real;
	
	service_set_chanmsg(seensrv.me, true);
}

void _modinit(module_t *m)
{
	hook_add_event("config_ready");
	hook_add_config_ready(seen_config_ready);

    hook_add_event("nick_check");
    hook_add_nick_check(seen_handle_nickchange);
		
	seensrv.me = service_add("seenserv", seenserv);
	
	hook_add_event("user_delete_info");
	hook_add_user_delete_info(seen_user_delete_info_hook);
	
	hook_add_event("channel_join");
	hook_add_event("channel_part");
	hook_add_event("shutdown");
	hook_add_event("user_nickchange");
	hook_add_channel_join(seena_join);
	hook_add_channel_part(seena_part);
	hook_add_shutdown(seena_on_shutdown);
	hook_add_user_nickchange(seen_nickchange);
	
	seen_leave_empty_timer = mowgli_timer_add(base_eventloop, "seen_leave_empty", seen_leave_empty, NULL, 300);
	add_dupstr_conf_item("TRIGGER", &seensrv.me->conf_table, 0, &seensrv.trigger, "!");
	add_bool_conf_item("FANTASY", &seensrv.me->conf_table, 0, &seensrv.fantasy, false);
	add_uint_conf_item("SEENEXPIRE", &seensrv.me->conf_table, 0, &seensrv.seenexpire, 1, 1000, 30);
}

void _moddeinit(module_unload_intent_t intent)
{    	
	hook_del_config_ready(seen_config_ready);
    hook_del_nick_check(seen_handle_nickchange);
	if (seensrv.me)
	{
		seensrv.nick = NULL;
		seensrv.user = NULL;
		seensrv.host = NULL;
		seensrv.real = NULL;
		service_delete(seensrv.me);
		seensrv.me = NULL;
	}
	
	//hook_del_user_delete_info(seen_user_delete_info_hook);   
 	
    hook_del_channel_join(seena_join);
	hook_del_channel_part(seena_part);
    hook_del_shutdown(seena_on_shutdown);
	//hook_del_user_nickchange(seen_nickchange);
	mowgli_timer_destroy(base_eventloop, seen_leave_empty_timer);
}

static void seena_join(hook_channel_joinpart_t *hdata)
{
	chanuser_t *cu = hdata->cu;
	channel_t *chan;
	mychan_t *mc;
	user_t *u;
	
	if (cu == NULL || is_internal_client(cu->user))
		return;
	chan = cu->chan;

	/* first check if this is a registered channel at all */
	mc = mychan_from(chan);
	if (mc == NULL)
		return;

	if (mc->flags & MC_SEEN_GUARD)
	   {
		join(chan->name, seensrv.nick);
	   }
	if (mc->flags & MC_GUARD)
	   {
		join(chan->name, chansvs.nick);
	   }	
}

static void seena_part(hook_channel_joinpart_t *hdata)
{
	chanuser_t *cu;
	mychan_t *mc;

	cu = hdata->cu;
	if (cu == NULL)
		return;
	mc = mychan_find(cu->chan->name);
	if (mc == NULL)
		return;
	
	if (!config_options.leave_chans)
		return;
	
	if (cu->chan->nummembers - cu->chan->numsvcmembers > 1)
		return;

	if (is_internal_client(cu->user))
		return;
	
	/* if we're enforcing an akick, we're MC_INHABIT.  do not part. */
	if (mc->flags & MC_INHABIT)
	{
		slog(LG_DEBUG, "cs_part(): not leaving channel %s due to MC_INHABIT flag", mc->name);
		return;
	}
	
	part(cu->chan->name, seensrv.nick);
}
static void seena_on_shutdown(void *unused)
{
	if (seensrv.me != NULL)
		quit_sts(seensrv.me->me, "shutting down");
}

static void seen_leave_empty(void *unused)
{
	mychan_t *mc;
	mowgli_patricia_iteration_state_t state;

	(void)unused;
	MOWGLI_PATRICIA_FOREACH(mc, &state, mclist)
	{
		if (!(mc->flags & MC_INHABIT))
			continue;
		/* If there is only one user, stay indefinitely. */
		if (mc->chan != NULL && mc->chan->nummembers - mc->chan->numsvcmembers == 1)
			continue;
		mc->flags &= ~MC_INHABIT;
		if (mc->chan != NULL &&
				!(mc->chan->flags & CHAN_LOG) &&
				(!(mc->flags & MC_SEEN_GUARD) ||
				 (config_options.leave_chans && mc->chan->nummembers == mc->chan->numsvcmembers)) &&
				chanuser_find(mc->chan, seensrv.me->me))
		{
			slog(LG_DEBUG, "seen_leave_empty(): leaving %s", mc->chan->name);
			 part(mc->chan->name, seensrv.nick);
		}
	}
}
