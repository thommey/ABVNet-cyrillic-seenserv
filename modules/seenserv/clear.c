/*
 * Copyright (c) 2016 Deyan Hristanov <gameoverbg@gmail.com>.
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
	"seenserv/clear", false, _modinit, _moddeinit,
	PACKAGE_STRING,
	VENDOR_STRING
);

static void seen_cmd_clear(sourceinfo_t *si, int parc, char *parv[]);

command_t seen_clear = { "CLEAR", N_("Clear info in SeenServ db."), PRIV_ADMIN, 1, seen_cmd_clear, { .path = "seenserv/clear" } };

void _modinit(module_t *m)
{
        service_named_bind_command("seenserv", &seen_clear);
}

void _moddeinit(module_unload_intent_t intent)
{
	service_named_unbind_command("seenserv", &seen_clear);
}

static void seen_cmd_clear(sourceinfo_t *si, int parc, char *parv[])
{
	logcommand(si, CMDLOG_GET, "CLEAR");
	command_success_nodata(si, _("It works!"));
}